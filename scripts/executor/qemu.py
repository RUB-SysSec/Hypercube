import os
import subprocess
import ptrace
import ptrace.debugger
from ptrace.binding import ptrace_traceme
import signal
import sys
import threading
import queue

import selectors
import psutil
import collections
import time
import resource
import uuid

from scripts.executor.general import TargetExecutor, ExitReason

class QEMUTargetExecutor(TargetExecutor):

    def set_mem_limit_trace_me(limit):
        resource.setrlimit(resource.RLIMIT_AS, (limit, resource.RLIM_INFINITY))
        ptrace_traceme()

    def __terminate_target(process, gcov_mode):
        if gcov_mode:
            process.kill(signal.SIGSEGV)
        else:
            process.kill(signal.SIGKILL)

    def watchdog_worker(process, queue, gcov_mode):

        x = collections.deque(3*[0.0], 3)

        deadlock_detected = False

        time.sleep(1)
        while process.poll() is None:
            x.append(psutil.cpu_percent(percpu=True)[0])
            if x[0] <= 5.0 and x[1] <= 5.0 and x[2] <= 5.0:
                time.sleep(0.5)
                if psutil.cpu_percent(percpu=True)[0] <= 5.0:
                    #print(x)
                    #time.sleep(1)
                    QEMUTargetExecutor.__terminate_target(process, gcov_mode)
                    deadlock_detected = True
                    break
            time.sleep(0.5)

        queue.put(deadlock_detected)
        #print("WATCHDOG: Done")

    def io_worker(pipe, pipe2, process, queue, gcov_mode):
        data = ""
        data2 = ""
        no_fuzzing_regions_error = False
        reproducer_finished = False
        os_panicked = False

        sel = selectors.DefaultSelector()
        sel.register(pipe, selectors.EVENT_READ)
        #sel.register(pipe2, selectors.EVENT_READ)

        running = True

        while running:
            for key, val1 in sel.select():
                try:
                    line = key.fileobj.readline().decode('utf-8')
                except UnicodeDecodeError:
                    continue
                if not line:
                    running = False
                    break
                if key.fileobj is pipe:
                    #print(line)
                    data += line
                    if "Error: No fuzzing regions found..." in line: 
                        no_fuzzing_regions_error = True
                        QEMUTargetExecutor.__terminate_target(process, gcov_mode)
                        running = False
                        break
                    if "reproducer finished!" in line:
                        reproducer_finished = True
                        QEMUTargetExecutor.__terminate_target(process, gcov_mode)
                        running = False
                        break
                    if "Kernel Panic" in line:
                        os_panicked = True
                        QEMUTargetExecutor.__terminate_target(process, gcov_mode)
                        running = False
                        break

                else:
                    #print(line)
                    data2 += line #.decode('utf-8')

        queue.put((data, data2, no_fuzzing_regions_error, reproducer_finished, os_panicked)) 

    def __kill_qemu(process, gcov_mode):
        QEMUTargetExecutor.__terminate_target(process, gcov_mode)


    def __run_target(self, cmd, timeout_sec=10.0, debug=False, deadlock_watchdog=False, gcov_mode=False, memlimit=None, log_asan=False):

        if gcov_mode:

            if not os.path.exists("misc/gcov_hooks.so"):
                print("[!] Error: gcov_hooks.so not found! Please build it first.")
                sys.exit(1)

            gcov_module_path = os.path.realpath("misc/gcov_hooks.so")
            env = {}
            env["LD_PRELOAD"] = gcov_module_path
        else:
            env = None

        if log_asan:
            if env is None:
                env = {}
            env["ASAN_OPTIONS"] = "log_path=/tmp/asan.log"

        
        stderr_file = "/tmp/stderr.txt.%s"%uuid.uuid4()
        stderr_file_f = open(stderr_file, "w")

        if memlimit is not None:
            f = (lambda : QEMUTargetExecutor.set_mem_limit_trace_me(memlimit))
            child_process = subprocess.Popen(cmd, preexec_fn=f, stdout=subprocess.PIPE, stderr=stderr_file_f, stdin=subprocess.PIPE, bufsize=1 , env=env)
        else:
            child_process = subprocess.Popen(cmd, preexec_fn=ptrace_traceme, stdout=subprocess.PIPE, stderr=stderr_file_f, stdin=subprocess.PIPE, bufsize=1 , env=env)

        os.kill(child_process.pid, signal.SIGSTOP)

        if debug:
            print("PID: %s" % child_process.pid)

        pid = child_process.pid

        # Attach to the child process using the ptrace debugger
        debugger = ptrace.debugger.PtraceDebugger()

        debugger.traceFork()
        debugger.traceExec()
        debugger.traceClone()

        process = debugger.addProcess(child_process.pid, True)

        q = queue.Queue()
        q2 = queue.Queue()

        stdout_thread = threading.Thread(target=QEMUTargetExecutor.io_worker, args=(child_process.stdout, child_process.stderr, process, q, gcov_mode))
        kill_timer = threading.Timer(timeout_sec, QEMUTargetExecutor.__kill_qemu, [process, gcov_mode])

        stdout_thread.start()
        kill_timer.start()

        if deadlock_watchdog:
            watchdog_thread = threading.Thread(target=QEMUTargetExecutor.watchdog_worker, args=(child_process, q2, gcov_mode))
            watchdog_thread.start()


        process.cont()
        start_time = time.time()

        exit_reason_type = None
        exit_reason_str = ""

        stdout_output = ""

        #help(process.kill)
        #help(debugger)

        while len(debugger) > 0:

            try:
                event = debugger.waitSignals()
                if debug:
                    print("Signal: %s" % event)
            except ptrace.debugger.process_event.NewProcessEvent as e:
                if debug:
                    print("process_event.NewProcessEvent: %s" % e)
                event = e
            except ptrace.debugger.process_event.ProcessExit as e:
                if debug:
                    print("process_event.ProcessExit: %s" % e)
                event = e

            
            #help(debugger)
            #event = debugger.waitProcessEvent()

            if debug:
                print("Event: %s" % event)

            if type(event) == ptrace.debugger.ptrace_signal.ProcessSignal:

                signum = event.getSignalInfo().si_signo

                if debug and signum != signal.SIGUSR1:
                    print("Signum: %s" % signum)

                if signum == signal.SIGILL :
                    exit_reason_str = "IP: %s" % hex(event.process.getInstrPointer())
                    exit_reason_type= ExitReason.SIGILL
                    QEMUTargetExecutor.__terminate_target(process, gcov_mode)
                    #child_process.kill()
                    #break

                elif signum == signal.SIGFPE :
                    exit_reason_str = "IP: %s" % hex(event.process.getInstrPointer())
                    exit_reason_type= ExitReason.SIGFPE
                    QEMUTargetExecutor.__terminate_target(process, gcov_mode)
                    #child_process.kill()
                    #break

                elif signum == signal.SIGBUS:
                    exit_reason_str = "IP: %s" % hex(event.process.getInstrPointer())
                    exit_reason_type= ExitReason.SIGBUS
                    QEMUTargetExecutor.__terminate_target(process, gcov_mode)
                    #child_process.kill()
                    #break

                elif signum == signal.SIGABRT:
                    kill_timer.cancel()
                    exit_reason_str = "IP: %s" % hex(event.process.getInstrPointer())
                    exit_reason_type= ExitReason.ABORT
                    QEMUTargetExecutor.__terminate_target(process, gcov_mode)
                    if gcov_mode:
                        event.process.cont(signal.SIGABRT)

                    #child_process.kill()
                    #break
                
                elif signum == signal.SIGKILL:
                    # check if this signal was sent by us
                    exit_reason_str = "IP: %s" % hex(event.process.getInstrPointer())

                    if exit_reason_type is None:
                        exit_reason_type= ExitReason.SIGKILL

                    #break

                elif signum == signal.SIGSEGV:

                    if gcov_mode:
                        # pass intercepted signal to the target
                        event.process.cont(signal.SIGSEGV)
                    else:
                        ip = event.process.getInstrPointer()
                        fault_address = event.process.getsiginfo()._sigfault._addr 

                        if fault_address:
                            exit_reason_str = "IP: %s / Fault address: %s" % (hex(ip), hex(fault_address))
                        else:
                            exit_reason_str = "IP: %s" % (hex(ip))
                        exit_reason_type= ExitReason.SIGSEGV
                        #QEMUTargetExecutor.__terminate_target(process, gcov_mode)
                        #child_process.kill()

                elif signum == signal.SIGUSR1:
                    event.process.cont(signal.SIGUSR1)
                else:
                    process.cont()
            
            elif type(event) == ptrace.debugger.process_event.NewProcessEvent:
                if debug:
                    print("New process: %s" % event)
                #print("PID: %s" % event.process.pid)

                process.cont()
                debugger[event.process.pid].cont()

            else:
                if debug:
                    print("Event: %s" % event)

        end_time = time.time()

        if deadlock_watchdog:
            watchdog_thread.join()
            deadlock_detected = q2.get()
        else:
            deadlock_detected = False

        timeout = False
        try:
            if kill_timer.is_alive():
                kill_timer.cancel()
            else:
                timeout = True
        except:
            pass

        stdout_thread.join()
        stdout_output, stderr_output, no_fuzzing_regions_error, reproducer_finished, os_panicked = q.get()


        if log_asan:
            if os.path.exists("/tmp/asan.log.%s" % pid):
                f = open("/tmp/asan.log.%s" % pid)
                while True:
                    tmp = f.read()
                    if tmp is None or len(tmp) == 0:
                        break
                    stderr_output += tmp
                f.close()
                os.remove("/tmp/asan.log.%s" % pid)

        stderr_file_f.close()

        stderr_file_f = open(stderr_file, "r")
        stderr_output += stderr_file_f.read()
        stderr_file_f.close()

        os.remove(stderr_file)



        if no_fuzzing_regions_error:
            exit_reason_type= ExitReason.NO_FUZZING_REGIONS

        if reproducer_finished:
            exit_reason_type= ExitReason.REPRODUCER_FINISHED

        if os_panicked:
            exit_reason_type= ExitReason.OS_PANIC

        # get seed:
        seed = None
        for stdout_line in stdout_output.splitlines():
            if stdout_line.startswith("Seed: "):
                seed = stdout_line.split("Seed: ")[1]
                break

        #if exit_reason_type== None:
        #    for line in stderr_output.splitlines():
        #        print(line)

        if exit_reason_type== None and timeout:
            exit_reason_type= ExitReason.TIMEOUT
            exit_reason_str = "timer=%d" % timeout_sec
        elif exit_reason_type== None and deadlock_detected:
            exit_reason_type= ExitReason.DEADLOCK
        elif exit_reason_type== None:
            if "ERROR:" in stderr_output:
                exit_reason_type= ExitReason.ASAN
                for idx, line in enumerate(stderr_output.splitlines()):
                    if "ERROR:" in line:
                        #print(stderr_output)
                        exit_reason_str = line + " " + stderr_output.splitlines()[idx+1]
                        exit_reason_str = exit_reason_str.split("ERROR: ")[1]
                        exit_reason_str += " " + stderr_output.splitlines()[idx+2]
                        break
                    
        elif exit_reason_type == ExitReason.ABORT:
            if "Assertion" in stderr_output:
                for line in stderr_output.splitlines():
                    if "Assertion" in line:
                        exit_reason_str = exit_reason_str + " reason:" + stderr_output.splitlines()[-1]

        if exit_reason_type is None:
            #print(stdout_line)
            if "Gcov fault handler called (Signal: " in stdout_output:
                signum = stdout_output.split("Gcov fault handler called (Signal: ")[1].split(")")[0]
                exit_reason_type= ExitReason.SIGSEGV
                exit_reason_str = "handled by gcov hooks (Signal: %s)" % signum
        
        #print(stderr_output)
        #print(stdout_output)

        return stdout_output, stderr_output, exit_reason_type, exit_reason_str, seed, end_time-start_time

    def __dry_run_target(self, cmd):

        qemu_process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE, bufsize=1) # encoding='utf-8',

        sel = selectors.DefaultSelector()
        sel.register(qemu_process.stdout, selectors.EVENT_READ)
        sel.register(qemu_process.stderr, selectors.EVENT_READ)

        running = True
        failed = False

        available_areas = []
        registered_areas = []
        stdout_output = ""
        stderr_output = ""

        while running:
                
                for key, val1 in sel.select():
                    line = key.fileobj.readline().decode('utf-8')
                    if not line:
                        running = False
                        break
                    if key.fileobj is qemu_process.stdout:
                        stdout_output = stdout_output + line

                        if "Error: No fuzzing regions found..." in line:
                            running = False
                            qemu_process.kill()
                            break

                        if "Fuzzer ready!" in line:
                            running = False
                            qemu_process.kill()
                            break

                        if "available area - " in line:
                            available_areas.append(line.split("available area - ")[1].strip())

                        if "registered area - " in line:
                            registered_areas.append(line.split("registered area - ")[1].strip())
                    else:
                        stderr_output = stderr_output + line

        stderr_output = stderr_output + qemu_process.stderr.read().decode('utf-8')

        if len(available_areas) != 0:
            return len(available_areas) != 0, available_areas, registered_areas, stdout_output, stderr_output
        
        else:
            qemu_process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE, bufsize=1) #  encoding='utf-8'
            stdout_output, stderr_output = qemu_process.communicate()
            return False, [], [], stdout_output, stderr_output
        #print(stdout_output)
        #return len(available_areas) != 0, available_areas, registered_areas, stdout_output, stderr_output

    def dry_run_target(self, image_path, mem_size=100):
        
        if not os.path.exists(image_path):
            print("[!] Error: Image (%s) not found" % image_path)
            sys.exit(1)

        if self.extra is None:
            extra = ""
        else:
            extra = self.extra
        
        cmd_line = "%s -cdrom %s -serial mon:stdio -m %s -nographic -net none %s " % (self.executable_path, image_path, mem_size, extra)
        
        print("[!] Target cmd: %s" % cmd_line)

        return self.__dry_run_target(cmd_line.split())


    def run_target(self, image_path, timeout_sec=10.0, debug=False, mem_size=100, deadlock_watchdog=False, gcov_mode=False, memlimit=None):

        if not os.path.exists(image_path):
            print("[!] Error: Image (%s) not found" % image_path)
            sys.exit(1)

        if deadlock_watchdog:
            p = psutil.Process()
            p.cpu_affinity([0])

        if self.extra is None:
            extra = ""
        else:
            extra = self.extra

        log_asan = False

        if "-asan/x86_64-softmmu/qemu-system-x86_64" in self.executable_path or "-asan/build/x86_64-softmmu/qemu-system-x86_64" in self.executable_path:
            log_asan = True

        cmd_line = "%s -cdrom %s -serial mon:stdio -m %s -nographic -net none %s " % (self.executable_path, image_path, mem_size, extra)
        
        if debug:
            print("[!] Target cmd: %s" % cmd_line)

        cmd = cmd_line.split()

        stdout_output, stderr_output, exit_reason_type, exit_reason_str, seed, run_time = self.__run_target(cmd, timeout_sec=timeout_sec, debug=debug, deadlock_watchdog=deadlock_watchdog, gcov_mode=gcov_mode, memlimit=memlimit, log_asan=log_asan)

        if exit_reason_type== None:
            if "qemu-system-x86_64: " in stderr_output and stderr_output.count("qemu-system-x86_64: ") == 1:
                for line in stderr_output.splitlines():
                    if "qemu-system-x86_64: " in line:
                            exit_reason_type = ExitReason.OTHER
                            exit_reason_str = line.split("qemu-system-x86_64: ")[1]
                            break

        return stdout_output, stderr_output, exit_reason_type, exit_reason_str, seed, run_time


    def get_target_cmd_line(self, image_path, mem_size=100):
        if self.extra is None:
            extra = ""
        else:
            extra = self.extra

        cmd_line = "%s -cdrom %s -serial mon:stdio -m %s -nographic -net none %s " % (self.executable_path, image_path, mem_size, extra)

        return cmd_line

