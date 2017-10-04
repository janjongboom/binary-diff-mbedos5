# Binary diff application demo - AT45 flash

Using [JANPatch](https://github.com/janjongboom/janpatch). Does binary patching of firmware using BlockDevice on Mbed OS 5.

Takes:

* Source binary.
* Diff file.

Produces:

* A new binary.

It then flashes the new binary using the bootloader (see `bootloader` folder). The bootloader source code is [here](http://github.com/janjongboom/lorawan-at45-fota-bootloader).

This application does not use dynamically allocated memory. See macro in `main.cpp`.

## Usage

1. Open `patch.h` and put the old file, the patch, and the SHA256 hash of the new file (see `demo/*.js` for scripts).
1. Build this application:

    ```
    $ mbed compile -m xdot_l151cc -t GCC_ARM
    ```

1. Run the application and inspect serial output (on baud rate 9,600).
1. When the application is done, restart the board. The new binary should be running.

## How to create diffs

See the documentation for [JANPatch](https://github.com/janjongboom/janpatch).
