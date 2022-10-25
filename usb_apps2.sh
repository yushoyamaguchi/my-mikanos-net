#!/bin/bash

#
# 試験用パソコンで実行するためのファイルをUSBに書き込む
# P30,P80
#

set -ex

#
# usbを差し込んでdmsgコマンドを実行した結果、/dev/sdbになっていることを確認のうえ実行
#
USB=/dev/sda1
KERNEL_ELF=/home/y-yamaguchi/yusho/mikanos-net/kernel/kernel.elf
LOADER_EFI=/home/y-yamaguchi/edk2/Build/MikanLoaderX64/DEBUG_CLANG38/X64/Loader.efi
APPS_DIR_IN_DEV=/home/y-yamaguchi/yusho/mikanos-net/apps
MNT_DIR=/mnt/usbmem


umount $USB
mkfs.fat $USB
mkdir -p $MNT_DIR
mount $USB $MNT_DIR
mkdir -p $MNT_DIR/EFI/BOOT
cp $KERNEL_ELF $MNT_DIR/kernel.elf
cp $LOADER_EFI $MNT_DIR/EFI/BOOT/BOOTX64.EFI
for APP in $(ls $APPS_DIR_IN_DEV)
do
  if [ -f "$APPS_DIR_IN_DEV/$APP/$APP" ]
  then
    cp "$APPS_DIR_IN_DEV/$APP/$APP" $MNT_DIR
  fi
done
umount $USB