/**
* Licence
* -------
*
* This program is free software. Terms of the GNU General Public License apply.
*
* This program is distributed WITHOUT ANY WARRANTY, without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.a
*
* A copy of the GNU General Public License if found in the file "Licence.txt"
* deliverd along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
* Parts or all of this source code may only be reused within other GNU free, open
* source, software.
* So if your project is not an open source project, you are non entitled to read
* any further below this line!
*
*******************************************************************************/

#ifndef _JPATCH_BD_FILE_
#define _JPATCH_BD_FILE_

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
    int fseek(int pos, uint8_t origin) {
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

    /**
     * Retrieves character on current position and ups position of the stream with 1
     */
    int getc() {
        if (current_pos >= size) return -1;

        char buff[1];
        if (bd->read(buff, current_pos + offset, 1) != 0) {
            return -1;
        }
        current_pos++;
        return buff[0];
    }

    /**
     * Writes a character to the stream, ups the position with 1
     * @param c Character to write
     */
    int putc(char c) {
        char buff[] = { c };
        if (bd->program(buff, current_pos + offset, 1) != 0) {
            return -1;
        }
        current_pos++;
        if (current_pos > size) {
            size = current_pos; // not really sure if this is OK
        }
        return 0;
    }

    int fread(void *buffer, size_t elements, size_t size) {
        int r = bd->read(buffer, offset + current_pos, elements * size);
        if (r != 0) return -1;

        current_pos += (elements * size);

        return elements * size;
    }

    int fwrite(void *buffer, size_t elements, size_t size) {
        int r = bd->program(buffer, offset + current_pos, elements * size);
        if (r != 0) return -1;

        current_pos += (elements * size);

        return elements * size;
    }

    int ftell() {
        return current_pos;
    }

private:
    BlockDevice* bd;
    size_t offset;
    size_t size;
    int current_pos;
};

// Functions similar to the POSIX functions
int bd_fseek(BDFILE *file, int pos, uint8_t origin) {
    return file->fseek(pos, origin);
}

int bd_getc(BDFILE *file) {
    return file->getc();
}

int bd_putc(char c, BDFILE *file) {
    return file->putc(c);
}

int bd_fread(void *buffer, size_t elements, size_t size, BDFILE *file) {
    return file->fread(buffer, elements, size);
}

int bd_fwrite(void *buffer, size_t elements, size_t size, BDFILE *file) {
    return file->fwrite(buffer, elements, size);
}

#endif // _JPATCH_BD_FILE_
