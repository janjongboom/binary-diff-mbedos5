# Binary diff application demo

Using [JojoDiff](http://jojodiff.sourceforge.net). Does binary diffing using BlockDevice on Mbed OS 5.

Takes:

* Source binary.
* Diff file.

Produces:

* A new binary.

This application does not use dynamically allocated memory. See the `BLKSZE` macro.

## Usage

1. Open `patch.h` and put the old file, the patch, and the SHA256 hash of the new file (see `demo/*.js` for scripts).
1. Build this application:

    ```
    $ mbed compile -m xdot_l151cc -t GCC_ARM
    ```

1. Run the application and inspect serial output (on baud rate 9,600).
1. When the application is done, restart the board. The new binary should be running.

## How to create diffs

* Build jdiff from source.
* Generate a patch file:

    ```
    $ ./jdiff blinky-xdot-old.bin blinky-xdot-new.bin > blinky-xdot.patch
    ```

## License

This application is licensed under GPLv3, due to JojoDiff being licensed under GPL. I've made [a request](https://sourceforge.net/p/jojodiff/discussion/190623/thread/39eb393e/) to re-license under LGPL, so this program can be licensed under Apache 2.0, like the rest of Mbed.
