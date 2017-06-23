#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main() {
  char *str = "hello\nevery one!";
  FILE *f = stdout;

  printf(str);
  size_t l = strlen(str);
  sleep(2);
  char *er = (char *)malloc(2*l+1);
  er = memset(er, '\b', l);
  memset(er+l, ' ', l);
  er[2*l] = 0;
  printf(er);
  int i;
  scanf("%d", &i);

  printf("\n%d", stdout -f);

  return 0;
}
