#ifndef _VAULT_ENC_H
# define __VAULT_ENC_H 1
#include <stdio.h>
#include <openssl/evp.h>

#define IP_SIZE 1024
#define OP_SIZE 1024 + EVP_MAX_BLOCK_LENGTH
#define KEY_SIZE 32
#define IV_SIZE 16
#define INIT_BUF_SIZE 1024

int generate_key(char *keyf);

int encrypt(char *in, FILE *out, int in_len, char *keyf);

int decrypt(FILE *in, char **out, size_t *out_len, char *keyf);

int fencrypt(char *in, char *out, char *keyf);

int fdecrypt(char *in, char *out, char *keyf);

#endif
