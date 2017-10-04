# Binary diff application demo - SD card

Using [JANPatch](https://github.com/janjongboom/janpatch). Does binary patching of firmware using FATFileSystem on Mbed OS 5.

**[There is also a demo which uses BlockDevice instead.](http://github.com/janjongboom/binary-diff-mbedos5/tree/xdot)**

Takes:

* Source binary.
* Diff file.

Produces:

* A new binary.

This application does not use dynamically allocated memory. See macro in `main.cpp`.

## Usage

1. Put `demo/blinky-k64f-old.bin` and `demo/blinky.patch` on an SD card.
1. Put SD card in FRDM-K64F development board.
1. Build this application:

    ```
    $ mbed compile -m K64F -t GCC_ARM
    ```

1. Run the application and inspect serial output (on baud rate 9,600).
1. When the application is done, inspect the SD card. `blinky-k64f-new.bin` should have appeared, and it should match the file in the demo directory:

    ```
    $ diff /Volumes/SD/blinky-k64f-new.bin demo/blinky-k64f-new.bin

    # should not return anything
    ```

## How to create diffs

See the documentation for [JANPatch](https://github.com/janjongboom/janpatch).
