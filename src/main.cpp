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

#include "mbed.h"
#include "jpatch.h"
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

  /* Go... */
  jpatch(&originalFile, &patchFile, &newFile);

  printf("new size is %d\n", newFile.ftell());

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
