#include "../vault.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static bool vault_compare(const struct hlist_node *node,
            const void *data2)
{
  const void *data1 = container_of(node, struct vault_entry,
            hash_entry);
  const struct vault_entry *ve1 = data1;
  const struct vault_entry *ve2 = data2;
  return strcmp(ve1->key, ve2->key);
}

static unsigned int vault_choose(const void *data, u32 index)
{
  struct vault_entry *e;

  e = (struct vault_entry *)data;
  return jhash(e->key, strlen(e->key), 0) % HASH_LEN;
}

static struct vault_entry *
vault_hash_find(vault_hashtable * hash, char *key)
{
  struct hlist_head *head;
  struct vault_entry to_search, *e, *tmp = NULL;
  u32 index;

  if (!hash)
    return NULL;

  

}

int main(int argc, char *argv[])
{
  struct vault_entry e1;
  e1.key = "test";
  e1.value = "test_value";

  vault_hashtable *hash = vault_hash_new(HASH_LEN);
  vault_hash_add(hash, vault_compare, vault_choose, &e1, &e1.hash_entry);
  void *node = vault_hash_find(hash, vault_compare, 
}
