#include "mbed.h"
#include "SDBlockDevice.h"
#include "FATFileSystem.h"
#include "janpatch.h"

SDBlockDevice sd(MBED_CONF_APP_SD_CARD_MOSI, MBED_CONF_APP_SD_CARD_MISO,
    MBED_CONF_APP_SD_CARD_SCK, MBED_CONF_APP_SD_CARD_CS);
FATFileSystem fs("sd");

void progress(uint8_t percentage) {
    printf("Progress: %d%%\n", percentage);
}

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

    printf("Opened files? source=%p patch=%p target=%p\n", source, patch, target);
    if (source == NULL || patch == NULL || target == NULL) {
        return 1;
    }

    printf("Patching...\n");

    // janpatch_ctx contains buffers, and references to the file system functions
    janpatch_ctx ctx = {
        { (unsigned char*)malloc(1024), 1024 },
        { (unsigned char*)malloc(1024), 1024 },
        { (unsigned char*)malloc(1024), 1024 },

        &fread,
        &fwrite,
        &fseek,
        &ftell,

        &progress
    };

    int jpr = janpatch(ctx, source, patch, target);
    if (jpr != 0) {
        printf("Patching failed... (%d)\n", jpr);
        return 1;
    }

    // Close files
    fclose(source);
    fclose(patch);
    fclose(target);

    printf("Done!\n");
}
