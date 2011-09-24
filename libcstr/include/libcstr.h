/*
 * Static C-strings and arrays
 * Written 2011 by David Herrmann
 * Dedicated to the Public Domain
 */

#ifndef CSTR_LIBCSTR_H
#define CSTR_LIBCSTR_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct cstr {
	size_t len;
	ssize_t size;
	uint8_t *buf;
} cstr;

#define CSTR__LVALUE(arg_l, arg_s, arg_b) \
			{ .len = arg_l, .size = arg_s, .buf = (void*)arg_b }

#define CSTR__B(len, size, buf) CSTR__BUFFER(len, size, buf)
#define CSTR__BUFFER(len, size, buf) \
					(&(cstr)CSTR__LVALUE(len, size, buf))
#define CSTR__CB(len, size, buf) CSTR__CONST_BUFFER(len, size, buf)
#define CSTR__CONST_BUFFER(len, size, buf) \
				(&(const cstr)CSTR__LVALUE(len, size, buf))

#define CSTR_S(str) CSTR_STATIC(str)
#define CSTR_STATIC(str) CSTR__B((sizeof(str) - 1), -(sizeof(str) - 1), (str))
#define CSTR_CS(str) CSTR_CONST_STATIC(str)
#define CSTR_CONST_STATIC(str) \
			CSTR__CB((sizeof(str) - 1), -(sizeof(str) - 1), (str))

#define CSTR_D(str) CSTR_DYNAMIC(str)
#define CSTR_DYNAMIC(str) CSTR__B(strlen(str), -strlen(str), (str))
#define CSTR_CD(str) CSTR_CONST_DYNAMIC(str)
#define CSTR_CONST_DYNAMIC(str) CSTR__CB(strlen(str), -strlen(str), (str))

#define CSTR_LEN(str) ((str)->len)
#define CSTR_SIZE(str) (abs((str)->size))
#define CSTR_CHAR(str) ((char*)((str)->buf))
#define CSTR_VOID(str) ((void*)((str)->buf))
#define CSTR_UINT8(str) ((uint8_t*)((str)->buf))

#define CSTR(ptr) CSTR_CD(ptr)

extern cstr *cstr_alloc(size_t len, ssize_t size, void *buf);
extern void cstr_clear(cstr *str);
extern void cstr_free(cstr *str);
extern bool cstr__fit(cstr *str, size_t len, bool constant);
extern cstr *cstr__dup(const cstr *old, bool constant);
extern bool cstr__cat(cstr *dest, const cstr *src, bool constant);
extern bool cstr_cmp(const cstr *str1, const cstr *str2);
extern bool cstr_ncmp(const cstr *str1, const cstr *str2, size_t n);
extern bool cstr__cpy(cstr *dest, const cstr *src, bool constant);

static inline bool cstr_fit(cstr *str, size_t len)
	{ return cstr__fit(str, len, false); }
static inline bool cstr_cfit(cstr *str, size_t len)
	{ return cstr__fit(str, len, true); }

static inline cstr *cstr_new(size_t len)
	{ return cstr_alloc(len, len * 2, NULL); }
static inline cstr *cstr_cnew(size_t len)
	{ return cstr_alloc(len, len, NULL); }

static inline cstr *cstr_dup(const cstr *old)
	{ return cstr__dup(old, false); }
static inline cstr *cstr_cdup(const cstr *old)
	{ return cstr__dup(old, true); }

static inline bool cstr_cat(cstr *dest, const cstr *src)
	{ return cstr__cat(dest, src, false); }
static inline bool cstr_ccat(cstr *dest, const cstr *src)
	{ return cstr__cat(dest, src, true); }

static inline bool cstr_cpy(cstr *dest, const cstr *src)
	{ return cstr__cpy(dest, src, false); }
static inline bool cstr_ccpy(cstr *dest, const cstr *src)
	{ return cstr__cpy(dest, src, true); }

#endif /* CSTR_LIBCSTR_H */
