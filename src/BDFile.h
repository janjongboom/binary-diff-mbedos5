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

#ifndef _JANPATCH_BD_FILE_
#define _JANPATCH_BD_FILE_

#include "BlockDevice.h"

// So, jpatch uses POSIX FS calls, let's emulate them, but backed by BlockDevice driver

class BDFILE {
public:
    /**
     * Creates a new BDFILE
     * @param _bd Instance of a BlockDevice
     * @param _offset Offset of the file in flash
     * @param _size Size of the file in flash
     */
    BDFILE(BlockDevice* _bd, size_t _offset, size_t _size) :
        bd(_bd), offset(_offset), size(_size), current_pos(0)
    {

    }

    /**
     * Sets position in the file
     * @param pos New position
     * @param origin Seek position
     */
    int fseek(long int pos, int origin) {
        switch (origin) {
            case SEEK_SET: { // from beginning
                current_pos = pos;
                break;
            }
            case SEEK_CUR: {
                current_pos += pos;
                break;
            }
            case SEEK_END: {
                current_pos = size + pos;
                break;
            }
            default: return -1;
        }

        if (current_pos < 0) return -1;
        if (current_pos > size) return -1;
        return 0;
    }

    size_t fread(void *buffer, size_t elements, size_t element_size) {
        size_t bytes_to_read = elements * element_size;
        if (current_pos + bytes_to_read > size) {
            bytes_to_read = size - current_pos; // don't read over file boundary
        }

        int r = bd->read(buffer, offset + current_pos, bytes_to_read);
        if (r != 0) return -1;

        current_pos += bytes_to_read;

        return bytes_to_read;
    }

    size_t fwrite(const void *buffer, size_t elements, size_t element_size) {
        int r = bd->program(buffer, offset + current_pos, elements * element_size);
        if (r != 0) return -1;

        current_pos += (elements * element_size);

        if (current_pos > size) size = current_pos;

        return elements * element_size;
    }

    long int ftell() {
        return current_pos;
    }

private:
    BlockDevice* bd;
    size_t offset;
    size_t size;
    int current_pos;
};

// Functions similar to the POSIX functions
int bd_fseek(BDFILE *file, long int pos, int origin) {
    return file->fseek(pos, origin);
}

long int bd_ftell(BDFILE *file) {
    return file->ftell();
}

size_t bd_fread(void *buffer, size_t elements, size_t size, BDFILE *file) {
    return file->fread(buffer, elements, size);
}

size_t bd_fwrite(const void *buffer, size_t elements, size_t size, BDFILE *file) {
    return file->fwrite(buffer, elements, size);
}

#endif // _JANPATCH_BD_FILE_
