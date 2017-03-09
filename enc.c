#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <string.h>
#include "enc.h"

// unsigned char key[KEY_SIZE], iv[IV_SIZE];

# ifdef __cplusplus
extern "C" {
# endif

#define byte unsigned char

void print_key();

int generate_key(char *keyf)
{
  int keyfd;
  byte key[KEY_SIZE], iv[IV_SIZE];
  int fd;
  if((fd = open("/dev/random", O_RDONLY)) == -1) {
    perror("open error");
    return -1;
  }

  if(read(fd, key, KEY_SIZE) == -1) {
    perror("generate key error");
    return -1;
  }

  if(read(fd, iv, IV_SIZE) == -1) {
    perror("generate iv error");
    return -1;
  }

#ifdef DEBUG_KEY
  print_key(key, iv);
#endif

  // control the permission of file to owner.
  if ((keyfd = open(keyf, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) == -1) {
    perror("Could not create key file.\n");
    return -1;
  }

  if(write(keyfd, key, KEY_SIZE) == -1) {
    perror("write key error");
    return -1;
  }
  if(write(keyfd, iv, IV_SIZE) == -1) {
    perror("write iv error");
    return -1;
  }
  close(keyfd);

  return 0;
}

/*
 * Print the key and vector, be ware, it's dangerous.
 */
void print_key(byte *key, byte *iv)
{
  int i;
  printf("128 bit key:\n");
  for(i = 0; i < KEY_SIZE; ++i)
    printf("%x \t", key[i]);
  printf("\n--------\n");

  printf("initialization vector:\n");
  for(i = 0; i < IV_SIZE; ++i)
    printf("%x \t", iv[i]);

  printf("\n--------\n");
}

int read_key(char *keyf, char *key, char *iv)
{
  FILE *keyfd;
  keyfd = fopen(keyf, "r");
  printf(keyf);
  if (!keyfd) {
    printf("%s\n", keyfd );
    perror("failed to open key file2.\n");
    return -1;
  }
  if(fread(key, 1, KEY_SIZE, keyfd) == -1) {
    perror("read key error");
    return -1;
  }
  if(fread(iv, 1, IV_SIZE, keyfd) == -1) {
    perror("read iv error");
    return -1;
  }
  fclose(keyfd);

  return 1;
}

/*
 * Encrypt a block of memory data to file.
 * Returns:
 * 1  - succeed
 * -1 - fail to allocate buffer
 * -2 - fail to encrypt
 * -3 - fail to final encryption context
 */
int encrypt(char *in, FILE *out, int in_len, char *keyf)
{
  byte *outbuf, *ptr= in;
  byte key[KEY_SIZE], iv[IV_SIZE];

  int outlen, len;
  EVP_CIPHER_CTX ctx;

  read_key(keyf, key, iv);

  outbuf = (char*)calloc(1024 + EVP_MAX_BLOCK_LENGTH, sizeof(char));
  if (!outbuf) {
    perror("Failed to allocate spaces for encryption!\n");
    return -1;
  }

  /* Don't set key or IV right away; we want to check lengths */
  EVP_CIPHER_CTX_init(&ctx);
  EVP_CipherInit_ex(&ctx, EVP_aes_256_cbc(), NULL, NULL, NULL, 1);
  OPENSSL_assert(EVP_CIPHER_CTX_key_length(&ctx) == 32);
  OPENSSL_assert(EVP_CIPHER_CTX_iv_length(&ctx) == 16);

  EVP_CipherInit_ex(&ctx, NULL, NULL, key, iv, 1);

  while(in_len > 0) {
    len = in_len > 1024 ? 1024 : in_len;

    if (!EVP_CipherUpdate(&ctx, outbuf, &outlen, ptr, len)) {
      /* Error */
      EVP_CIPHER_CTX_cleanup(&ctx);
      return -2;
    }
    fwrite(outbuf, 1, outlen, out);
    ptr += 1024;
    in_len -= 1024;
  }
  if (!EVP_CipherFinal_ex(&ctx, outbuf, &outlen)) {
    /* Error */
    EVP_CIPHER_CTX_cleanup(&ctx);
    return -3;
  }
  fwrite(outbuf, 1, outlen, out);
  EVP_CIPHER_CTX_cleanup(&ctx);
  return 1;
}

int fencrypt(char *inf, char *outf, char *keyf)
{
  FILE *in, *out, *key;
  in = fopen(inf, "rb");
  size_t sz;

  fseek(in, 0L, SEEK_END);
  sz = ftell(in);
  fseek(in, 0L, SEEK_SET);

  byte *buf = (byte *)malloc(sz);

  fread(buf, 1, sz, in);
  out = fopen(outf, "wb");

  return encrypt(buf, out, sz, keyf);
}

/*
 * decrypt file to a buffer.
 */
int decrypt(FILE *in, char **out, size_t *out_len, char *keyf)
{
  char *inbuf, *ptr;
  int o_len, len;
  int cur = INIT_BUF_SIZE, prev = cur;
  byte key[KEY_SIZE], iv[IV_SIZE];

  read_key(keyf, key, iv);

  *out = malloc(cur);
  ptr = *out;

  EVP_CIPHER_CTX ctx;

  inbuf = (char*)calloc(1024, sizeof(char));
  if (!inbuf) {
    perror("Failed to allocate spaces for decryption!\n");
    return -1;
  }

  /* Don't set key or IV right away; we want to check lengths */
  EVP_CIPHER_CTX_init(&ctx);
  EVP_DecryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, NULL, NULL);
  OPENSSL_assert(EVP_CIPHER_CTX_key_length(&ctx) == 32);
  OPENSSL_assert(EVP_CIPHER_CTX_iv_length(&ctx) == 16);

  EVP_DecryptInit_ex(&ctx, NULL, NULL, key, iv);

  while(1) {
    len = fread(inbuf, 1, IP_SIZE, in);
    if (!len) break;

    if ((int)cur - (ptr - *out) < IP_SIZE) {
      int index = ptr - *out;
      cur += prev;
      prev = cur - prev;
      *out = realloc(*out, cur);
      ptr = *out + index;
    }

    if (!EVP_DecryptUpdate(&ctx, ptr, &o_len, inbuf, len)) {
      /* Error */
      EVP_CIPHER_CTX_cleanup(&ctx);
      return -2;
    }
    ptr += o_len;
    *out_len += o_len;
  }
  if (!EVP_DecryptFinal_ex(&ctx, ptr, &o_len)) {
    /* Error */
    EVP_CIPHER_CTX_cleanup(&ctx);
    return -3;
  }
  *out_len += o_len;
  EVP_CIPHER_CTX_cleanup(&ctx);
  return 1;
}

# ifdef __cplusplus
}
# endif
