#ifndef _VAULT_ENC_H
# define __VAULT_ENC_H 1
#include <openssl/evp.h>

#define IP_SIZE 1024
#define OP_SIZE 1024 + EVP_MAX_BLOCK_LENGTH
#define KEY_SIZE 32
#define IV_SIZE 16
#define INIT_BUF_SIZE 1024

#endif

int generate_key(int keyfd);

int encrypt(unsigned char *in, FILE *out, int in_len);

int decrypt(FILE *in, unsigned char **out, size_t *out_len);
