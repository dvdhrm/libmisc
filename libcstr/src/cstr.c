/*
 * Static C-strings and arrays
 * Written 2011 by David Herrmann
 * Dedicated to the Public Domain
 */

/*
 * CSTR Objects
 * A cstr object consists of a buffer, the size of the buffer and the length of
 * the string. The length is always smaller or equal to the absolute size of the
 * buffer. The length may be 0.
 * The buffer pointer is always non-NULL and points to a valid buffer. The
 * buffer size is either the absolute size of the buffer minus 1 or its
 * negative. If the size is negative, this means that the buffer is not owned by
 * us and that we should not call free() on it. We simply drop it and allocate a
 * new buffer if we need to resize our buffer.
 * If the size is zero or bigger, it means that our buffer is allocated by us
 * and that we need to free it with free() when no longer used.
 * The actual size of the buffer is abs(obj->size) + 1. We keep a +1 for the
 * terminating zero for backwards compatibility.
 *
 * To speed up string manipulations, the buffer is often twice the size as
 * required. However, many strings do not need to be manipulated and hence do
 * not need a big buffer. Therefore, many functions take as last argument a
 * \constant boolean which should be true if no additional buffer space should
 * be allocated. If it is false, the buffer size is always twice as big as
 * requested. The default value is false.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "libcstr.h"

/*
 * Allocates a new cstr object on the heap.
 * The cstr object will have length \len. If \size is greater than or equal to
 * zero and \buf is NULL, then a new buffer is allocated. If \buf is non-NULL,
 * the buffer is set to \buf and we own it now (that is, we free it if we no
 * longer need it).
 * If \size is smaller than 0, then \buf is taken as constant buffer which we do
 * not own.
 * In all cases, abs(\size) must be greater than or equal to \len.
 * If \buf is non-NULL, then buffer pointed to by \buf must be at least of size
 * abs(\size) + 1. The +1 is important!
 *
 * This returns NULL on memory allocation errors, otherwise it returns the new
 * cstr object.
 */
cstr *cstr_alloc(size_t len, ssize_t size, void *buf)
{
	cstr *str;

	assert(len <= abs(size));

	str = malloc(sizeof(*str));
	if (!str)
		return NULL;

	if (size >= 0 && !buf) {
		str->buf = malloc(size + 1);
		if (!str->buf) {
			free(str);
			return NULL;
		}
	} else {
		assert(buf);
		str->buf = buf;
	}

	str->len = len;
	str->size = size;
	str->buf[len] = 0;

	return str;
}

/*
 * This frees the allocated buffer of \str. It does not free the cstr object.
 * However, the cstr object is invalid after this call and must not be used,
 * anymore. The only valid function that may be called on the cstr object after
 * this is cstr_free().
 * You do not need to call this on dynamically allocated cstr objects as
 * cstr_free() also frees the buffers. However, statically allocated cstr
 * objects which are not constant must be freed with this function.
 */
void cstr_clear(cstr *str)
{
	if (str->size >= 0)
		free(str->buf);
	str->len = 0;
	str->size = 0;
	str->buf = NULL;
}

/*
 * Frees all allocated buffers of \str and the object itself.
 * After this call \str should not be used anymore.
 */
void cstr_free(cstr *str)
{
	if (str) {
		cstr_clear(str);
		free(str);
	}
}

/*
 * Resize cstr buffer
 * This resizes the buffer of \str to make room for at least \len bytes. The
 * string length is also reset to \len and the zero terminating character is set
 * to the end of the new buffer.
 * If the current buffer is big enough, no new buffer is allocated. Only if we
 * need more space, the current buffer is freed or dropped and a new buffer is
 * allocated.
 * This returns false if the new buffer cannot be allocated. Otherwise it
 * returns true.
 * If a new buffer is allocated and \constant is true, the new buffer will have
 * the exact same size as required. If \constant is false, the buffer will be
 * twice as big as required.
 * \str is not touched at all if the memory allocation fails and false is
 * returned. So on failure the old state is preserved.
 */
bool cstr__fit(cstr *str, size_t len, bool constant)
{
	void *snew;
	size_t size;

	assert(str);

	if (abs(str->size) < (ssize_t)len) {
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

/*
 * Duplicate string
 * This creates a duplicate of \old and returns it. The new object is in no way
 * dependent of \old and can be used as usual.
 * Returns NULL on memory allocation errors.
 * The buffer of the new object may be of different size as \old. \constant
 * specifies how the new buffer is allocated.
 */
cstr *cstr__dup(const cstr *old, bool constant)
{
	cstr *dest;

	assert(old);

	if (constant)
		dest = cstr_cnew(CSTR_LEN(old));
	else
		dest = cstr_new(CSTR_LEN(old));
	if (!dest)
		return NULL;

	memcpy(CSTR_VOID(dest), CSTR_VOID(old), CSTR_LEN(old));
	return dest;
}

/*
 * Concatenate strings
 * This puts \src at the end of \dest. \constant controls how the buffer of
 * \dest is reallocated if required.
 */
bool cstr__cat(cstr *dest, const cstr *src, bool constant)
{
	size_t dlen = CSTR_LEN(dest);

	if (!cstr__fit(dest, dlen + CSTR_LEN(src), constant))
		return false;

	memcpy(CSTR_UINT8(dest) + dlen, CSTR_VOID(src), CSTR_LEN(src));
	return true;
}

/*
 * Compare strings
 * Returns true if \str1 and \str2 are equal.
 */
bool cstr_cmp(const cstr *str1, const cstr *str2)
{
	if (CSTR_LEN(str1) != CSTR_LEN(str2))
		return false;

	return !memcmp(CSTR_VOID(str1), CSTR_VOID(str2), CSTR_LEN(str1));
}

/*
 * Compare substrings
 * Compares the first \n characters of both strings for equality.
 * Returns true if they are equal.
 * If \n is bigger than the length of any of both strings, then this is
 * identical to cstr_cmp().
 */
bool cstr_ncmp(const cstr *str1, const cstr *str2, size_t n)
{
	if (CSTR_LEN(str1) < n || CSTR_LEN(str2) < n)
		return cstr_cmp(str1, str2);

	return !memcmp(CSTR_VOID(str1), CSTR_VOID(str2), n);
}

/*
 * Copy string
 * This copies \src into \dest. \constant controls how the new buffer is
 * allocated.
 * This function is similar to cstr_dup() but takes an existing object instead
 * of creating a new object.
 * Returns true on success and false on memory allocation failure.
 */
bool cstr__cpy(cstr *dest, const cstr *src, bool constant)
{
	if (!cstr__fit(dest, CSTR_LEN(src), constant))
		return false;

	memcpy(CSTR_VOID(dest), CSTR_VOID(src), CSTR_LEN(src));
	return true;
}
