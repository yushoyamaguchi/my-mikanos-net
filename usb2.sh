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

umount $USB
mkfs.fat $USB
mkdir -p /mnt/usbmem
mount $USB /mnt/usbmem
mkdir -p /mnt/usbmem/EFI/BOOT
cp $KERNEL_ELF /mnt/usbmem/kernel.elf
cp $LOADER_EFI /mnt/usbmem/EFI/BOOT/BOOTX64.EFI
umount $USB