/*******************************************************************************
* Jojo's Patch: apply patch from jdiff to a binary file
*
* Copyright (C) 2002-2009 Joris Heirbaut
*
* Author                Version Date       Modification
* --------------------- ------- -------    -----------------------
* Joris Heirbaut        v0.0    10-06-2002 hashed compare
* Joris Heirbaut        v0.1    20-06-2002 optimized esc-sequences & lengths
* Joris Heirbaut        v0.4c   09-01-2003 use seek for DEL-instruction
* Joris Heirbaut        v0.5    13-05-2003 do not count past EOF
* Joris Heirbaut        v0.6    13-05-2005 large-file support
* Joris Heirbaut        v0.7    01-09-2009 large-file support
* Joris Heirbaut        v0.7    29-10-2009 use buffered reading
* Joris Heirbaut        v0.7    01-11-2009 do not read original file on MOD
* Joris Heirbaut		v0.8	15-09-2011 C++ wrapping
*
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "JDefs.h"

#define BLKSZE 1024

/*
 * Global settings (may be modified by commandline options)
 */
int giVerbse = 0;       /* Verbose level 0=no, 1=normal, 2=high            */
int gbTst = false;      /* Test mode = only display contents of patch file, no i/o */
FILE *stddbg;           /* Debug output to stddbg or stdout                */

/*******************************************************************************
* Input routines
*******************************************************************************/
#define ESC     0xA7
#define MOD     0xA6
#define INS     0xA5
#define DEL     0xA4
#define EQL     0xA3
#define BKT     0xA2

off_t ufGetInt( FILE *lpFil ){
  off_t liVal ;

  liVal = getc(lpFil) ;
  if (liVal < 252)
    return liVal + 1 ;
  else if (liVal == 252)
    return 253 + getc(lpFil) ;
  else if (liVal == 253) {
    liVal = getc(lpFil) ;
    liVal = (liVal << 8) + getc(lpFil) ;
    return liVal ;
  }
  else if (liVal == 254) {
    liVal = getc(lpFil) ;
    liVal = (liVal << 8) + getc(lpFil) ;
    liVal = (liVal << 8) + getc(lpFil) ;
    liVal = (liVal << 8) + getc(lpFil) ;
    return liVal ;
  } else {
#ifdef JDIFF_LARGEFILE
    liVal = getc(lpFil) ;
    liVal = (liVal << 8) + getc(lpFil) ;
    liVal = (liVal << 8) + getc(lpFil) ;
    liVal = (liVal << 8) + getc(lpFil) ;
    liVal = (liVal << 8) + getc(lpFil) ;
    liVal = (liVal << 8) + getc(lpFil) ;
    liVal = (liVal << 8) + getc(lpFil) ;
    liVal = (liVal << 8) + getc(lpFil) ;
    return liVal ;
#else
    fprintf(stderr, "64-bit length numbers not supported!\n");
    exit(EXI_LRG);
#endif
  }
}

/*******************************************************************************
* Patch function
*******************************************************************************
* Input stream consists of a series of
*   <op> (<data> || <len>)
* where
*   <op>   = <ESC> (<MOD>||<INS>||<DEL>||<EQL>)
*   <data> = <chr>||<ESC><ESC>
*   <chr>  = any byte different from <ESC><MOD><INS><DEL> or <EQL>
*   <ESC><ESC> yields one <ESC> byte
*******************************************************************************/
void jpatch ( FILE *asFilOrg, FILE *asFilPch, FILE *asFilOut )
{
  int liInp ;         /* Current input from patch file          */
  int liOpr ;         /* Current operand                        */
  off_t lzOff ;       /* Current operand's offset               */
  off_t lzMod = 0;    /* Number of bytes to skip on MOD         */
  int lbChg=false ;   /* Changing operand?                      */
  int lbEsc=false ;   /* Non-operand escape char found?	        */

  uchar lcDta[BLKSZE];

  size_t processed = 0;

  liOpr = ESC ;
  while ((liInp = getc(asFilPch)) != EOF) {
	// Parse an operator: ESC liOpr [lzOff]
    if (liInp == ESC) {
      liInp = getc(asFilPch);
      switch(liInp) {
        case MOD:
          liOpr = MOD;
          if (giVerbse == 1) {
            fprintf(stddbg, ""P8zd" "P8zd" MOD ...    \n", ftell(asFilOrg)+lzMod-1, ftell(asFilOut)) ;
          }
          lbChg = true;
          break ;

        case INS:
          liOpr = INS;
          if (giVerbse == 1) {
            fprintf(stddbg, ""P8zd" "P8zd" INS ...    \n",
                    ftell(asFilOrg)+lzMod-1, ftell(asFilOut))   ;
          }
          lbChg = true;
          break ;

        case DEL:
          liOpr = DEL;
          lzOff = ufGetInt(asFilPch);
          if (giVerbse >= 1) {
            fprintf(stddbg, ""P8zd" "P8zd" DEL %"PRIzd"\n",
                    ftell(asFilOrg)+lzMod, ftell(asFilOut), lzOff)  ;
          }

          if (fseek(asFilOrg, lzOff + lzMod, SEEK_CUR) != 0) {
            fprintf(stderr, "Could not position on original file (seek %"PRIzd" + %"PRIzd").\n", lzOff, lzMod);
            exit(EXI_SEK);
          }
          lzMod = 0;
          lbChg = true;
          break ;

        case EQL:
          liOpr = EQL;
          lzOff = ufGetInt(asFilPch);
          if (giVerbse >= 1) {
            fprintf(stddbg, ""P8zd" "P8zd" EQL %"PRIzd"\n",
                    ftell(asFilOrg)+lzMod, ftell(asFilOut), lzOff) ;
          }

          if (lzMod > 0) {
              if (fseek(asFilOrg, lzMod, SEEK_CUR) != 0) {
                  fprintf(stderr, "Could not position on original file (skip %"PRIzd").\n", lzMod);
                  exit(EXI_SEK);
              }
              lzMod = 0;
          }
          while (lzOff > BLKSZE) {
              if (fread(&lcDta, 1, BLKSZE, asFilOrg ) != BLKSZE) {
                  fprintf(stderr, "Error reading original file.\n");
                  exit(EXI_RED);
              }
              if (fwrite(&lcDta, 1, BLKSZE, asFilOut) != BLKSZE) {
                  fprintf(stderr, "Error writing output file.\n");
                  exit(EXI_WRI);
              }

              processed += BLKSZE;

              printf("Processed %lu bytes\n", processed);
              lzOff-=BLKSZE;
          }
          if (lzOff > 0){
              if (fread(&lcDta, 1, lzOff, asFilOrg) != lzOff) {
                  fprintf(stderr, "Error reading original file.\n");
                  exit(EXI_RED);
              }
              if (fwrite(&lcDta, 1, lzOff, asFilOut) != lzOff) {
                  fprintf(stderr, "Error writing output file.\n");
                  exit(EXI_WRI);
              }
          }
          lbChg = true;
          break ;

        case BKT:
          liOpr = BKT ;
          lzOff = ufGetInt(asFilPch) ;
          if (giVerbse >= 1) {
            fprintf(stddbg, ""P8zd" "P8zd" BKT %"PRIzd"\n",
                    ftell(asFilOrg)+lzMod, ftell(asFilOut), lzOff)   ;
          }

          if (fseek(asFilOrg, lzMod - lzOff, SEEK_CUR) != 0) {
            fprintf(stderr, "Could not position on original file (seek back %"PRIzd" - %"PRIzd").\n",
                    lzMod, lzOff);
            exit(EXI_SEK);
          }
          lzMod = 0 ;
          lbChg = true;
          break ;

        case ESC:
          if (giVerbse > 2) {
            fprintf(stddbg, ""P8zd" "P8zd" ESC ESC\n",
                    ftell(asFilOrg)+lzMod, ftell(asFilOut)) ;
          }
          break;

        default:
          if (giVerbse > 2) {
            fprintf(stddbg, ""P8zd" "P8zd" ESC XXX\n",
                    ftell(asFilOrg)+lzMod, ftell(asFilOut)) ;
          }
          lbEsc = true;
          break;
      }
    }

    if ( lbChg ) {
      lbChg = false ;
    } else {
      switch (liOpr) {
        case DEL: break;
        case EQL: break;
        case BKT: break;
        case MOD:
          if (lbEsc) {
            putc(ESC, asFilOut) ;
            lzMod ++ ;
            if (giVerbse > 2) {
                fprintf(stddbg, P8zd" "P8zd" MOD %3o ESC\n",
                        ftell(asFilOrg)+lzMod-1, ftell(asFilOut)-1, ESC)  ;
            }
          }

          putc(liInp, asFilOut) ;
          lzMod ++ ;
          if (giVerbse > 2) {
              fprintf(stddbg, P8zd" "P8zd" MOD %3o %c\n",
                      ftell(asFilOrg)+lzMod-1, ftell(asFilOut)-1, liInp,
                      ((liInp >= 32 && liInp <= 127)?(char) liInp:' '))  ;
          }
          break ;

        case INS :
          if (lbEsc) {
            if (giVerbse > 2) {
                fprintf(stddbg, P8zd" "P8zd" INS %3o ESC\n",
                        ftell(asFilOrg)+lzMod-1, ftell(asFilOut), ESC)  ;
            }
            putc(ESC, asFilOut) ;
          }

          if (giVerbse > 2) {
              fprintf(stddbg, P8zd" "P8zd" INS %3o %c\n",
                      ftell(asFilOrg)+lzMod-1, ftell(asFilOut), liInp,
                      ((liInp >= 32 && liInp <= 127)?(char) liInp:' '))  ;
          }

          putc(liInp, asFilOut) ;
          break ;
      }
    } /* if lbChg */

    lbEsc = false ;
  } /* while */

  if (giVerbse > 1) {
      fprintf(stddbg, P8zd" "P8zd" EOF",
              ftell(asFilOrg)+lzMod, ftell(asFilOut))  ;
  }
}
