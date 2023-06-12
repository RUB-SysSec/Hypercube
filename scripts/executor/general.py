from abc import ABC, abstractmethod
import enum

class ExitReason(enum.Enum):
    TIMEOUT = 0 
    SIGSEGV = 1
    SIGBUS = 2
    SIGILL = 3
    SIGKILL = 4
    SIGFPE = 5
    ASAN = 6
    ABORT = 7
    NO_FUZZING_REGIONS = 8
    DEADLOCK = 9
    REPRODUCER_FINISHED = 10
    OS_PANIC = 11
    OTHER = 12

class TargetExecutor(ABC):

    def __init__(self, executable_path, extra):
        self.executable_path = executable_path
        self.extra = extra

    # return values: success, available_areas, configured_areas, stdout_output, stderr_output 
    @abstractmethod
    def dry_run_target(self, image_path, mem_size=100):
        pass

    # return values: stdout_output, stderr_output, exit_reason_type, exit_reason_str, seed, run_time
    @abstractmethod
    def run_target(self, image_path, timeout_sec=10.0, debug=False, mem_size=100, deadlock_watchdog=False, gcov_mode=False, memlimit=None):
        pass

    # return value: cmd_line
    @abstractmethod
    def get_target_cmd_line(self, image_path):
        pass