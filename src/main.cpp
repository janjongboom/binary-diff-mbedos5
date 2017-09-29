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
#include "SDBlockDevice.h"
#include "FATFileSystem.h"
#include "jpatch.h"

SDBlockDevice sd(MBED_CONF_APP_SD_CARD_MOSI, MBED_CONF_APP_SD_CARD_MISO,
    MBED_CONF_APP_SD_CARD_SCK, MBED_CONF_APP_SD_CARD_CS);
FATFileSystem fs("sd");

int main()
{
  sd.init();
  fs.mount(&sd);

  const char *lcFilNamOrg = "/sd/blinky-k64f-old.bin";
  const char *lcFilNamPch = "/sd/blinky.patch";
  const char *lcFilNamOut = "/sd/blinky-k64f-new.bin";

  FILE *lpFilOrg = fopen(lcFilNamOrg, "rb");
  FILE *lpFilPch = fopen(lcFilNamPch, "rb");
  FILE *lpFilOut = fopen(lcFilNamOut, "wb");

  printf("made files? %p %p %p\n", lpFilOrg, lpFilPch, lpFilOut);

  int lbOptArgDne=false;
  int liOptArgCnt=0;
  char lcHlp='\0';

  printf("HELLO WORLD\n");

  /* Go... */
  jpatch(lpFilOrg, lpFilPch, lpFilOut);

  /* Close files */
  fclose(lpFilOrg);
  fclose(lpFilPch);
  fclose(lpFilOut);

  printf("I'm done!\n");

  exit(0);
}
