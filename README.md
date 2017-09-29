# Binary diff application demo

Using [JojoDiff](http://jojodiff.sourceforge.net). Does binary diffing using FATFileSystem on Mbed OS 5.

Takes:

* Source binary.
* Diff file.

Produces:

* A new binary.

This application does not use dynamically allocated memory. See the `BLKSZE` macro.

## Usage

1. Put `demo/blinky-k64f-old.bin` and `demo/blinky.patch` on an SD card.
1. Put SD card in FRDM-K64F development board.
1. Build this application:

    ```
    $ mbed compile -m K64F -t GCC_ARM
    ```

1. Run the application and inspect serial output (on baud rate 9,600).
1. When the application is done, inspect the SD card. `blinky-k64f-new.bin` should have appeared, and it should match the file in the demo directory.

## How to create diffs

* Build jdiff from source.
* Generate a patch file:

    ```
    $ ./jdiff blinky-k64f-old.bin blinky-k64f-new.bin > blinky-k64f.patch
    ```

## License

This application is licensed under GPLv3, due to JojoDiff being licensed under GPL. I've made [a request](https://sourceforge.net/p/jojodiff/discussion/190623/thread/39eb393e/) to re-license under LGPL, so this program can be licensed under Apache 2.0, like the rest of Mbed.
