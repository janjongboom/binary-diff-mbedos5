#include "mbed.h"
#include "SDBlockDevice.h"
#include "FATFileSystem.h"
#include "janpatch.h"

SDBlockDevice sd(MBED_CONF_APP_SD_CARD_MOSI, MBED_CONF_APP_SD_CARD_MISO,
    MBED_CONF_APP_SD_CARD_SCK, MBED_CONF_APP_SD_CARD_CS);
FATFileSystem fs("sd");

int main()
{
    sd.init();
    fs.mount(&sd);

    const char *sourcePath = "/sd/blinky-k64f-old.bin";
    const char *patchPath  = "/sd/blinky.patch";
    const char *targetPath = "/sd/blinky-k64f-new.bin";

    FILE *source = fopen(sourcePath, "rb");
    FILE *patch  = fopen(patchPath, "rb");
    FILE *target = fopen(targetPath, "wb");

    printf("made files? %p %p %p\n", source, patch, target);

    // janpatch_ctx contains buffers, and references to the file system functions
    janpatch_ctx ctx = {
        (char*)malloc(1024),
        1024,

        &getc,
        &putc,
        &fread,
        &fwrite,
        &fseek,
        &ftell
    };

    janpatch(ctx, source, patch, target);

    // Close files
    fclose(source);
    fclose(patch);
    fclose(target);

    printf("I'm done!\n");
}
