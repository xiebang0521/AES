#include "aes.h"

#define NK 4  // 秘钥4个字节16bit
#define NR 10 // 加密的轮数

static const uint8_t sbox[256] =
    {
        // 0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
        0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
        0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
        0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
        0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
        0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
        0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
        0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
        0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
        0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
        0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
        0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
        0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
        0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
        0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
        0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
        0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};

static const uint8_t rcon[] = {0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};
static const uint8_t constant[] =
    {
        0x02, 0x03, 0x01, 0x01,
        0x01, 0x02, 0x03, 0x01,
        0x01, 0x01, 0x02, 0x03,
        0x03, 0x01, 0x01, 0x02};

/**
 * @brief 秘钥扩展
 * @param key 秘钥
 * @param roundkey 秘钥扩展后的秘钥
 * @return null
 */
static void keyExpansion(const uint8_t *key, uint8_t *roundkey)
{
    uint8_t temp[4];
    uint8_t i, j, k;

    // 第一轮的秘钥就是key
    for (i = 0; i < NK; i++)
    {
        roundkey[(4 * i) + 0] = key[(4 * i) + 0];
        roundkey[(4 * i) + 1] = key[(4 * i) + 1];
        roundkey[(4 * i) + 2] = key[(4 * i) + 2];
        roundkey[(4 * i) + 3] = key[(4 * i) + 3];
    }

    // 后面秘钥的过程：
    // （i % 4） == 0 就是前一列的秘钥经过，左位移，字节替换，
    // 第一个数据与RCon数组中的值进行异或，数组下标由第几轮的秘钥扩展决定,在与前四列的秘钥进行异或操作得到。
    // 若 （i % 4） != 0,就是前列的秘钥与前四列的秘钥异或。
    // i 秘钥扩展的列
    for (i = NK; i < 4 * (NR + 1); i++)
    {
        j = (i - 1) * 4;

        temp[0] = RoundKey[j + 0];
        temp[1] = RoundKey[j + 1];
        temp[2] = RoundKey[j + 2];
        temp[3] = RoundKey[j + 3];

        if (i % NK == 0)
        {
            uint8_t ret = temp[0];
            for (uint8_t l = 0; l < 3; l++)
            {
                temp[l] = temp[l + 1];
            }
            temp[3] = ret;

            temp[0] = sbox[temp[0]];
            temp[1] = sbox[temp[1]];
            temp[2] = sbox[temp[2]];
            temp[3] = sbox[temp[3]];

            temp[0] ^= rcon[i / NK];
        }

        j = (i - NK) * 4;
        k = i * 4;

        roundkey[k + 0] = roundkey[j + 0] ^ temp[0];
        roundkey[k + 1] = roundkey[j + 1] ^ temp[1];
        roundkey[k + 2] = roundkey[j + 2] ^ temp[2];
        roundkey[k + 3] = roundkey[j + 3] ^ temp[3];
    }
}

static void addRoundKey(uint8_t *state, uint8_t *roundkey, uint8_t round)
{
    uint8_t i, j;
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            state[4 * i + j] ^= roundkey[(round * 4 * 4) + (i * 4) + j];
        }
    }
}

/**
 * @brief 字符代替
 * @param state 进行字节替换的数据数组
 * @return null
 */
void subByte(uint8_t *state)
{
    for (uint8_t i = 0; i < 16; i++)
    {
        *(state + i)= sbox[*(state + i)];
    }
}

/**
 * @brief 行位移
 * @param state 旋转的数据
 * @return null
 */
//void shiftRow(uint8_t *state)
//{
//    uint8_t temp[4][4];
//
//    for (uint8_t i = 0; i < 4; i++)
//    {
//        for (uint8_t j = 0; j < 4; j++)
//        {
//            temp[i][j] = state [(4 * i) + (i + j) % 4];
//        }
//    }
//
//    for (uint8_t i = 0; i < 4; i++)
//    {
//        for (uint8_t j = 0; j < 4; j++)
//        {
//           state[(4 * i) + j] = temp[i][j];
//        }
//    }
//}
void shiftRow(uint8_t* state)
{
    uint8_t temp[4][4];

    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
        {
            temp[j][i] = state[(4 * ((j + i) % 4)) + i];
        }
    }

    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
        {
            state[(4 * i) + j] = temp[i][j];
        }
    }
}

uint8_t gfMultiply(uint8_t x, uint8_t y)
{
    if (0x01 == y)
    {
        return x;
    }
    else if (0x02 == y)
    {
        return ((x << 1) ^ (x >> 7 & 0x01) * 0x1b);
    }
    else if (0x03 == y)
    {
        return (x ^ ((x << 1) ^ (x >> 7 & 0x01) * 0x1b));
    }
}

/**
 * @brief 列混合
 * @param data 待处理数据
 * @return null
 */
 void mixColumns(uint8_t *state, uint8_t *constant)
 {
     uint8_t temp[4][4];

     for (uint8_t i = 0; i < 4; i++)
     {
         for (uint8_t j = 0; j < 4; j++)
         {
             temp[i][j] = gfMultiply(*(state + 4 * 0 + j), *(constant + 4 * i + 0)) ^ gfMultiply(*(state + 4 * 1 + j), *(constant + 4 * i + 1)) ^ gfMultiply(*(state + 4 * 2 + j), *(constant + 4 * i + 2)) ^ gfMultiply(*(state + 4 * 3 + j), *(constant + 4 * i + 3));
         }
     }

     for (uint8_t i = 0; i < 4; i++)
     {
         for (uint8_t j = 0; j < 4; j++)
         {
             *(state + 4 * i + j) = temp[i][j];
         }
     }
 }
static uint8_t xtime(uint8_t x)
{
    return ((x << 1) ^ (((x >> 7) & 1) * 0x1b));
}

// MixColumns function mixes the columns of the state matrix

/**s'0,0 =0x02 * s0,0 + 0x03*s1,0 + 0x01 * s2,0 + 0x01 *s3.0 */
static void MixColumns(state_t *state)
{
    uint8_t i;
    uint8_t Tmp, Tm, t;
    for (i = 0; i < 4; ++i)
    {
        t = (*state)[i][0];
        Tmp = (*state)[i][0] ^ (*state)[i][1] ^ (*state)[i][2] ^ (*state)[i][3];
        Tm = (*state)[i][0] ^ (*state)[i][1];
        Tm = xtime(Tm);
        (*state)[i][0] ^= Tm ^ Tmp;
        Tm = (*state)[i][1] ^ (*state)[i][2];
        Tm = xtime(Tm);
        (*state)[i][1] ^= Tm ^ Tmp;
        Tm = (*state)[i][2] ^ (*state)[i][3];
        Tm = xtime(Tm);
        (*state)[i][2] ^= Tm ^ Tmp;
        Tm = (*state)[i][3] ^ t;
        Tm = xtime(Tm);
        (*state)[i][3] ^= Tm ^ Tmp;
    }
}
void cipher(uint8_t *state, uint8_t *key)
{
    int round = 0;
    keyExpansion(key, RoundKey);
    addRoundKey(state, RoundKey, 0);
    for (int i = 0; i < 16; i++)
    {
        printf("0x%02x ", state[i]);
    }
    printf("\n", round);
    for (uint8_t i = 1; i <= 10; i++)
    {
        subByte(state);
        printf("sub:");
        for (int i = 0; i < 16; i++)
        {
            printf("0x%02x ", state[i]);
        }
        printf("\n", round);
        shiftRow(state);
        printf("shi:");
        for (int i = 0; i < 16; i++)
        {
            printf("0x%02x ", state[i]);
        }
        printf("\n", round);

        if (i == 10)
            break;
        //mixColumns(state,constant);
        MixColumns(state);
        printf("mix:");
        for (int i = 0; i < 16; i++)
        {
            printf("0x%02x ", state[i]);
        }
        printf("\n", round);
        addRoundKey(state, RoundKey, i);
        printf("add:");
        for (int i = 0; i < 16; i++)
        {
            printf("0x%02x ", state[i]);
        }
        printf("\n", round);
        printf("\n", round);

    }


    addRoundKey(state, RoundKey, NR);
    for (int i = 0; i < 16; i++)
    {
        printf("0x%02x ", state[i]);
    }
    printf("\n", round);
    printf("\n", round);
}
