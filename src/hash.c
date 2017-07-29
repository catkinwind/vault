#include <stdlib.h>
#ifdef __APPLE__
#include <sys/malloc.h>
#elif __LINUX__
#include <malloc.h>
#endif

#include "hash.h"

static void vault_hash_init(vault_hashtable *hash)
{
  u32 i;

  for (i = 0; i < hash->size; i++) {
    INIT_HLIST_HEAD(&hash->table[i]);
    pthread_mutex_init(&hash->list_locks[i], NULL);
  }
}

void vault_hash_destroy(vault_hashtable *hash)
{
  free(hash->list_locks);
  free(hash->table);
  free(hash);
}

vault_hashtable *vault_hash_new(u32 size)
{
  vault_hashtable *hash;

  hash = malloc(sizeof(*hash));
  hash->table = malloc(size * sizeof(*hash->table));
  hash->list_locks = malloc(size * sizeof(*hash->list_locks));

  if (!hash->list_locks)
    goto free_table;

  hash->size = size;
  vault_hash_init(hash);
  return hash;

free_table:
  free(hash->table);
free_hash:
  free(hash);
  return NULL;
}

