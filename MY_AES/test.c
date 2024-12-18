#include "aes.h"

// uint8_t plaintext[16] = {0x32, 0x043, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
// uint8_t key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
uint8_t plaintext[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
uint8_t key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

void print_state_hex(uint8_t *state)
{
    for (int i = 0; i < 16; i++)
    {
        printf("0x%2x ", state[i]);
    }
    printf("\n");
}
int main()
{
    printf("plaintext:");
    print_state_hex(plaintext);
    printf("key:");
    print_state_hex(key);
    cipher(plaintext, key);
    printf("ciphertext:");
    print_state_hex(plaintext);
    invcipher(plaintext, key);
    print_state_hex(plaintext);
    return 0;
}