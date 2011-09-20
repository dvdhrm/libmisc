/*
 * Static C-strings and arrays
 * Written 2011 by David Herrmann
 * Dedicated to the Public Domain
 */

/*
 * C-string helper functions. Each string consists of a char-array and a size_t
 * integer which contains the string length. This allows the string to contain
 * \0 characters anywhere in the buffer. For faster string modifications, there
 * is also a size member which contains the currently allocated string size.
 * This is always twice as much as the string size on allocation. For constant
 * strings, this is always the same as the string size to reduce memory
 * consumption.
 * For compatibility reasons, every string still contains a terminating 0
 * character, which, however, is not considered part of the string and hence not
 * included in the string length.
 *
 * Compatibility functions for classic C-strings are prefixed with "str". They
 * accept a "len" parameter for the passed c-string which can be <0, so strlen()
 * is used to determine the size. For constant strings, the function is prefixed
 * with "c". Constant strings do not allocate additional buffer size for faster
 * string modifications, however, they allow the same modifications as normal
 * cstrs, but it may be way slower.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "libcstr.h"

cstr *cstr_alloc(size_t len, ssize_t size, void *buf)
{
	cstr *str;

	assert(len <= size || len <= -size);

	str = malloc(sizeof(*str));
	if (!str)
		return NULL;

	if (size < 0)
		str->buf = buf;
	else
		str->buf = malloc(size + 1);

	if (!str->buf) {
		free(str);
		return NULL;
	}

	str->len = len;
	str->size = size;
	str->buf[len] = 0;

	return str;
}

void cstr_dealloc(cstr *str)
{
	if (str) {
		if (str->size >= 0)
			free(str->buf);
		str->buf = NULL;
	}
}

void cstr_free(cstr *str)
{
	if (str) {
		cstr_dealloc(str);
		free(str);
	}
}

static bool fit(cstr *str, size_t len, bool constant)
{
	void *snew;
	size_t size;

	if (str->size < (ssize_t)len && (-str->size) < (ssize_t)len) {
		if (constant)
			size = len;
		else
			size = len * 2;

		if (str->size < 0)
			snew = malloc(size + 1);
		else
			snew = realloc(str->buf, size + 1);
		if (!snew)
			return false;
		str->buf = snew;
		str->size = size;
	}

	str->len = len;
	str->buf[len] = 0;

	return true;
}

bool cstr_fit(cstr *str, size_t len)
{
	return fit(str, len, false);
}

bool cstr_cfit(cstr *str, size_t len)
{
	return fit(str, len, true);
}

cstr *cstr_new0(size_t len)
{
	cstr *str;

	str = cstr_new(len);
	if (!str)
		return NULL;

	memset(CSTR_VOID(str), 0, len);
	return str;
}

cstr *cstr_cnew0(size_t len)
{
	cstr *str;

	str = cstr_cnew(len);
	if (!str)
		return NULL;

	memset(CSTR_VOID(str), 0, len);
	return str;
}

cstr *cstr_strdup(ssize_t len, const void *init)
{
	cstr *str;

	if (len < 0)
		len = strlen(init);

	str = cstr_new(len);
	if (!str)
		return NULL;

	memcpy(CSTR_VOID(str), init, len);
	return str;
}

cstr *cstr_strcdup(ssize_t len, const void *init)
{
	cstr *str;

	if (len < 0)
		len = strlen(init);

	str = cstr_cnew(len);
	if (!str)
		return NULL;

	memcpy(CSTR_VOID(str), init, len);
	return str;
}

bool cstr_strcat(cstr *str, ssize_t len, const void *cat)
{
	size_t slen = CSTR_LEN(str);

	if (len < 0)
		len = strlen(cat);

	if (!cstr_fit(str, slen + len))
		return false;

	memcpy(&str->buf[slen], cat, len);
	return true;
}

bool cstr_strccat(cstr *str, ssize_t len, const void *cat)
{
	size_t slen = CSTR_LEN(str);

	if (len < 0)
		len = strlen(cat);

	if (!cstr_cfit(str, slen + len))
		return false;

	memcpy(&str->buf[slen], cat, len);
	return true;
}

static inline cstr *dir(const cstr *str, bool constant)
{
	cstr *dir;
	size_t len, last = 0;

	for (len = 0; len < CSTR_LEN(str); ++len) {
		if (str->buf[len] == '/')
			last = len;
	}

	/* if result is root directory, we need to copy the trailing '/' */
	if (!last && str->buf[0] == '/') {
		if (constant)
			return cstr_strcdup(-1, "/");
		else
			return cstr_strdup(-1, "/");
	}

	if (constant)
		dir = cstr_strcdup(last, CSTR_VOID(str));
	else
		dir = cstr_strdup(last, CSTR_VOID(str));

	return dir;
}

cstr *cstr_dir(const cstr *str)
{
	return dir(str, false);
}

cstr *cstr_cdir(const cstr *str)
{
	return dir(str, true);
}

bool cstr_strcmp(const cstr *str1, ssize_t len, const void *str2)
{
	if (len < 0)
		len = strlen(str2);

	if (CSTR_LEN(str1) != len)
		return false;

	return !memcmp(CSTR_VOID(str1), str2, len);
}

static bool copy(cstr *dest, ssize_t len, const void *src, bool constant)
{
	if (len < 0)
		len = strlen(src);

	if (!fit(dest, len, constant))
		return false;

	memcpy(CSTR_VOID(dest), src, len);
	return true;
}

bool cstr_strcpy(cstr *dest, ssize_t len, const void *src)
{
	return copy(dest, len, src, false);
}

bool cstr_strccpy(cstr *dest, ssize_t len, const void *src)
{
	return copy(dest, len, src, true);
}
