/*
 * Static C-strings and arrays examples
 * Written 2011 by David Herrmann
 * Dedicated to the Public Domain
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libcstr.h"

#define MEMFAIL2 fail(CSTR_CS("memfail"))
#define MEMFAIL(str) ((str) ? 0 : MEMFAIL2)

/* print \err and abort application */
static void fail(const cstr *err)
{
	printf("ERROR: %s\n", CSTR_CHAR(err));
	abort();
}

/* print \str to stdout with length and size displayed */
static void echo(const cstr *str)
{
	printf("(len: %lu size: %ld buf: '%s')\n", str->len, str->size,
								str->buf);
}

/*<
 * Stress test for \str
 * This resets \str to its previous content afterwards.
 */
static void stress(cstr *str)
{
	size_t i, size;
	cstr *buf;

	echo(str);

	/* make copy to be able to restore the string properly */
	buf = cstr_dup(str);
	MEMFAIL(str);

	size = CSTR_SIZE(str);

	for (i = 0; i < size; ++i)
		str->buf[i] = -1;

	/* also test for the additional NULL byte */
	str->buf[i] = -1;

	if (!cstr_cpy(str, buf))
		MEMFAIL2;

	echo(str);
}

/*
 * Examples for CSTR_STATIC, CSTR_DYNAMIC and CSTR_CONST_*
 * Calling cstr_clear() on the cstr objects is not required in this example as
 * the strings are not modified. However, if stress() (which is called on all
 * strings in this example) would reallocate the object to get a bigger buffer,
 * then we need to free that buffer again.
 * However, most time the caller doesn't know what the function does to the
 * string object so we should always assume that it might reallocate the string
 * and hence we need to free the buffer.
 */
static void example_stack()
{
	/* a static buffer and a static string pointer */
	char buf[] = "buffer";
	char *str = buf;
	const char *constant = "constant";

	/*
	 * Creating static cstr objects with static buffer.
	 * CSTR_STATIC uses sizeof() and CSTR_DYNAMIC uses strlen() so you
	 * should use *_DYNAMIC when using static buffers.
	 *
	 * Passing it directly to a function is unsafe as it may lead to memory
	 * leaks if the function reallocates the string. Casting to constant
	 * beforehand is safe, though.
	 */
	printf("static 1\n");
	cstr a = *CSTR_STATIC(buf);
	cstr b = *CSTR_DYNAMIC(buf);

	stress(&a);
	stress(&b);

	stress(CSTR_S(buf));			/* unsafe */
	echo((const cstr*)CSTR_S(buf));		/* safe */

	stress(CSTR_D(buf));			/* unsafe */
	echo((const cstr*)CSTR_D(buf));		/* safe */

	cstr_clear(&b);
	cstr_clear(&a);

	/*
	 * Creating static cstr objects with static string pointer. We cannot
	 * use CSTR_STATIC as sizeof() doesn't make sense on string pointers to
	 * get the string length. Hence, we use CSTR_DYNAMIC.
	 *
	 * Passing it directly to a function is unsafe as it may lead to memory
	 * leaks if the function reallocates the string. Casting to constant
	 * beforehand is safe, though.
	 */
	printf("static 2\n");
	cstr c = *CSTR_DYNAMIC(str);

	stress(&c);
	stress(CSTR_D(str));			/* unsafe */
	echo((const cstr*)CSTR_D(str));		/* safe */

	cstr_clear(&c);

	/*
	 * \constant is initialized with a static string which is not placed on
	 * the stack (at least GCC doesn't put it there). We cannot modify it so
	 * we need to use CSTR_CONST_DYNAMIC here.
	 *
	 * These macros create constant objects so it is safe to pass them
	 * directly to other functions. No special cast is needed.
	 */
	printf("static 3\n");
	const cstr d = *CSTR_CONST_DYNAMIC(constant);

	echo(&d);
	echo(CSTR_CD(constant)); /* safe */
	echo(CSTR_CS("const")); /* safe */
	/* no cstr_clear needed as the objects are constant */
}

/*
 * Examples for dynamic string creation
 * cstr_new, cstr_alloc and cstr_dup
 */
static void example_heap()
{
	/* new strings from scratch */
	printf("dynamic 1\n");
	cstr *a = cstr_new(10);
	MEMFAIL(a);

	/*
	 * strcpy may only fail if the destination target is not big enough for
	 * the new string and reallocation fails. Our buffer is big enough so we
	 * do not need this check. We do it anyway.
	 */
	if (!cstr_cpy(a, CSTR("hello!")))
		MEMFAIL2;

	cstr *b = cstr_dup(a);
	MEMFAIL(b);

	stress(a);
	stress(b);

	cstr_free(b);
	cstr_free(a);

	/* same with constant strings (notice the size difference) */
	printf("dynamic 2\n");
	a = cstr_cnew(10);
	MEMFAIL(a);

	if (!cstr_ccpy(a, CSTR("hello!")))
		MEMFAIL2;

	b = cstr_cdup(a);
	MEMFAIL(b);

	stress(a);
	stress(b);

	cstr_free(b);
	cstr_free(a);

	/* miscellaneous functions */
	printf("dynamic 3\n");
	a = cstr_dup(CSTR("Hello World!"));
	MEMFAIL(a);

	stress(a);

	if (!cstr_ccat(a, CSTR(" This is the future and more...")))
		MEMFAIL2;

	stress(a);

	cstr_free(a);
}

int main(int argc, char **argv)
{
	printf("stack examples\n");
	example_stack();
	printf("heap examples\n");
	example_heap();

	return EXIT_SUCCESS;
}
