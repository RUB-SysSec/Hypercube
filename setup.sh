#/bin/bash
set -e

run_setup() {
    echo "[!] Installing required dependencies..."

    sudo -Eu root apt-get update
    sudo -Eu root apt-get install automake bash-completion bison cpio debhelper dh-autoreconf dh-systemd dosfstools flex fonts-dejavu-core gcc-5 gcc-5-multilib gettext help2man libdevmapper-dev libefivar-dev libfreetype6-dev libfuse-dev liblzma-dev liblzo2-dev libparted-dev libparted-dev libsdl1.2-dev libxen-dev mtools parted patchutils pkg-config po-debconf python python3 qemu-system texinfo ttf-dejavu-core wamerican xfonts-unifont xorriso make automake autoconf bison flex python python3 g++-multilib libisoburn1 pkg-config zlib1g-dev libglib2.0-dev libfreetype6-dev unifont mtools gcc python3-dev

    cd /tmp
    wget https://bootstrap.pypa.io/pip/3.5/get-pip.py
    python3 get-pip.py
    cd -

    cd /tmp
    if [ ! -d "/tmp/python-ptrace" ]; then
        git clone https://github.com/vstinner/python-ptrace.git
    fi
    cd - 

    cd /tmp/python-ptrace
    sudo -Eu root python3 setup.py install
    cd -
}

if [ "$EUID" -eq 1 ]
  then echo "Please do not run this script as root"
  exit 1
fi

if ! grep -q -E "Ubuntu 16.04|Ubuntu 18.04" /etc/lsb-release; 
then
    echo "This script works only on Ubuntu 16.04 or 18.04"
    exit 1
fi

WORKDIR=$PWD

if cat /etc/lsb-release | grep 16.04 > /dev/null; 
then
    run_setup
    $HOME/.local/bin/pip3.5 install psutil
fi

if cat /etc/lsb-release | grep 18.04 > /dev/null; 
then
    run_setup
    sudo -Eu root apt-get install libpixman-1-dev ninja-build
    $HOME/.local/bin/pip3.6 install psutil
fi

cd $WORKDIR

# compile GRUB
cd os/grub/
sh install.sh
cd -

# compiler tesseract helper tool
cd os/tools/
sh compile.sh
cd -

# compile segfault wrapper
cd misc/
sh compile.sh
cd - 

echo "[!] Setup finished!"
