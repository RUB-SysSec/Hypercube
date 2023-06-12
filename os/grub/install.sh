#/bin/bash

GRUB_VERSION="2.02"
GRUB_URL="ftp://ftp.gnu.org//gnu/grub/grub-2.02.tar.gz"
GRUB_MD5="1116d1f60c840e6dbd67abbc99acb45d"
PWD_BIOS=$(pwd)/bios/
PWD_EFI=$(pwd)/efi/
PWD_EFI_APP=$(pwd)/efi_app/

#echo "[*] Installing build dependencies for GRUB2 $GRUB_VERSION ..."
#sudo -Eu root apt-get build-dep grub2 -y > /dev/null

echo "[*] Downloading GRUB2 $GRUB_VERSION ..."
wget -O grub.tar.gz $GRUB_URL 2> /dev/null

echo "[*] Checking signature of GRUB2 $GRUB_VERSION ..."
CHKSUM=`md5sum grub.tar.gz| cut -d' ' -f1`

if [ "$CHKSUM" != "$GRUB_MD5" ]; then
  echo "[-] Error: signature mismatch..."
  exit 1
fi

echo "[*] Unpacking GRUB2 $GRUB_VERSION ..."
tar xf grub.tar.gz

rm ./compile-bios.stdout
rm ./compile-bios.stderr
set -e
echo "[*] Compiling GRUB2 $GRUB_VERSION BIOS bootloader ..."
cd grub-2.02/
CFLAGS=-Wno-error=unused-value ./autogen.sh >> ../compile-bios.stdout.log 2>> ../compile-bios.stderr.log
CFLAGS=-Wno-error=unused-value ./configure --target=i386 --with-platform="pc" --prefix=$PWD_BIOS >> ../compile-bios.stdout.log 2>> ../compile-bios.stderr.log
CFLAGS=-Wno-error=unused-value make -j 8 >> ../compile-bios.stdout.log 2>> ../compile-bios.stderr.log
make install  >> ../compile-bios.stdout.log 2>> ../compile-bios.stderr.log
make clean >> ../compile-bios.stdout.log 2>> ../compile-bios.stderr.log
cd .. 
set +e

rm ./compile-efi.stdout
rm ./compile-efi.stderr
set -e
echo "[*] Compiling GRUB2 $GRUB_VERSION EFI bootloader ..."
cd grub-2.02/
CFLAGS=-Wno-error=unused-value ./autogen.sh >> ../compile-efi.stdout.log 2>> ../compile-efi.stderr.log
CFLAGS=-Wno-error=unused-value ./configure --target=i386 --with-platform="efi" --enable-grub-mkfont --prefix=$PWD_EFI >> ../compile-efi.stdout.log 2>> ../compile-efi.stderr.log
CFLAGS=-Wno-error=unused-value make -j 8 >> ../compile-efi.stdout.log 2>> ../compile-efi.stderr.log
make install  >> stdout.log 2>> ../compile-efi.stderr.log
make clean >> stdout.log 2>> ../compile-efi.stderr.log
cd .. 
set +e

rm ./compile-efi-app.stdout
rm ./compile-efi-app.stderr
set -e
echo "[*] Compiling GRUB2 $GRUB_VERSION EFI-APP bootloader ..."
patch grub-2.02/grub-core/video/video.c 		< patches/video_video.patch 
patch grub-2.02/grub-core/loader/multiboot.c 	< patches/loader_multiboot.patch 
patch grub-2.02/grub-core/kern/efi/mm.c 		< patches/kern_efi_mm.patch 
cd grub-2.02/
CFLAGS=-Wno-error=unused-value ./autogen.sh >> ../compile-efi-app.stdout.log 2>> ../compile-efi-app.stderr.log
CFLAGS=-Wno-error=unused-value ./configure --target=i386 --with-platform="efi" --enable-grub-mkfont --prefix=$PWD_EFI_APP >> ../compile-efi-app.stdout.log 2>> ../compile-efi-app.stderr.log
CFLAGS=-Wno-error=unused-value make -j 8 >> ../compile-efi-app.stdout.log 2>> ../compile-efi-app.stderr.log
make install  >> stdout.log 2>> ../compile-efi-app.stderr.log
make clean >> stdout.log 2>> ../compile-efi-app.stderr.log
cd .. 

rm -rf ./grub-2.02/
rm -f grub.tar.gz
echo "[*] Done ..."
exit 0
