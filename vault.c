#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "vault.h"
#include "entry.h"
#include "strutil.h"
#include "enc.h"

static bool starts_with(char *str, char *pref);

char *VAULT_FILE_NAME = ".vault";

/*
 * Compare entity by their key. The list is chained by a
 * property of the entity (hash_entry), So we need to get
 * the entity using container_of.
 */
static bool vault_compare(const struct hlist_node *node,
            const void *data2)
{
  const void *data1 = container_of(node, struct vault_entry,
            hash_entry);
  const struct vault_entry *ve1 = data1;
  const struct vault_entry *ve2 = data2;
  return strcmp(ve1->key, ve2->key);
}

/*
 * This will calc hash by key of entry. The algorithm is
 * Jenkins Hash from Linux core 4.9 rc2.
 */
static u32 vault_choose(const void *data, u32 size)
{
  struct vault_entry *e;

  e = (struct vault_entry *)data;
  return jhash(e->key, strlen(e->key), 0) % size;
}

/*
 * Locate an entry in hashtable. Comparing by key.
 */
static struct vault_entry *
vault_hash_find(vault_hashtable *hash, char *key)
{
  struct hlist_head *head;
  struct vault_entry to_search, *e, *tmp = NULL;
  struct hlist_node *pos;
  u32 index;

  if (!hash)
    return NULL;
  to_search.key = key;
  index = vault_choose(&to_search, hash->size);
  head = hash->table + index;
  hlist_for_each(pos, head) {
    tmp = container_of(pos, struct vault_entry, hash_entry);
    if (strcmp(key, tmp->key) == 0)
      return tmp;
  }
  return NULL;
}

/*
 * Test if a string is a title.
 * @param line: this should be pointer instead of array.
 */
static bool is_title(char *line)
{
  return starts_with(line, "* ");
}

/*
 * Load entries from file. The file should not be encrypted.
 * @param hash: hashtable to store the entries.
 * @param keys: list of keys.
 * @param tail: tail of keys list.
 * @param file_name: file name of data file.
 */
int vault_load(vault_hashtable *hash, char *keys[],
               int *tail, char *keyf)
{
  char *line;
  line = (char *)malloc(MAXLINE);
  size_t blen = MAXLINE, len;
  ssize_t cnt;
  char *key = NULL, *value = NULL;
  char *buf;

  FILE *vault;
  vault = fopen(VAULT_FILE_NAME, "r");

  decrypt(vault, &buf, &len, keyf);

  struct vault_entry *e;
  char *del = "\r\n", *saveptr;
  line = strtok_r(buf, del, &saveptr);
  while(line){
    trim(line);
    if (strcmp(line, "") == 0) continue;

    if (is_title(line)) {
      if (key) {
        create_vault_entry(key, value, &e);
        if (*tail == MAXITEMS) {
          fprintf(stderr, "The list of keys is full, \
                  number of items exceeds %d\n", MAXITEMS);
          exit(1);
        }
        keys[*tail] = key;
        (*tail)++;
        value = NULL;
        key = NULL;
        vault_hash_add(hash, vault_compare, vault_choose,
                       e, &e->hash_entry);
      }
      key = (char *)malloc(strlen(line) + 1);
      strcpy(key, line);
    } else {
      /* copy string to another space */
      value = append_values(value, line);
    }
    line = strtok_r(NULL, del, &saveptr);
  }
  if (key) {
    create_vault_entry(key, value, &e);
    keys[*tail] = key;
    *tail += 1;
    vault_hash_add(hash, vault_compare, vault_choose, e,
                   &e->hash_entry);
  }
  free(buf);
}

/*
 * Display entry list.
 */
static void show_entries(char **keys, int tail)
{
  int i = 0;
  while (i != tail) {
    printf("%d: %s\n", i, keys[i]);
    i++;
  }
  printf("Use 'dk n' to show the vaule of it\n");
}

/*
 * Show vaule by index;
 */
static void show_entry(vault_hashtable *hash, char **keys, int tail, int i) {
  if (i >= tail) {
    printf("Index out of range!\n");
    return;
  }
  char *key = keys[i];
  struct vault_entry *node = vault_hash_find(hash, key);
  printf("%s\n\n", node->value);
}

static void prompt()
{
  printf("> ");
}

/*
 * extract substr after pref from str separated by space
 * @param str:  raw str
 * @param pref: prefix to be removed
 */
static bool starts_with(char *str, char *pref) {
  size_t len = strlen(pref);
  char *tmp = str;
  if (strncmp(tmp, pref, len)) {
    return false;
  }
  while (tmp[len] != '\0') {
    tmp[0] = tmp[len];
    tmp++;
  }
  *tmp = '\0';
  trim(str);
  return true;
}

#define MAXCMD 301            /* max length of command line */
#define FILENAME "vaultdata"  /* name of data file */

enum vault_state {
  UNINITIALIZED,
  LOADED,
  EDITED
};

/*
 * Read line from stdin
 */
static char* readstdin(char *buff, size_t *n)
{
  getline(&buff, n, stdin);
  return trim(buff);
}

/*
 * Check nptr is octal number
 */
static bool is_octal(const char *nptr)
{
  while(*nptr != '\0') {
    if (*nptr < '0' || *nptr > '9') {
      return false;
    }
    nptr++;
  }
  return true;
}

static int is_exit(char *cmd)
{
  return strcmp(cmd, "exit") == 0 ||
         strcmp(cmd, "e") == 0;
}

/*
 * Init vault. vault file will be created
 */
static int init(char *inf, char *keyf)
{
  generate_key(keyf);
  fencrypt(inf, VAULT_FILE_NAME, keyf);
  printf("Please securely keep the key file.\n"
    "We could not recover it.");
  return 1;
}

static int enter_cli(char *keyf)
{
  char *keys[MAXITEMS];
  int tail = 0;
  char *buff = (char *)malloc((MAXCMD) * sizeof(char));
  size_t blen = MAXCMD;
  char *cmd = buff;
  enum vault_state state = UNINITIALIZED;

  vault_hashtable *hash = vault_hash_new(HASH_LEN);
  struct vault_entry *e;

  prompt();
  // load vault
  vault_load(hash, keys, &tail, keyf);
  state = LOADED;
  show_entries(keys, tail);

  cmd = readstdin(buff, &blen);
  while (!is_exit(cmd)) {
    if (starts_with(cmd, "load")) {

    } else if (starts_with(cmd, "r")) {
      e = vault_hash_find(hash, cmd);
      if (e) {
        printf("%s:\n---\n%s\n", e->key, e->value);
      }
    } else if (starts_with(cmd, "dk")) {
      if (*cmd == '\0') show_entries(keys, tail);
      else if (is_octal(cmd)) {
        int index = atoi(cmd);
        show_entry(hash, keys, tail, index);
      } else {
        // TODO
      }
    } else if (!strlen(cmd)){
      printf("\n");
    } else {
      printf("Unrecognized command.\n");
    }
    prompt();
    cmd = readstdin(buff, &blen);
  }

  vault_hash_delete(hash, free_vault_entry, NULL);
  hash = NULL;
}

static void export(char *outf, char *keyf)
{

}

static void usage()
{
  printf("vault to keep some secrete items.\n"
   "  init\t-\tinit a vault.\n");
}

int main(int argc, char *argv[])
{
  if (argc == 1) {
    usage();
    return -1;
  }

  if (argc == 2) {
    enter_cli(argv[1]);
    return 0;
  }

  if (strcmp(argv[1], "init") == 0) {
    if (argc != 4) return 1;

    init(argv[2], argv[3]);
    return 0;
  }

  if (strcmp(argv[2], "export") == 0) {
    if (argc != 4) return 1;
    export(argv[2], argv[3]);

    return 0;
  }

  usage();
  return 1;
}
