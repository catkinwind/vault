#include "enc.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

int compare(char *fname1, char *fname2)
{
  FILE *f1, *f2;
  f1 = fopen(fname1, "rb");
  f2 = fopen(fname2, "rb");

  size_t l1, l2;
  fseek(f1, 0L, SEEK_END);
  l1 = ftell(f1);
  fseek(f1, 0, SEEK_SET);

  fseek(f2, 0, SEEK_END);
  l2 = ftell(f2);
  fseek(f2, 0, SEEK_SET);
  if (l1 != l2) {
    printf(ANSI_COLOR_RED "File size not equal." ANSI_COLOR_RESET "\n");
    return 0;
  }

  char c1, c2;
  while(!feof(f1)) {
    if (fgetc(f1) != fgetc(f2)) {
      printf(ANSI_COLOR_RED "Content different." ANSI_COLOR_RESET "\n");
      return 0;
    }
  }
  return 1;
}

int main(int argc, char *argv[])
{
  FILE *f1, *f2, *f3;
  char *buf, *outbuf, *ptr;
  size_t len, in_len = 0;
  char *keyf = "key2";

  generate_key(keyf);

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

  if (encrypt(buf, f2, in_len, keyf)) {
    fclose(f2);
  }

  if (!(f2 = fopen("out1", "rb"))) {
    printf("error to read out1\n");
    return 1;
  }

  if (decrypt(f2, &outbuf, &len, keyf)) {
    fclose(f2);
    fwrite(outbuf, 1, len, f3);
    fclose(f3);
  }

  if (compare("out2", "reminder.txt")) {
    printf(ANSI_COLOR_GREEN "Success." ANSI_COLOR_RESET "\n");
  }

  remove(keyf);
  remove("out1");
  remove("out2");
}


