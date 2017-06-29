#ifndef _VAULT_H_
#define _VAULT_H_

#include "types.h"
#include "enc.h"
#include "hash.h"
#include "compiler.h"

#define HASH_LEN 512
#define KEY_MAX_LEN 100
#define VALUE_MAX_LEN 1000

#define container_of(ptr, type, member) ({      \
  const typeof( ((type *)0)->member ) *__mptr = (ptr);  \
  (type *)( (char *)__mptr - offsetof(type,member) );})

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE, MEMBER)  __compiler_offsetof(TYPE, MEMBER)
#else
#define offsetof(TYPE, MEMBER)  ((size_t)&((TYPE *)0)->MEMBER)
#endif

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned int __u32;

struct __una_u32 { u32 x; } __packed;

#define JHASH_INITVAL           0xdeadbeef

#define __jhash_mix(a, b, c)    \
{   \
  a -= c; a ^= rol32(c, 4);  c += b; \
  b -= a; b ^= rol32(a, 6);  a += c; \
  c -= b; c ^= rol32(b, 8);  b += a; \
  a -= c; a ^= rol32(c, 16); c += b; \
  b -= a; b ^= rol32(a, 19); a += c; \
  c -= b; c ^= rol32(b, 4);  b += a; \
}   \


/**
 * rol32 - rotate a 32-bit value left
 * @word: value to rotate
 * @shift: bits to roll
 */
static inline u32 rol32(u32 word, unsigned int shift)
{
	return (word << shift) | (word >> ((-shift) & 31));
}

static inline u32 __get_unaligned_cpu32(const void *p)
{
	const struct __una_u32 *ptr = (const struct __una_u32 *)p;
	return ptr->x;
}

/* __jhash_final - final mixing of 3 32-bit values (a,b,c) into c */
#define __jhash_final(a, b, c)			\
{						\
	c ^= b; c -= rol32(b, 14);		\
	a ^= c; a -= rol32(c, 11);		\
	b ^= a; b -= rol32(a, 25);		\
	c ^= b; c -= rol32(b, 16);		\
	a ^= c; a -= rol32(c, 4);		\
	b ^= a; b -= rol32(a, 14);		\
	c ^= b; c -= rol32(b, 24);		\
}

static inline u32 jhash(const void *key, u32 length, u32 initval)
{
  u32 a, b, c;
  const u8 *k = key;

  /* Set up the internal state */
  a = b = c = JHASH_INITVAL + length + initval;

  /* All but the last block: affect some 32 bits of (a,b,c) */
  while (length > 12) {
    a += __get_unaligned_cpu32(k);
    b += __get_unaligned_cpu32(k + 4);
    c += __get_unaligned_cpu32(k + 8);
    __jhash_mix(a, b, c);
    length -= 12;
    k += 12;
  }
  /* Last block: affect al 32 bits of (c) */
  /* All the case statements fall through */
  switch (length) {
  case 12: c += (u32)k[11]<<24;
  case 11: c += (u32)k[10]<<16;
  case 10: c += (u32)k[9]<<8;
  case 9: c += (u32)k[8];
  case 8: b += (u32)k[7]<<24;
  case 7: b += (u32)k[6]<<16;
  case 6: b += (u32)k[5]<<8;
  case 5: b += (u32)k[4];
  case 4: a += (u32)k[3]<<24;
  case 3: a += (u32)k[2]<<16;
  case 2: a += (u32)k[1]<<8;
  case 1: a += (u32)k[0];
    __jhash_final(a, b, c);
  case 0: /* Nothing left to add */
    break;
  }

  return c;
}

/* For vault storage. TODO: move those to configuration */
#define MAXLINE 256   /* max length of line in file */
#define MAXITEMS 200  /* max number of items in file */

#endif /* _VAULT_H_ */
