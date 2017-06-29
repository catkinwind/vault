#include <openssl/evp.h>

int do_crypt(FILE *in, FILE *out, int do_encrypt)
{
  unsigned char inbuf[1024], outbuf[1024 + EVP_MAX_BLOCK_LENGTH];
  int inlen, outlen;
  EVP_CIPHER_CTX ctx;

  unsigned char key[] = "0123456789abcdeF0123456789abcdeF";
  unsigned char iv[] = "1234567887654321";

  /* Don't set key or IV right away; we want to check lengths */
  EVP_CIPHER_CTX_init(&ctx);
  EVP_CipherInit_ex(&ctx, EVP_aes_256_cbc(), NULL, NULL, NULL,
      do_encrypt);
  OPENSSL_assert(EVP_CIPHER_CTX_key_length(&ctx) == 32);
  OPENSSL_assert(EVP_CIPHER_CTX_iv_length(&ctx) == 16);

  /* Now we can set key and IV */
  EVP_CipherInit_ex(&ctx, NULL, NULL, key, iv, do_encrypt);

  for (;;) {
    inlen = fread(inbuf, 1, 1024, in);
    if (inlen <= 0) break;
    if (!EVP_CipherUpdate(&ctx, outbuf, &outlen, inbuf, inlen)) {
      /* Error */
      EVP_CIPHER_CTX_cleanup(&ctx);
      return 0;
    }
    fwrite(outbuf, 1, outlen, out);
  }
  if (!EVP_CipherFinal_ex(&ctx, outbuf, &outlen)) {
    /* Error */
    EVP_CIPHER_CTX_cleanup(&ctx);
    return 0;
  }
  fwrite(outbuf, 1, outlen, out);

  EVP_CIPHER_CTX_cleanup(&ctx);
  return 1;
}

int main(int argc, char *argv[])
{
  FILE *f1, *f2, *f3;
  if (!(f1 = fopen("reminder.txt", "rb"))) {
    printf("error to open reminder.txt\n");
    return 1;
  }
  if (!(f2 = fopen("out1", "wb"))) {
    printf("error to write out1\n");
    return 1;
  }
  if (!(f3 = fopen("out2", "wb"))) {
    printf("error to open out2\n");
    return 1;
  }

  if (do_crypt(f1, f2, 1)) {
    fclose(f1);
    fclose(f2);
  }

  if (!(f2 = fopen("out1", "rb"))) {
    printf("error to read out1\n");
    return 1;
  }

  if (do_crypt(f2, f3, 0)) {
    fclose(f2);
    fclose(f3);
  }
}
