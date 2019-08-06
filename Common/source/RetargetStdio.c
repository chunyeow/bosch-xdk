/***************************************************************************//**
 * @file
 * @brief Provide stdio retargeting for all supported toolchains.
 * @author Energy Micro AS
 * @version 3.20.2
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2012 Energy Micro AS, http://www.energymicro.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 4. The source and compiled code may only be used on Energy Micro "EFM32"
 *    microcontrollers and "EFR4" radios.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 *****************************************************************************/

/***************************************************************************//**
 * @addtogroup RetargetIo
 * @brief This module provide low-level stubs for retargetting stdio for all
 *    supported toolchains.
 *    The stubs are minimal yet sufficient implementations.
 *    Refer to chapter 12 in the reference manual for newlib 1.17.0
 *    for details on implementing newlib stubs.
 ******************************************************************************/

#include "XdkCommonInfo.h"
#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID XDK_COMMON_ID_RETARGETSTDIO

extern int RETARGET_ReadChar(void);
extern int RETARGET_WriteChar(const char * data,int length);

#if (BCDS_EMLIB_INCLUDE_USB == 0)
int RETARGET_ReadChar(void)
{
    return -1;
}
int RETARGET_WriteChar(const char * data,int length)
{
    (void) data;
    (void) length;
    return -1;
}
#endif
#if !defined(__CROSSWORKS_ARM) && defined(__GNUC__)

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "em_device.h"

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
int fileno(FILE *);
/** @endcond */

int _close(int file);
int _fstat(int file, struct stat *st);
int _isatty(int file);
int _lseek(int file, int ptr, int dir);
int _read(int file, char *ptr, int len);
caddr_t _sbrk(int incr);
extern int _write(int file, const char *ptr, int len);

extern char _end;                 /**< Defined by the linker */

/**************************************************************************//**
 * @brief
 *  Close a file.
 *
 * @param[in] file
 *  File you want to close.
 *
 * @return
 *  Returns 0 when the file is closed.
 *****************************************************************************/
int _close(int file)
{
  (void) file;
  return 0;
}

/**************************************************************************//**
 * @brief Exit the program.
 * @param[in] status The value to return to the parent process as the
 *            exit status (not used).
 *****************************************************************************/
void _exit (int status)
{
  (void) status;
  while (1) {}      /* Hang here forever... */
}

/**************************************************************************//**
 * @brief
 *  Status of an open file.
 *
 * @param[in] file
 *  Check status for this file.
 *
 * @param[in] st
 *  Status information.
 *
 * @return
 *  Returns 0 when st_mode is set to character special.
 *****************************************************************************/
int _fstat(int file, struct stat *st)
{
  (void) file;
  st->st_mode = S_IFCHR;
  return 0;
}

/**************************************************************************//**
 * @brief Get process ID.
 *****************************************************************************/
int _getpid(void)
{
  return 1;
}

/**************************************************************************//**
 * @brief
 *  Query whether output stream is a terminal.
 *
 * @param[in] file
 *  Descriptor for the file.
 *
 * @return
 *  Returns 1 when query is done.
 *****************************************************************************/
int _isatty(int file)
{
  (void) file;
  return 1;
}

/**************************************************************************//**
 * @brief Send signal to process.
 * @param[in] pid Process id (not used).
 * @param[in] sig Signal to send (not used).
 *****************************************************************************/
int _kill(int pid, int sig)
{
  (void)pid;
  (void)sig;
  return -1;
}

/**************************************************************************//**
 * @brief
 *  Set position in a file.
 *
 * @param[in] file
 *  Descriptor for the file.
 *
 * @param[in] ptr
 *  Poiter to the argument offset.
 *
 * @param[in] dir
 *  Directory whence.
 *
 * @return
 *  Returns 0 when position is set.
 *****************************************************************************/
int _lseek(int file, int ptr, int dir)
{
  (void) file;
  (void) ptr;
  (void) dir;
  return 0;
}

/**************************************************************************//**
 * @brief
 *  Read from a file.
 *
 * @param[in] file
 *  Descriptor for the file you want to read from.
 *
 * @param[in] ptr
 *  Pointer to the chacaters that are beeing read.
 *
 * @param[in] len
 *  Number of characters to be read.
 *
 * @return
 *  Number of characters that have been read.
 *****************************************************************************/
int _read(int file, char *ptr, int len)
{
  int c, rxCount = 0;

  (void) file;

  while (len--)
  {
    if ((c = RETARGET_ReadChar()) != -1)
    {
      *ptr++ = c;
      rxCount++;
    }
    else
    {
      break;
    }
  }

  if (rxCount <= 0)
  {
    return -1;                        /* Error exit */
  }

  return rxCount;
}

/**************************************************************************//**
 * @brief
 *  Increase heap.
 *
 * @param[in] incr
 *  Number of bytes you want increment the program's data space.
 *
 * @return
 *  Rsturns a pointer to the start of the new area.
 *****************************************************************************/
caddr_t _sbrk(int incr)
{
  static char       *heap_end;
  char              *prev_heap_end;
  static const char heaperr[] = "Heap and stack collision\n";

  if (heap_end == 0)
  {
    heap_end = &_end;
  }

  prev_heap_end = heap_end;
  if ((heap_end + incr) > (char*) __get_MSP())
  {
    _write(fileno(stdout), heaperr, strlen(heaperr));
    exit(1);
  }
  heap_end += incr;

  return (caddr_t) prev_heap_end;
}

/**************************************************************************//**
 * @brief
 *  Write to a file.
 *
 * @param[in] file
 *  Descriptor for the file you want to write to.
 *
 * @param[in] ptr
 *  Pointer to the text you want to write
 *
 * @param[in] len
 *  Number of characters to be written.
 *
 * @return
 *  Number of characters that have been written.
 *****************************************************************************/
int _write(int file, const char *ptr, int len)
{

  (void) file;

    RETARGET_WriteChar(ptr,len);

  return len;
}
#endif /* !defined( __CROSSWORKS_ARM ) && defined( __GNUC__ ) */
