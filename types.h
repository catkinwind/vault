#ifndef _VAULT_TYPES_H_
#define _VAULT_TYPES_H_

#ifndef _VAULT_H_
#error only "vault.h" can be included
#endif

struct list_head {
  struct list_head *next, *prev;
};

struct hlist_head {
  struct hlist_node *first;
};

struct hlist_node {
  struct hlist_node *next, **pprev;
};

struct vault_entry {
  char *key;
  char *value;
  struct hlist_node hash_entry;
};

#endif /* _VAULT_TYPES_H_ */
