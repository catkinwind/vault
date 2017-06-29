#ifndef _VAULT_HASH_H_
#define _VAULT_HASH_H_

#include <pthread.h>
#include <linux/stddef.h>
#include "vault.h"
#include "list.h"
#include "pthread.h"

typedef unsigned int u32;

typedef bool (*vault_hashdata_compare_cb)(const struct hlist_node *,
             const void *);
typedef u32 (*vault_hashdata_choose_cb)(const void *, u32);
typedef void (*vault_hashdata_free_cb)(struct hlist_node *, void *);

typedef struct _vault_hashtable {
  struct hlist_head *table;
  pthread_mutex_t *list_locks;
  u32 size;
} vault_hashtable;

vault_hashtable *vault_hash_new(u32 size);

void vault_hash_destroy(vault_hashtable *hash);

static inline void vault_hash_delete(vault_hashtable *hash,
              vault_hashdata_free_cb free_cb,
              void *arg)
{
  struct hlist_head *head;
  struct hlist_node *node, *node_tmp;
  pthread_mutex_t *list_lock;
  u32 i;

  for (i = 0; i < hash->size; i++) {
    head = &hash->table[i];
    list_lock = &hash->list_locks[i];

    pthread_mutex_lock(list_lock);
    hlist_for_each_safe(node, node_tmp, head) {
      hlist_del(node);

      if (free_cb)
        free_cb(node, arg);
    }
    pthread_mutex_unlock(list_lock);
  }

  vault_hash_destroy(hash);
}

static inline int vault_hash_add(vault_hashtable *hash,
  vault_hashdata_compare_cb compare,
  vault_hashdata_choose_cb choose,
  const void *data,
  struct hlist_node *data_node)
{
  u32 index;
  struct hlist_head *head;
  struct hlist_node *node;
  pthread_mutex_t *list_lock;

  if (!hash)
    return -1;

  index = choose(data, hash->size);
  head = &hash->table[index];
  list_lock = &hash->list_locks[index];
  pthread_mutex_lock(list_lock);
  hlist_for_each(node, head) {
    if (!compare(node, data))
      continue;

    pthread_mutex_unlock(list_lock);
    return 1;
  }

  /* no duplicate found in list, add new element */
  hlist_add_head(data_node, head);
  pthread_mutex_unlock(list_lock);
  return 0;
}

static inline void *vault_hash_remove(vault_hashtable *hash,
              vault_hashdata_compare_cb compare,
              vault_hashdata_choose_cb choose,
              void *data)
{
  u32 index;
  struct hlist_node *node;
  struct hlist_head *head;
  void *data_save = NULL;

  index = choose(data, hash->size);
  head = &hash->table[index];

  pthread_mutex_lock(&hash->list_locks[index]);
  hlist_for_each(node, head) {
    if (!compare(node, data))
      continue;

    data_save = node;
    hlist_del(node);
    break;
  }
  pthread_mutex_unlock(&hash->list_locks[index]);

  return data_save;
}

#endif /* _VAULT_HASH_H_ */
