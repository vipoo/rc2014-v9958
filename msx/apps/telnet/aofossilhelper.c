/*
--
-- UnapiHelper.c
--   UNAPI Abstraction functions.
--   Revision 0.60
--
-- Requires SDCC and Fusion-C library to compile
-- Copyright (c) 2020 Andres Ortiz and Oduvaldo Pavan Junior ( ducasp@gmail.com )
-- All rights reserved.
--
-- Redistribution and use of this source code or any derivative works, are
-- permitted provided that the following conditions are met:
--
-- 1. Redistributions of source code must retain the above copyright notice,
--    this list of conditions and the following disclaimer.
-- 2. Redistributions in binary form must reproduce the above copyright
--    notice, this list of conditions and the following disclaimer in the
--    documentation and/or other materials provided with the distribution.
-- 3. Redistributions may not be sold, nor may they be used in a commercial
--    product or activity without specific prior written permission.
-- 4. Source code of derivative works MUST be published to the public.
--
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
-- "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
-- TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
-- PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
-- CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
-- EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
-- PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
-- OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
-- WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
-- OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
-- ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--
*/
#include "aofossilhelper.h"
#include "fossil_interface.h"
#include "msx_fusion.h"
#include "print.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char          chHelperString[128];
unsigned char ucFossilUnsafeDataTXBuffer[128];
const char    modem_atz[] = "ATZ\r\n";
const char    modem_cmd[] = "+++";
const char    modem_disc[] = "ATH\r\n";
unsigned char cmdline[134];

void Breath() { return; }

unsigned char InitializeTCPIP() {
  unsigned char uchRet = 0;
  if (fossil_link()) {
    fossile_set_baud(9, 9);
    fossil_set_protocol(7); // 8N1
    // Fossil_FastInt(0);
    fossil_init();
    TxData(0x50, modem_atz, 5);
    uchRet = 1;
  }

  return uchRet;
}

unsigned char OpenSingleConnection(unsigned char *uchHost, unsigned char *uchPort, unsigned char *uchConn) {
  if ((uchHost) && (uchPort)) {
    sprintf(cmdline, "ATD\"%s:%s\"\r\n", uchHost, uchPort);
    // print(cmdline);
    TxData(0x50, cmdline, strlen(cmdline));
  }
  *uchConn = 0x50;
  return ERR_OK;
}

unsigned char CloseConnection(unsigned char ucConnNumber) {
  unsigned char uchRet = 0;
  unsigned char ucCount = 0;

  if (ucConnNumber == 0x50) {
    TxData(0x50, modem_cmd, 3);
    do {
      __asm halt __endasm;

      ++ucCount;
    } while (ucCount < 20);
    fossil_deinit();
  } else
    uchRet = ERR_INV_PARAM;

  return uchRet;
}

unsigned char IsConnected(unsigned char ucConnNumber) {
  if (ucConnNumber == 0x50)
    return 1;
  else
    return ERR_INV_PARAM;
}

// This routine retrieves as much as bytes as indicated in uiSize
// Note that uiSize=1024 in normal characters receiving mode (RcvMemorySize=1024 in telnet.h)
// Receives up to uiSize bytes
// Number of bytes retrieved from serial port are returned into uiSize
unsigned char RXData(unsigned char ucConnNumber, unsigned char *ucBuffer, unsigned int *uiSize, unsigned char ucWaitAllDataReceived) {
  unsigned char ucRet = 0;
  unsigned int  nbytes = 0;
  unsigned int  tbytes = *uiSize;

  if (ucConnNumber != 0x50)
    return ERR_INV_PARAM;

  if (ucWaitAllDataReceived) {
    // While bytes are available and we are recevied less bytes than requested...
    while ((fossil_rs_in_stat() != 0) && (nbytes < *uiSize)) {
      ucRet = 1;
      fossil_rs_in_stat();
      ucBuffer[nbytes] = fossil_rs_in();
      nbytes++;
    }
    *uiSize = nbytes;
  } else {
    if (fossil_rs_in_stat() != 0) {
      ucRet = 1;
      *uiSize = fossil_chars_in_buf();
      for (int i = 0; i < *uiSize; i++) {
        fossil_rs_in_stat();
        ucBuffer[i] = fossil_rs_in();
      }
    } else
      *uiSize = 0;
  }
  return ucRet;
}

// This routine sends only one byte
unsigned char TxByte(unsigned char ucConnNumber, unsigned char uchByte) { return TxData(ucConnNumber, &uchByte, 1); }

unsigned char TxUnsafeData(unsigned char ucConnNumber, unsigned char *lpucData, unsigned int uiDataSize) { return TxData(ucConnNumber, lpucData, uiDataSize); }

// The same as TxUnsafeData but without page 3 buffer addressing
unsigned char TxData(unsigned char ucConnNumber, unsigned char *lpucData, unsigned int uiDataSize) {
  if (ucConnNumber != 0x50)
    return ERR_INV_PARAM;
  for (int i = 0; i < uiDataSize; i++) {
    fossil_rs_out(*lpucData);
    *lpucData++;
  }
  return ERR_OK;
}
