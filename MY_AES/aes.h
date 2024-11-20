#pragma once

#ifndef __AES_H__
#define __AES_H__
#include "stdio.h"
typedef unsigned char uint8_t;
typedef uint8_t state_t[4][4];
// ECB密码本模式
#ifndef ECB
#define ECB
#endif

#define AES128

#define AES_DATA_LEN 16
#define AES_KEY_LEN 16
#define AES_ROUND_KEY_LEN 176

uint8_t RoundKey[AES_ROUND_KEY_LEN];

void cipher(uint8_t *state, uint8_t *key);
#endif // ! __AES_H__
