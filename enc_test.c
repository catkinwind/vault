#include "enc.h"

int main(int argc, char *argv[])
{
  FILE *f1, *f2, *f3;
  unsigned char *buf, *outbuf, *ptr;
  size_t len, in_len = 0;

  if (!(f1 = fopen("reminder.txt", "rb"))) {
    printf("error to open reminder.txt\n");
    return 1;
  }

  buf = malloc(IP_SIZE * 16);
  ptr = buf;
  while((len = fread(ptr, 1, IP_SIZE, f1)) > 0) {
    in_len += len;
    ptr += len;
  }
  fclose(f1);

  if (!(f2 = fopen("out1", "wb"))) {
    printf("error to write out1\n");
    return 1;
  }
  if (!(f3 = fopen("out2", "wb"))) {
    printf("error to open out2\n");
    return 1;
  }

  if (encrypt(buf, f2, in_len)) {
    fclose(f2);
  }

  if (!(f2 = fopen("out1", "rb"))) {
    printf("error to read out1\n");
    return 1;
  }

  if (decrypt(f2, &outbuf, &len)) {
    fclose(f2);
    fwrite(outbuf, 1, len, f3);
    fclose(f3);
  }
}


