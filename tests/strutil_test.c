#include <stdio.h>
#include <string.h>
#include "../strutil.h"

int main(int argc, char *argv[])
{
  char str[] = "   ssst \t\0";
  char str1[] = "\t dd \t\0";
  char str2[] = "\0";
  char *str3 = NULL;
  // str[4] = 0;

  printf("str: %s, %d\n", trim(str), strlen(str));
  printf("str1: %s, %d\n", trim(str1), strlen(str1));
  printf("str2: %s, %d\n", trim(str2), strlen(str2));
  if (trim(str3))
    printf("error for str3");
}
