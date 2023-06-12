#/bin/sh
set -e

compile_qemu() {

    VERSION=$1
    #GCOV=$2
    TYPE=$2

    # NORMAL
    if [ "$TYPE" = "normal" ]; then
        if [ -d "./qemu-$VERSION" ]; then
            echo "Directory ./qemu-$VERSION already exists."
            exit 1
        fi
    fi

    # GCOV
    if [ "$TYPE" = "gcov" ]; then
        if [ -d "./qemu-$VERSION-gcov" ]; then
            echo "Directory ./qemu-$VERSION-gcov already exists."
            exit 1
        fi
    fi

    # ASAN
    if [ "$TYPE" = "asan" ]; then
        if [ -d "./qemu-$VERSION-asan" ]; then
            echo "Directory ./qemu-$VERSION-asan already exists."
            exit 1
        fi
    fi

    wget https://download.qemu.org/qemu-$VERSION.tar.xz
    tar xf qemu-$VERSION.tar.xz -C /tmp/


    # NORMAL
    if [ "$TYPE" = "normal" ]; then
        mv /tmp/qemu-$VERSION ./qemu-$VERSION
        cd qemu-$VERSION
    fi

    # GCOV
    if [ "$TYPE" = "gcov" ]; then
        mv /tmp/qemu-$VERSION ./qemu-$VERSION-gcov
        cd qemu-$VERSION-gcov
    fi

    # ASAN
    if [ "$TYPE" = "asan" ]; then
        mv /tmp/qemu-$VERSION ./qemu-$VERSION-asan
        cd qemu-$VERSION-asan
    fi


    # NORMAL
    if [ "$TYPE" = "normal" ]; then
        ./configure --target-list=x86_64-softmmu
    fi

    # GCOV
    if [ "$TYPE" = "gcov" ]; then
        # Patch for GCOV
        patch -p1 < ../$VERSION-gcov.patch
        
        ./configure --target-list=x86_64-softmmu --enable-gcov
    fi

    # ASAN
    if [ "$TYPE" = "asan" ]; then
        echo $VERSION
        if [ "$VERSION" = "2.0.0" ] || [ "$VERSION" = "2.5.0" ]; then
	    CC="gcc -fsanitize=address" ./configure --target-list=x86_64-softmmu
        else
            ./configure --target-list=x86_64-softmmu --enable-sanitizers
        fi
    fi

    make -j 8
    cd -

    rm qemu-$VERSION.tar.xz
}

show_help() {
    echo "Usage: compile_qemu.sh [version] [gcov]"
    echo "Compile QEMU"
    echo ""
    echo "    version        version of QEMU to compile"
    echo "    type           enable different types "
    echo "                    => [normal, gcov, asan]"
    echo ""
    echo "    Available versions:      [2.0.0, 2.5.0, 3.0.0, 4.0.0, 5.0.0, 7.2.1]"
    echo "    Supported GCOV versions: [2.5.0, 3.0.0, 4.0.0]"
    echo ""
}

if [ "$#" -ne 2 ]; then
    show_help
    exit 1
fi

if [ "$1" != "2.0.0" ] && [ "$1" != "2.5.0" ] && [ "$1" != "3.0.0" ] && [ "$1" != "4.0.0" ] && [ "$1" != "5.0.0" ] && [ "$1" != "7.2.1" ]; then
    show_help
    exit 1
fi

if [ "$2" != "normal" ] && [ "$2" != "gcov" ] && [ "$2" != "asan" ]; then
    show_help
    exit 1
fi

if [ "$2" = "gcov" ]; then
    if [ "$1" != "2.5.0" ] && [ "$1" != "3.0.0" ] && [ "$1" != "4.0.0" ]; then
        show_help
        exit 1
    fi
fi

compile_qemu $1 $2
