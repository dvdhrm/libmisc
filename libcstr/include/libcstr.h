/*
 * Static C-strings and arrays
 * Written 2011 by David Herrmann
 * Dedicated to the Public Domain
 */

#ifndef CSTR_H
#define CSTR_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct cstr {
	size_t len;
	ssize_t size;
	uint8_t *buf;
} cstr;

#define CSTR_LVALUE(arg_l, arg_s, arg_b) \
			{ .len = arg_l, .size = arg_s, .buf = (void*)arg_b }

#define CSTR_B(len, size, buf) CSTR_BUFFER(len, size, buf)
#define CSTR_BUFFER(len, size, buf) \
					((cstr)CSTR_LVALUE(len, size, buf))

#define CSTR_CB(len, size, buf) CSTR_CONST_BUFFER(len, size, buf)
#define CSTR_CONST_BUFFER(len, size, buf) \
				((const cstr)CSTR_LVALUE(len, size, buf))

#define CSTR_S(str) CSTR_STATIC(str)
#define CSTR_STATIC(str) CSTR_B((sizeof(str) - 1), -(sizeof(str) - 1), (str))
#define CSTR_CS(str) CSTR_CONST_STATIC(str)
#define CSTR_CONST_STATIC(str) \
			CSTR_CB((sizeof(str) - 1), -(sizeof(str) - 1), (str))

#define CSTR_D(str) CSTR_DYNAMIC(str)
#define CSTR_DYNAMIC(str) CSTR_B(strlen(str), -strlen(str), (str))
#define CSTR_CD(str) CSTR_CONST_DYNAMIC(str)
#define CSTR_CONST_DYNAMIC(str) CSTR_CB(strlen(str), -strlen(str), (str))

#define CSTR(str) CSTR_CHAR(str)
#define CSTR_LEN(str) ((str)->len)
#define CSTR_CHAR(str) ((char*)((str)->buf))
#define CSTR_VOID(str) ((void*)((str)->buf))
#define CSTR_UINT8(str) ((uint8_t*)((str)->buf))

extern cstr *cstr_alloc(size_t len, ssize_t size, void *buf);
extern void cstr_dealloc(cstr *str);
extern void cstr_free(cstr *str);
extern bool cstr_fit(cstr *str, size_t len);
extern bool cstr_cfit(cstr *str, size_t len);
extern cstr *cstr_new0(size_t len);
extern cstr *cstr_cnew0(size_t len);
extern cstr *cstr_strdup(ssize_t len, const void *init);
extern cstr *cstr_strcdup(ssize_t len, const void *init);
extern bool cstr_strcat(cstr *str, ssize_t len, const void *cat);
extern bool cstr_strccat(cstr *str, ssize_t len, const void *cat);
extern cstr *cstr_dir(const cstr *str);
extern cstr *cstr_cdir(const cstr *str);
extern bool cstr_strcmp(const cstr *str1, ssize_t len, const void *str2);
extern bool cstr_strcpy(cstr *dest, ssize_t len, const void *src);
extern bool cstr_strccpy(cstr *dest, ssize_t len, const void *src);

static inline bool cstr_cmp(const cstr *str1, const cstr *str2)
{
	return cstr_strcmp(str1, CSTR_LEN(str2), CSTR_VOID(str2));
}

static inline cstr *cstr_new(size_t len)
{
	return cstr_alloc(len, len * 2, NULL);
}

static inline cstr *cstr_cnew(size_t len)
{
	return cstr_alloc(len, len, NULL);
}

static inline cstr *cstr_dup(const cstr *str)
{
	return cstr_strdup(CSTR_LEN(str), CSTR_VOID(str));
}

static inline cstr *cstr_cdup(const cstr *str)
{
	return cstr_strcdup(CSTR_LEN(str), CSTR_VOID(str));
}

static inline bool cstr_cat(cstr *str, const cstr *cat)
{
	return cstr_strcat(str, CSTR_LEN(cat), CSTR_VOID(cat));
}

static inline bool cstr_ccat(cstr *str, const cstr *cat)
{
	return cstr_strccat(str, CSTR_LEN(cat), CSTR_VOID(cat));
}

static inline bool cstr_cpy(cstr *dest, const cstr *src)
{
	return cstr_strcpy(dest, CSTR_LEN(src), CSTR_VOID(src));
}

static inline bool cstr_ccpy(cstr *dest, const cstr *src)
{
	return cstr_strccpy(dest, CSTR_LEN(src), CSTR_VOID(src));
}

#endif /* CSTR_H */
