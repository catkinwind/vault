#ifndef _VAULT_ENTRY_H_
#define _VAULT_ENTRY_H_

#include "vault.h"

void free_vault_entry(struct hlist_node *node, void * arg);
int create_vault_entry(char* key, char* value, struct vault_entry **e);

#endif
