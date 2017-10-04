/*
* PackageLicenseDeclared: Apache-2.0
* Copyright (c) 2017 ARM Limited
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "mbed.h"
#include "BDFile.h"
#include "janpatch.h"
#include "patch.h"
#include "AT45BlockDevice.h"
#include "FragmentationSha256.h"
#include "update_params.h"

int main()
{
    // 1. Put original file in 0x1900
    // 2. Put patch file in 0x1801
    // 3. Calculate the new file and put in 0x2000
    // 4. Set update params, and see if we can update...
    AT45BlockDevice at45;
    at45.init();

    // OK, let's give it a try...
    int r;
    r = at45.program(ORIGINAL, 0x1900 * at45.get_read_size(), sizeof(ORIGINAL));
    printf("Programming ORIGINAL in page 0x1900, r=%d\n", r);

    printf("sizeof patch is %d\n", sizeof(PATCH));

    r = at45.program(PATCH, 0x1801 * at45.get_read_size(), sizeof(PATCH));
    printf("Programming PATCH in page 0x1801, r=%d\n", r);

    BDFILE originalFile(&at45, 0x1900 * at45.get_read_size(), sizeof(ORIGINAL));
    BDFILE patchFile(&at45, 0x1801 * at45.get_read_size(), sizeof(PATCH));
    BDFILE newFile(&at45, 0x2000 * at45.get_read_size(), 0);

    printf("HELLO WORLD\n");

    janpatch_ctx ctx = {
        (char*)malloc(1024),
        1024,       // buffer size

        &bd_getc,
        &bd_putc,
        &bd_fread,
        &bd_fwrite,
        &bd_fseek,
        &bd_ftell
    };

    /* Go... */
    janpatch(ctx, &originalFile, &patchFile, &newFile);

    printf("new size is %ld\n", newFile.ftell());

    uint8_t* sha_buffer = (uint8_t*)malloc(528);
    unsigned char sha_out[32];

    FragmentationSha256 sha256(&at45, sha_buffer, sizeof(sha_buffer));
    // first 256 bytes are the RSA/SHA256 signature, ignore those
    sha256.calculate(
        0x2000 * at45.get_read_size(),
        newFile.ftell(),
        sha_out);

    free(sha_buffer);

    printf("SHA256 hash is: ");
    for (size_t ix = 0; ix < 32; ix++) {
        printf("%02x", sha_out[ix]);
    }
    printf("\n");

    bool matches = true;
    for (size_t ix = 0; ix < 32; ix++) {
        if (sha_out[ix] != SHA256_HASH[ix]) {
            matches = false;
        }
    }

    if (!matches) {
        printf("SHA256 hash does not match. Abort!\n");
        return 1;
    }

    // Hash is matching, now populate the FOTA_INFO_PAGE with information about the update, so the bootloader can flash the update
    UpdateParams_t update_params;
    update_params.update_pending = 1;
    update_params.size = newFile.ftell();
    update_params.offset = 0x2000 * at45.get_read_size();
    update_params.signature = UpdateParams_t::MAGIC;
    memcpy(update_params.sha256_hash, sha_out, sizeof(sha_out));
    at45.program(&update_params, FOTA_INFO_PAGE * at45.get_read_size(), sizeof(UpdateParams_t));

    printf("Stored the update parameters in flash on page 0x%x. Reset the board to apply update.\n", FOTA_INFO_PAGE);

    printf("I'm done!\n");

    exit(0);
}
