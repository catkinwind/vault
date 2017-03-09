#ifndef _STR_EXT_H_
#define _STR_EXT_H_
#include <ctype.h>

static inline char * trim(char *str)
{
  int i;
  if (!str)
    return NULL;
  char *head = str;
  while (isspace(*head))
    head++;
  char *tail = head;
  while (*tail != '\0')
    tail++;
  while (--tail >= head && isspace(*tail))
    *tail = '\0';
  char *cur = str;
  while (head <= tail) {
    *cur++ = *head++;
  }
  *cur = 0;
  return str;
}

/*
 * Values could be multiple lines, so we will append values
 * continuously.
 */
static char * append_values(char *value, const char *line)
{
  char *tmp;
  size_t len = strlen(line) + 1;
  if (value) {
    len += strlen(value) + 1;
    tmp = (char *)realloc(value, len);
    strcat(tmp, "\n");
    strcat(tmp, line);
    return tmp;
  }
  tmp = (char *)malloc(len);
  tmp[0] = 0;
  strcat(tmp, line);
  return tmp;
}

#endif  /* _STR_EXT_H_ */
