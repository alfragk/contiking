/*
 * Copyright (c) 2016, Benoît Thébaudeau <benoit.thebaudeau.dev@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup cc2538-examples
 * @{
 *
 * \defgroup cc2538-cbc-mac-test cc2538dk AES-CBC-MAC Test Project
 *
 *   AES-CBC-MAC access example for CC2538 on SmartRF06EB.
 *
 *   This example shows how AES-CBC-MAC should be used. The example also
 *   verifies the AES-CBC-MAC functionality.
 *
 * @{
 *
 * \file
 *     Example demonstrating AES-CBC-MAC on the cc2538dk platform
 */
#include "contiki.h"
#include "sys/rtimer.h"
#include "dev/rom-util.h"
#include "dev/cbc-mac.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define MDATA_MAX_LEN   160
/*---------------------------------------------------------------------------*/
PROCESS(cbc_mac_test_process, "cbc_mac test process");
AUTOSTART_PROCESSES(&cbc_mac_test_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cbc_mac_test_process, ev, data)
{
  static const char *const str_res[] = {
    "success",
    "invalid param",
    "NULL error",
    "resource in use",
    "DMA bus error",
    "keystore read error",
    "keystore write error",
    "authentication failed"
  };
  static const uint8_t keys128[][128 / 8] = {
    { 0x1f, 0x8e, 0x49, 0x73, 0x95, 0x3f, 0x3f, 0xb0,
      0xbd, 0x6b, 0x16, 0x66, 0x2e, 0x9a, 0x3c, 0x17 },
    { 0xb7, 0xf3, 0xc9, 0x57, 0x6e, 0x12, 0xdd, 0x0d,
      0xb6, 0x3e, 0x8f, 0x8f, 0xac, 0x2b, 0x9a, 0x39 },
    { 0x89, 0xa5, 0x53, 0x73, 0x04, 0x33, 0xf7, 0xe6,
      0xd6, 0x7d, 0x16, 0xd3, 0x73, 0xbd, 0x53, 0x60 },
    { 0x2c, 0x14, 0x41, 0x37, 0x51, 0xc3, 0x1e, 0x27,
      0x30, 0x57, 0x0b, 0xa3, 0x36, 0x1c, 0x78, 0x6b },
    { 0x6a, 0x70, 0x82, 0xcf, 0x8c, 0xda, 0x13, 0xef,
      0xf4, 0x8c, 0x81, 0x58, 0xdd, 0xa2, 0x06, 0xae },
    { 0x7b, 0x1a, 0xb9, 0x14, 0x4b, 0x02, 0x39, 0x31,
      0x5c, 0xd5, 0xee, 0xc6, 0xc7, 0x56, 0x63, 0xbd },
    { 0xba, 0xb0, 0xcc, 0xed, 0xdc, 0x0a, 0xbd, 0x63,
      0xe3, 0xf8, 0x2e, 0x9f, 0xbf, 0xf7, 0xb8, 0xaa },
    { 0x97, 0xa1, 0x02, 0x55, 0x29, 0xb9, 0x92, 0x5e,
      0x25, 0xbb, 0xe7, 0x87, 0x70, 0xca, 0x2f, 0x99 }
  };
  static const uint8_t keys192[][192 / 8] = {
    { 0xba, 0x75, 0xf4, 0xd1, 0xd9, 0xd7, 0xcf, 0x7f,
      0x55, 0x14, 0x45, 0xd5, 0x6c, 0xc1, 0xa8, 0xab,
      0x2a, 0x07, 0x8e, 0x15, 0xe0, 0x49, 0xdc, 0x2c },
    { 0x16, 0x2a, 0xd5, 0x0e, 0xe6, 0x4a, 0x07, 0x02,
      0xaa, 0x55, 0x1f, 0x57, 0x1d, 0xed, 0xc1, 0x6b,
      0x2c, 0x1b, 0x6a, 0x1e, 0x4d, 0x4b, 0x5e, 0xee },
    { 0x8e, 0x27, 0x40, 0xfb, 0xa1, 0x57, 0xae, 0xf2,
      0x42, 0x2e, 0x44, 0x23, 0x12, 0xd1, 0x5c, 0x14,
      0xd3, 0x12, 0x55, 0x36, 0x84, 0xfc, 0xdc, 0x15 },
    { 0x50, 0x9b, 0xaf, 0x46, 0xfb, 0x9d, 0xe3, 0x42,
      0x81, 0xda, 0xfc, 0xc3, 0xdb, 0x79, 0x59, 0x3b,
      0xff, 0xa8, 0x42, 0x69, 0x04, 0x30, 0x26, 0x88 }
  };
  static const uint8_t keys256[][256 / 8] = {
    { 0x6e, 0xd7, 0x6d, 0x2d, 0x97, 0xc6, 0x9f, 0xd1,
      0x33, 0x95, 0x89, 0x52, 0x39, 0x31, 0xf2, 0xa6,
      0xcf, 0xf5, 0x54, 0xb1, 0x5f, 0x73, 0x8f, 0x21,
      0xec, 0x72, 0xdd, 0x97, 0xa7, 0x33, 0x09, 0x07 },
    { 0x48, 0xbe, 0x59, 0x7e, 0x63, 0x2c, 0x16, 0x77,
      0x23, 0x24, 0xc8, 0xd3, 0xfa, 0x1d, 0x9c, 0x5a,
      0x9e, 0xcd, 0x01, 0x0f, 0x14, 0xec, 0x5d, 0x11,
      0x0d, 0x3b, 0xfe, 0xc3, 0x76, 0xc5, 0x53, 0x2b },
    { 0x43, 0xe9, 0x53, 0xb2, 0xae, 0xa0, 0x8a, 0x3a,
      0xd5, 0x2d, 0x18, 0x2f, 0x58, 0xc7, 0x2b, 0x9c,
      0x60, 0xfb, 0xe4, 0xa9, 0xca, 0x46, 0xa3, 0xcb,
      0x89, 0xe3, 0x86, 0x38, 0x45, 0xe2, 0x2c, 0x9e },
    { 0x87, 0x72, 0x5b, 0xd4, 0x3a, 0x45, 0x60, 0x88,
      0x14, 0x18, 0x07, 0x73, 0xf0, 0xe7, 0xab, 0x95,
      0xa3, 0xc8, 0x59, 0xd8, 0x3a, 0x21, 0x30, 0xe8,
      0x84, 0x19, 0x0e, 0x44, 0xd1, 0x4c, 0x69, 0x96 }
  };
  static const struct {
    const void *keys;
    uint8_t key_size;
    uint8_t count;
  } keys[] = {
    { keys128, AES_KEY_STORE_SIZE_KEY_SIZE_128,
      sizeof(keys128) / sizeof(keys128[0]) },
    { keys192, AES_KEY_STORE_SIZE_KEY_SIZE_192,
      sizeof(keys192) / sizeof(keys192[0]) },
    { keys256, AES_KEY_STORE_SIZE_KEY_SIZE_256,
      sizeof(keys256) / sizeof(keys256[0]) }
  };
  static const struct {
    uint8_t key_size_index;
    uint8_t key_area;
    uint8_t mdata[MDATA_MAX_LEN];
    uint16_t mdata_len;
    uint8_t mac[CBC_MAC_MAC_LEN];
  } vectors[] = {
    {
      0, /* key_size_index */
      0, /* key_area */
      { 0x45, 0xcf, 0x12, 0x96, 0x4f, 0xc8, 0x24, 0xab,
        0x76, 0x61, 0x6a, 0xe2, 0xf4, 0xbf, 0x08, 0x22 }, /* mdata */
      16, /* mdata_len */
      { 0x97, 0x67, 0x0c, 0x83, 0x99, 0x24, 0xa4, 0xc6,
        0x15, 0xb0, 0xf6, 0x14, 0xae, 0x75, 0xa8, 0x69 } /* mac */
    }, {
      0, /* key_size_index */
      1, /* key_area */
      { 0x9a, 0xc1, 0x99, 0x54, 0xce, 0x13, 0x19, 0xb3,
        0x54, 0xd3, 0x22, 0x04, 0x60, 0xf7, 0x1c, 0x1e,
        0x37, 0x3f, 0x1c, 0xd3, 0x36, 0x24, 0x08, 0x81,
        0x16, 0x0c, 0xfd, 0xe4, 0x6e, 0xbf, 0xed, 0x2e,
        0x79, 0x1e, 0x8d, 0x5a, 0x1a, 0x13, 0x6e, 0xbd,
        0x1d, 0xc4, 0x69, 0xde, 0xc0, 0x0c, 0x41, 0x87,
        0x72, 0x2b, 0x84, 0x1c, 0xda, 0xbc, 0xb2, 0x2c,
        0x1b, 0xe8, 0xa1, 0x46, 0x57, 0xda, 0x20, 0x0e }, /* mdata */
      64, /* mdata_len */
      { 0x9d, 0x15, 0xb9, 0xfb, 0xff, 0xe2, 0x32, 0x97,
        0x56, 0x36, 0x36, 0x6d, 0x13, 0x65, 0x78, 0xb5 } /* mac */
    }, {
      0, /* key_size_index */
      2, /* key_area */
      { 0x80, 0x7b, 0xc4, 0xea, 0x68, 0x4e, 0xed, 0xcf,
        0xdc, 0xca, 0x30, 0x18, 0x06, 0x80, 0xb0, 0xf1,
        0xae, 0x28, 0x14, 0xf3, 0x5f, 0x36, 0xd0, 0x53,
        0xc5, 0xae, 0xa6, 0x59, 0x5a, 0x38, 0x6c, 0x14,
        0x42, 0x77, 0x0f, 0x4d, 0x72, 0x97, 0xd8, 0xb9,
        0x18, 0x25, 0xee, 0x72, 0x37, 0x24, 0x1d, 0xa8,
        0x92, 0x5d, 0xd5, 0x94, 0xcc, 0xf6, 0x76, 0xae,
        0xcd, 0x46, 0xca, 0x20, 0x68, 0xe8, 0xd3, 0x7a,
        0x3a, 0x0e, 0xc8, 0xa7, 0xd5, 0x18, 0x5a, 0x20,
        0x1e, 0x66, 0x3b, 0x5f, 0xf3, 0x6a, 0xe1, 0x97,
        0x11, 0x01, 0x88, 0xa2, 0x35, 0x03, 0x76, 0x3b,
        0x82, 0x18, 0x82, 0x6d, 0x23, 0xce, 0xd7, 0x4b,
        0x31, 0xe9, 0xf6, 0xe2, 0xd7, 0xfb, 0xfa, 0x6c,
        0xb4, 0x34, 0x20, 0xc7, 0x80, 0x7a, 0x86, 0x25 }, /* mdata */
      112, /* mdata_len */
      { 0x5f, 0x4f, 0x06, 0xe0, 0x62, 0x65, 0xf2, 0xeb,
        0x16, 0x1c, 0xda, 0x9f, 0x30, 0xe9, 0x79, 0x53 } /* mac */
    }, {
      0, /* key_size_index */
      3, /* key_area */
      { 0x40, 0xd9, 0x30, 0xf9, 0xa0, 0x53, 0x34, 0xd9,
        0x81, 0x6f, 0xe2, 0x04, 0x99, 0x9c, 0x3f, 0x82,
        0xa0, 0x3f, 0x6a, 0x04, 0x57, 0xa8, 0xc4, 0x75,
        0xc9, 0x45, 0x53, 0xd1, 0xd1, 0x16, 0x69, 0x3a,
        0xdc, 0x61, 0x80, 0x49, 0xf0, 0xa7, 0x69, 0xa2,
        0xee, 0xd6, 0xa6, 0xcb, 0x14, 0xc0, 0x14, 0x3e,
        0xc5, 0xcc, 0xcd, 0xbc, 0x8d, 0xec, 0x4c, 0xe5,
        0x60, 0xcf, 0xd2, 0x06, 0x22, 0x57, 0x09, 0x32,
        0x6d, 0x4d, 0xe7, 0x94, 0x8e, 0x54, 0xd6, 0x03,
        0xd0, 0x1b, 0x12, 0xd7, 0xfe, 0xd7, 0x52, 0xfb,
        0x23, 0xf1, 0xaa, 0x44, 0x94, 0xfb, 0xb0, 0x01,
        0x30, 0xe9, 0xde, 0xd4, 0xe7, 0x7e, 0x37, 0xc0,
        0x79, 0x04, 0x2d, 0x82, 0x80, 0x40, 0xc3, 0x25,
        0xb1, 0xa5, 0xef, 0xd1, 0x5f, 0xc8, 0x42, 0xe4,
        0x40, 0x14, 0xca, 0x43, 0x74, 0xbf, 0x38, 0xf3,
        0xc3, 0xfc, 0x3e, 0xe3, 0x27, 0x73, 0x3b, 0x0c,
        0x8a, 0xee, 0x1a, 0xbc, 0xd0, 0x55, 0x77, 0x2f,
        0x18, 0xdc, 0x04, 0x60, 0x3f, 0x7b, 0x2c, 0x1e,
        0xa6, 0x9f, 0xf6, 0x62, 0x36, 0x1f, 0x2b, 0xe0,
        0xa1, 0x71, 0xbb, 0xdc, 0xea, 0x1e, 0x5d, 0x3f }, /* mdata */
      160, /* mdata_len */
      { 0xda, 0x15, 0x2d, 0xf1, 0xf5, 0x6a, 0x44, 0x0e,
        0x1c, 0x7e, 0x27, 0x80, 0xed, 0x87, 0xd9, 0x3d } /* mac */
    }, {
      0, /* key_size_index */
      4, /* key_area */
      { 0xf8, 0xeb, 0x31, 0xb3, 0x1e, 0x37, 0x4e, 0x96,
        0x00, 0x30, 0xcd, 0x1c, 0xad, 0xb0, 0xef, 0x0c }, /* mdata */
      16, /* mdata_len */
      { 0x11, 0x0b, 0x18, 0xdb, 0xe0, 0x97, 0x56, 0x39,
        0xf7, 0x1d, 0x87, 0x7d, 0x6d, 0x07, 0x27, 0x55 } /* mac */
    }, {
      0, /* key_size_index */
      5, /* key_area */
      { 0xd3, 0xf8, 0x9b, 0x71, 0xe0, 0x33, 0x07, 0x0f,
        0x9d, 0x75, 0x16, 0xa6, 0xcb, 0x4e, 0xa5, 0xef,
        0x51, 0xd6, 0xfb, 0x63, 0xd4, 0xf0, 0xfe, 0xa0,
        0x89, 0xd0, 0xa6, 0x0e, 0x47, 0xbb, 0xb3, 0xc2,
        0xe1, 0x0e, 0x9b, 0xa3, 0xb2, 0x82, 0xc7, 0xcb,
        0x79, 0xae, 0xfe, 0x30, 0x68, 0xce, 0x22, 0x83,
        0x77, 0xc2, 0x1a, 0x58, 0xfe, 0x5a, 0x0f, 0x88,
        0x83, 0xd0, 0xdb, 0xd3, 0xd0, 0x96, 0xbe, 0xca }, /* mdata */
      64, /* mdata_len */
      { 0x6d, 0x91, 0x75, 0x52, 0x10, 0xcc, 0x69, 0x4a,
        0xde, 0x6c, 0xf6, 0xa0, 0xcd, 0x2d, 0x51, 0x8e } /* mac */
    }, {
      0, /* key_size_index */
      6, /* key_area */
      { 0xc6, 0x5b, 0x94, 0xb1, 0xf2, 0x91, 0xfa, 0x9f,
        0x06, 0x00, 0xf2, 0x2c, 0x3c, 0x04, 0x32, 0xc8,
        0x95, 0xad, 0x5d, 0x17, 0x7b, 0xcc, 0xcc, 0x9e,
        0xa4, 0x4e, 0x8e, 0xc3, 0x39, 0xc9, 0xad, 0xf4,
        0x38, 0x55, 0xb3, 0x26, 0x17, 0x9d, 0x6d, 0x81,
        0xaa, 0x36, 0xef, 0x59, 0x46, 0x2f, 0xd8, 0x61,
        0x27, 0xe9, 0xd8, 0x1b, 0x0f, 0x28, 0x6f, 0x93,
        0x30, 0x6b, 0xf7, 0x4d, 0x4c, 0x79, 0xe4, 0x7c,
        0x1b, 0x3d, 0x4b, 0x74, 0xed, 0xd3, 0xa1, 0x62,
        0x90, 0xe3, 0xc6, 0x3b, 0x74, 0x2e, 0x41, 0xf2,
        0x0d, 0x66, 0xce, 0xee, 0x79, 0x43, 0x16, 0xbb,
        0x63, 0xd3, 0xbd, 0x00, 0x27, 0x12, 0xa1, 0xb1,
        0x36, 0xba, 0x61, 0x85, 0xbd, 0x5c, 0x1d, 0xab,
        0x81, 0xb0, 0x7d, 0xb9, 0x0d, 0x2a, 0xf5, 0xe5 }, /* mdata */
      112, /* mdata_len */
      { 0x47, 0x12, 0x32, 0xdc, 0x47, 0xca, 0x7e, 0xe2,
        0xd8, 0xd6, 0x43, 0x6c, 0xe5, 0x75, 0x03, 0xb9 } /* mac */
    }, {
      0, /* key_size_index */
      7, /* key_area */
      { 0x22, 0xcd, 0xc3, 0x30, 0x6f, 0xcd, 0x4d, 0x31,
        0xcc, 0xd3, 0x27, 0x20, 0xcb, 0xb6, 0x1b, 0xad,
        0x28, 0xd8, 0x55, 0x67, 0x06, 0x57, 0xc4, 0x8c,
        0x7b, 0x88, 0xc3, 0x1f, 0x4f, 0xa1, 0xf9, 0x3c,
        0x01, 0xb5, 0x7d, 0xa9, 0x0b, 0xe6, 0x3e, 0xad,
        0x67, 0xd6, 0xa3, 0x25, 0x52, 0x5e, 0x6e, 0xd4,
        0x50, 0x83, 0xe6, 0xfb, 0x70, 0xa5, 0x35, 0x29,
        0xd1, 0xfa, 0x0f, 0x55, 0x65, 0x3b, 0x94, 0x2a,
        0xf5, 0x9d, 0x78, 0xa2, 0x66, 0x03, 0x61, 0xd6,
        0x3a, 0x72, 0x90, 0x15, 0x5a, 0xc5, 0xc4, 0x33,
        0x12, 0xa2, 0x5b, 0x23, 0x5d, 0xac, 0xbb, 0xc8,
        0x63, 0xfa, 0xf0, 0x09, 0x40, 0xc9, 0x96, 0x24,
        0x07, 0x6d, 0xfa, 0x44, 0x06, 0x8e, 0x7c, 0x55,
        0x4c, 0x90, 0x38, 0x17, 0x69, 0x53, 0xe5, 0x71,
        0x75, 0x1d, 0xfc, 0x09, 0x54, 0xd4, 0x1d, 0x11,
        0x37, 0x71, 0xb0, 0x64, 0x66, 0xb1, 0xc8, 0xd1,
        0x3e, 0x0d, 0x4c, 0xb6, 0x75, 0xed, 0x58, 0xd1,
        0xa6, 0x19, 0xe1, 0x54, 0x09, 0x70, 0x98, 0x37,
        0x81, 0xdc, 0x11, 0xd2, 0xdd, 0x85, 0x25, 0xab,
        0x57, 0x45, 0x95, 0x8d, 0x61, 0x5d, 0xef, 0xda }, /* mdata */
      160, /* mdata_len */
      { 0xe9, 0xae, 0x7e, 0x63, 0x54, 0x23, 0xc3, 0x66,
        0xd5, 0x23, 0xf9, 0x7c, 0x0e, 0xdb, 0x18, 0xb0 } /* mac */
    }, {
      1, /* key_size_index */
      0, /* key_area */
      { 0xc5, 0x1f, 0xc2, 0x76, 0x77, 0x4d, 0xad, 0x94,
        0xbc, 0xdc, 0x1d, 0x28, 0x91, 0xec, 0x86, 0x68 }, /* mdata */
      16, /* mdata_len */
      { 0x40, 0xd1, 0x7f, 0x9a, 0x6f, 0x5b, 0xc6, 0xaf,
        0x34, 0x1e, 0x6a, 0xc5, 0xe4, 0x9e, 0x71, 0xad } /* mac */
    }, {
      1, /* key_size_index */
      2, /* key_area */
      { 0xbe, 0x8a, 0xbf, 0x00, 0x90, 0x13, 0x63, 0x98,
        0x7a, 0x82, 0xcc, 0x77, 0xd0, 0xec, 0x91, 0x69,
        0x7b, 0xa3, 0x85, 0x7f, 0x9e, 0x4f, 0x84, 0xbd,
        0x79, 0x40, 0x6c, 0x13, 0x8d, 0x02, 0x69, 0x8f,
        0x00, 0x32, 0x76, 0xd0, 0x44, 0x91, 0x20, 0xbe,
        0xf4, 0x57, 0x8d, 0x78, 0xfe, 0xca, 0xbe, 0x8e,
        0x07, 0x0e, 0x11, 0x71, 0x0b, 0x3f, 0x0a, 0x27,
        0x44, 0xbd, 0x52, 0x43, 0x4e, 0xc7, 0x00, 0x15,
        0x88, 0x4c, 0x18, 0x1e, 0xbd, 0xfd, 0x51, 0xc6,
        0x04, 0xa7, 0x1c, 0x52, 0xe4, 0xc0, 0xe1, 0x10,
        0xbc, 0x40, 0x8c, 0xd4, 0x62, 0xb2, 0x48, 0xa8,
        0x0b, 0x8a, 0x8a, 0xc0, 0x6b, 0xb9, 0x52, 0xac,
        0x1d, 0x7f, 0xae, 0xd1, 0x44, 0x80, 0x7f, 0x1a,
        0x73, 0x1b, 0x7f, 0xeb, 0xca, 0xf7, 0x83, 0x57,
        0x62, 0xde, 0xfe, 0x92, 0xec, 0xcf, 0xc7, 0xa9,
        0x94, 0x4e, 0x1c, 0x70, 0x2c, 0xff, 0xe6, 0xbc,
        0x86, 0x73, 0x3e, 0xd3, 0x21, 0x42, 0x31, 0x21,
        0x08, 0x5a, 0xc0, 0x2d, 0xf8, 0x96, 0x2b, 0xcb,
        0xc1, 0x93, 0x70, 0x92, 0xee, 0xbf, 0x0e, 0x90,
        0xa8, 0xb2, 0x0e, 0x3d, 0xd8, 0xc2, 0x44, 0xae }, /* mdata */
      160, /* mdata_len */
      { 0xea, 0x57, 0x08, 0xb7, 0x8b, 0xf0, 0x51, 0xd6,
        0x94, 0xeb, 0x37, 0x01, 0xca, 0x6b, 0xd5, 0x7b } /* mac */
    }, {
      1, /* key_size_index */
      4, /* key_area */
      { 0x39, 0xa9, 0xb4, 0x2d, 0xe1, 0x9e, 0x51, 0x2a,
        0xb7, 0xf3, 0x04, 0x35, 0x64, 0xc3, 0x51, 0x5a }, /* mdata */
      16, /* mdata_len */
      { 0xa8, 0xd9, 0x3d, 0x9e, 0x1c, 0x14, 0xe4, 0x1e,
        0xa3, 0xf0, 0xaa, 0x50, 0xa4, 0xa3, 0x26, 0x09 } /* mac */
    }, {
      1, /* key_size_index */
      6, /* key_area */
      { 0x69, 0x28, 0x29, 0x9c, 0x52, 0xb4, 0xf0, 0x47,
        0x92, 0x6f, 0x8a, 0x54, 0x15, 0x29, 0xda, 0x2d,
        0x6b, 0xba, 0xa3, 0x99, 0x14, 0x3c, 0xed, 0x8e,
        0xfb, 0x77, 0xab, 0x47, 0x40, 0x9d, 0x9a, 0x95,
        0x3a, 0x38, 0x6c, 0x7a, 0xbd, 0x60, 0x26, 0xf4,
        0x98, 0x31, 0xc7, 0x17, 0x62, 0x7c, 0x2a, 0x5e,
        0x77, 0xbd, 0x2d, 0x43, 0x3d, 0x4d, 0x13, 0x0d,
        0xac, 0xd9, 0x27, 0xea, 0x0d, 0x13, 0xa2, 0x3d,
        0x01, 0xa7, 0xcf, 0x39, 0xc6, 0x71, 0x6d, 0xaf,
        0xb6, 0xed, 0x55, 0x24, 0x10, 0xef, 0x5d, 0x27,
        0xfb, 0x94, 0x7b, 0xe2, 0xc8, 0x78, 0x2e, 0xee,
        0x78, 0x29, 0x19, 0x6c, 0x7e, 0xdc, 0xf1, 0x51,
        0xc6, 0x5f, 0x9a, 0x01, 0xf5, 0x4f, 0x8d, 0x20,
        0xf3, 0x8b, 0x7d, 0xa4, 0xa7, 0xe8, 0x3a, 0x2f,
        0x01, 0x27, 0xd5, 0x9d, 0x3e, 0x24, 0x05, 0xd8,
        0x67, 0x4f, 0xc9, 0xf4, 0x1b, 0x60, 0x4f, 0x78,
        0x8f, 0x47, 0x15, 0xf9, 0xd3, 0x62, 0x4e, 0xee,
        0x57, 0xf3, 0x87, 0xbf, 0xad, 0xd1, 0x8a, 0x1f,
        0x90, 0x5e, 0x83, 0x9c, 0x26, 0xb8, 0x61, 0x74,
        0x82, 0x34, 0x7f, 0xab, 0x6d, 0x08, 0x84, 0x5a }, /* mdata */
      160, /* mdata_len */
      { 0x69, 0x27, 0xe7, 0xfb, 0x4c, 0xb9, 0x9d, 0x9c,
        0x54, 0xe2, 0x7f, 0x1c, 0x76, 0x20, 0xa0, 0x07 } /* mac */
    }, {
      2, /* key_size_index */
      0, /* key_area */
      { 0x62, 0x82, 0xb8, 0xc0, 0x5c, 0x5c, 0x15, 0x30,
        0xb9, 0x7d, 0x48, 0x16, 0xca, 0x43, 0x47, 0x62 }, /* mdata */
      16, /* mdata_len */
      { 0xa1, 0x07, 0x61, 0x82, 0xc1, 0xf4, 0x33, 0xc3,
        0xda, 0xfb, 0xe1, 0x1d, 0x3e, 0x71, 0xdd, 0x8a } /* mac */
    }, {
      2, /* key_size_index */
      2, /* key_area */
      { 0x0c, 0x63, 0xd4, 0x13, 0xd3, 0x86, 0x45, 0x70,
        0xe7, 0x0b, 0xb6, 0x61, 0x8b, 0xf8, 0xa4, 0xb9,
        0x58, 0x55, 0x86, 0x68, 0x8c, 0x32, 0xbb, 0xa0,
        0xa5, 0xec, 0xc1, 0x36, 0x2f, 0xad, 0xa7, 0x4a,
        0xda, 0x32, 0xc5, 0x2a, 0xcf, 0xd1, 0xaa, 0x74,
        0x44, 0xba, 0x56, 0x7b, 0x4e, 0x7d, 0xaa, 0xec,
        0xf7, 0xcc, 0x1c, 0xb2, 0x91, 0x82, 0xaf, 0x16,
        0x4a, 0xe5, 0x23, 0x2b, 0x00, 0x28, 0x68, 0x69,
        0x56, 0x35, 0x59, 0x98, 0x07, 0xa9, 0xa7, 0xf0,
        0x7a, 0x1f, 0x13, 0x7e, 0x97, 0xb1, 0xe1, 0xc9,
        0xda, 0xbc, 0x89, 0xb6, 0xa5, 0xe4, 0xaf, 0xa9,
        0xdb, 0x58, 0x55, 0xed, 0xaa, 0x57, 0x50, 0x56,
        0xa8, 0xf4, 0xf8, 0x24, 0x22, 0x16, 0x24, 0x2b,
        0xb0, 0xc2, 0x56, 0x31, 0x0d, 0x9d, 0x32, 0x98,
        0x26, 0xac, 0x35, 0x3d, 0x71, 0x5f, 0xa3, 0x9f,
        0x80, 0xce, 0xc1, 0x44, 0xd6, 0x42, 0x45, 0x58,
        0xf9, 0xf7, 0x0b, 0x98, 0xc9, 0x20, 0x09, 0x6e,
        0x0f, 0x2c, 0x85, 0x5d, 0x59, 0x48, 0x85, 0xa0,
        0x06, 0x25, 0x88, 0x0e, 0x9d, 0xfb, 0x73, 0x41,
        0x63, 0xce, 0xce, 0xf7, 0x2c, 0xf0, 0x30, 0xb8 }, /* mdata */
      160, /* mdata_len */
      { 0xf2, 0xb7, 0x45, 0x13, 0xd6, 0xcf, 0x2d, 0x80,
        0xe6, 0x72, 0xb8, 0x37, 0x45, 0xfa, 0xdc, 0x0f } /* mac */
    }, {
      2, /* key_size_index */
      4, /* key_area */
      { 0xd5, 0x1d, 0x19, 0xde, 0xd5, 0xca, 0x4a, 0xe1,
        0x4b, 0x2b, 0x20, 0xb0, 0x27, 0xff, 0xb0, 0x20 }, /* mdata */
      16, /* mdata_len */
      { 0xb0, 0x52, 0xa1, 0xdf, 0xe6, 0xa4, 0x8e, 0x63,
        0x43, 0x23, 0xdc, 0xaa, 0x85, 0xfa, 0xda, 0x3c } /* mac */
    }, {
      2, /* key_size_index */
      6, /* key_area */
      { 0x5b, 0x97, 0xa9, 0xd4, 0x23, 0xf4, 0xb9, 0x74,
        0x13, 0xf3, 0x88, 0xd9, 0xa3, 0x41, 0xe7, 0x27,
        0xbb, 0x33, 0x9f, 0x8e, 0x18, 0xa3, 0xfa, 0xc2,
        0xf2, 0xfb, 0x85, 0xab, 0xdc, 0x8f, 0x13, 0x5d,
        0xeb, 0x30, 0x05, 0x4a, 0x1a, 0xfd, 0xc9, 0xb6,
        0xed, 0x7d, 0xa1, 0x6c, 0x55, 0xeb, 0xa6, 0xb0,
        0xd4, 0xd1, 0x0c, 0x74, 0xe1, 0xd9, 0xa7, 0xcf,
        0x8e, 0xdf, 0xae, 0xaa, 0x68, 0x4a, 0xc0, 0xbd,
        0x9f, 0x9d, 0x24, 0xba, 0x67, 0x49, 0x55, 0xc7,
        0x9d, 0xc6, 0xbe, 0x32, 0xae, 0xe1, 0xc2, 0x60,
        0xb5, 0x58, 0xff, 0x07, 0xe3, 0xa4, 0xd4, 0x9d,
        0x24, 0x16, 0x20, 0x11, 0xff, 0x25, 0x4d, 0xb8,
        0xbe, 0x07, 0x8e, 0x8a, 0xd0, 0x7e, 0x64, 0x8e,
        0x6b, 0xf5, 0x67, 0x93, 0x76, 0xcb, 0x43, 0x21,
        0xa5, 0xef, 0x01, 0xaf, 0xe6, 0xad, 0x88, 0x16,
        0xfc, 0xc7, 0x63, 0x46, 0x69, 0xc8, 0xc4, 0x38,
        0x92, 0x95, 0xc9, 0x24, 0x1e, 0x45, 0xff, 0xf3,
        0x9f, 0x32, 0x25, 0xf7, 0x74, 0x50, 0x32, 0xda,
        0xee, 0xbe, 0x99, 0xd4, 0xb1, 0x9b, 0xcb, 0x21,
        0x5d, 0x1b, 0xfd, 0xb3, 0x6e, 0xda, 0x2c, 0x24 }, /* mdata */
      160, /* mdata_len */
      { 0xc9, 0xa0, 0x14, 0x60, 0xaa, 0x2f, 0x85, 0x25,
        0x88, 0x75, 0xa1, 0x76, 0xcc, 0x85, 0x46, 0xf9 } /* mac */
    }
  };
  static uint8_t mdata[MDATA_MAX_LEN];
  static uint8_t mac[CBC_MAC_MAC_LEN];
  static int i;
  static uint8_t key_size_index = -1, ret;
  static rtimer_clock_t time, time2, total_time;

  PROCESS_BEGIN();

  puts("-----------------------------------------\n"
       "Initializing cryptoprocessor...");
  crypto_init();

  for(i = 0; i < sizeof(vectors) / sizeof(vectors[0]); i++) {
    if(key_size_index != vectors[i].key_size_index) {
      key_size_index = vectors[i].key_size_index;
      printf("-----------------------------------------\n"
             "Filling %d-bit key store...\n", 128 + (key_size_index << 6));
      time = RTIMER_NOW();
      ret = aes_load_keys(keys[key_size_index].keys,
              keys[key_size_index].key_size, keys[key_size_index].count, 0);
      time = RTIMER_NOW() - time;
      printf("aes_load_keys(): %s, %lu us\n", str_res[ret],
             (uint32_t)((uint64_t)time * 1000000 / RTIMER_SECOND));
      PROCESS_PAUSE();
      if(ret != CRYPTO_SUCCESS) {
        break;
      }
    }

    printf("-----------------------------------------\n"
           "Test vector #%d:\n"
           "key_area=%d mdata_len=%d\n",
           i, vectors[i].key_area, vectors[i].mdata_len);

    /* mdata has to be in SRAM. */
    rom_util_memcpy(mdata, vectors[i].mdata, vectors[i].mdata_len);

    time = RTIMER_NOW();
    ret = cbc_mac_auth_start(vectors[i].key_area, mdata, vectors[i].mdata_len,
                             &cbc_mac_test_process);
    time2 = RTIMER_NOW();
    time = time2 - time;
    total_time = time;
    if(ret == CRYPTO_SUCCESS) {
      PROCESS_WAIT_EVENT_UNTIL(cbc_mac_auth_check_status());
      time2 = RTIMER_NOW() - time2;
      total_time += time2;
    }
    printf("cbc_mac_auth_start(): %s, %lu us\n", str_res[ret],
           (uint32_t)((uint64_t)time * 1000000 / RTIMER_SECOND));
    if(ret != CRYPTO_SUCCESS) {
      PROCESS_PAUSE();
      continue;
    }
    printf("cbc_mac_auth_check_status() wait: %lu us\n",
           (uint32_t)((uint64_t)time2 * 1000000 / RTIMER_SECOND));

    time = RTIMER_NOW();
    ret = cbc_mac_auth_get_result(vectors[i].mac, mac);
    time = RTIMER_NOW() - time;
    total_time += time;
    printf("cbc_mac_auth_get_result(): %s, %lu us\n", str_res[ret],
           (uint32_t)((uint64_t)time * 1000000 / RTIMER_SECOND));
    PROCESS_PAUSE();
    if(ret != CRYPTO_SUCCESS) {
      continue;
    }

    if(rom_util_memcmp(mac, vectors[i].mac, CBC_MAC_MAC_LEN)) {
      puts("MAC does not match expected one");
    } else {
      puts("MAC OK");
    }

    printf("Total duration: %lu us\n",
           (uint32_t)((uint64_t)total_time * 1000000 / RTIMER_SECOND));
  }

  puts("-----------------------------------------\n"
       "Disabling cryptoprocessor...");
  crypto_disable();

  puts("Done!");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */