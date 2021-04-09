# MikanOS-Net

MikanOS にネットワーク機能を追加した改造版です。

プロトコルスタックは https://github.com/pandax381/microps を移植しています。

## 事前準備

### QEMUの起動起動オプションにNICの設定を追加

$HOME/osbook/devenv/run_image.sh
```
...
qemu-system-x86_64 \
    -m 1G \
    -drive if=pflash,format=raw,readonly,file=$DEVENV_DIR/OVMF_CODE.fd \
    -drive if=pflash,format=raw,file=$DEVENV_DIR/OVMF_VARS.fd \
    -drive if=ide,index=0,media=disk,format=raw,file=$DISK_IMG \
    -device nec-usb-xhci,id=xhci \
    -device usb-mouse -device usb-kbd \
+   -nic tap,ifname=tap0,model=e1000e,script=no,downscript=no \
    -monitor stdio \
    $QEMU_OPTS
```

### Tapデバイスの作成

```
$ sudo ip tuntap add mode tap user $USER name tap0
$ sudo ip addr add 192.0.2.1/24 dev tap0
$ sudo ip link set tap0 up
```

## ビルド＆起動

オリジナルの MikanOS と同じ手順です。
```
$ ./build.sh run
```

起動後に MikanOS のターミナルで ifconfig を実行してIPアドレスを割り当てます。
> ネットワークインタフェースは `net0` `net1` `net2` ... という名称が割り当てられます。

```
> ifconfig net0 192.0.2.2/24
```
IPアドレスと割り当てると ping へ応答できるようになります。

## アプリケーション

+ ifconfig
+ udps
+ tcps

# MikanOS

MikanOS はレガシーフリーなアーキテクチャ（UEFI BIOS、Intel 64 モード）で動作する教育用オペレーティングシステムです。

## ファイル構成

- MikanLoaderPkg
    - UEFI アプリとして構成したブートローダ
- kernel
    - MikanOS のカーネル
- resource/nihongo.ttf
    - IPA ゴシックのフォントファイル
- IPA_Font_License_Agreement_v1.0.txt
    - IPA フォントのライセンス文書

## ビルド方法

[mikanos-build リポジトリ](https://github.com/uchan-nos/mikanos-build/) に MikanOS をビルドするためのスクリプトがあります。
mikanos-build の手順に沿って開発ツールを導入した後、devenv/buildenv.sh を読み込むことでビルド可能です。
（devenv/buildenv.sh により環境変数 CPPFLAGS などが適切に設定されます。）

MikanOS の最新版をビルドするためには mikanos-build の最新版が必要です。

## 教科書

MikanOS の作り方を説明した教科書があります。
[ゼロからのOS自作入門](https://zero.osdev.jp/)

## スクリーンショット

「ゼロからのOS自作入門」の最終章を終えたときの姿
![30章後の姿](mikanos-after30-photo.png)

## 開発への参加

MikanOS への機能追加、バグ修正の提案は Pull Request にてお願いします。

実装が伴わない「単なる要望」は基本的に受け付けません。
実装をきちんと作ってから Pull Request を提出してください。

もし、実装したいけど力が不足して実装できない、という場合はお気軽に Issues でご連絡ください。
実装ができるようになるように、できるだけご協力いたします。
