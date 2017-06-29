#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "entry.h"
#include "strutil.h"

/*
 * Free vault entry, the entry don't need release.
 */
void free_vault_entry(struct hlist_node *node, void * arg)
{
  struct vault_entry *tmp;
  tmp = container_of(node, struct vault_entry, hash_entry);
  free(tmp->key);
  free(tmp->value);
  free(tmp);
}

/*
 * Create vault entry, remember to release it.
 */
int create_vault_entry(char* key, char* value, struct vault_entry **e)
{
  *e = (struct vault_entry *)malloc(sizeof(struct vault_entry));
  struct vault_entry *tmp = *e;
  tmp->key = key;
  tmp->value = value;
  return 0;
}

