/*
 * Universal Config Files Library
 * Written 2011 by David Herrmann
 * Dedicated to the Public Domain
 */

/*
 * File Management
 * Implements opening and closing files and raw I/O.
 */

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <libcstr.h>

#include "libuconf.h"
#include "uconf.h"

/*
 * Datatypes
 * Datatype support is modular. You can add new types easily. Each type has to
 * provide the following functions:
 *  - .init: Initialize required memory.
 *  - .destroy: Free all allocated memory.
 *
 * The *_NULL type is the most basic type. It does not provide any data. It can
 * be set without calling .init so it is a safe fallback. The
 * *_LIST type is the recursion-type. It uses the ->list member instead of the
 * ->value member to provide a list of child elements.
 */

static int t_list_init(struct uconf_entry *entry)
{
	entry->v.list.first = NULL;
	entry->v.list.last = NULL;
	return 0;
}

static void t_list_destroy(struct uconf_entry *entry)
{
	uconf_entry_unlink_all(entry, NULL);
	entry->v.list.first = NULL;
	entry->v.list.last = NULL;
}

static int t_qstr_init(struct uconf_entry *entry)
{
	entry->v.qstr = NULL;
	return 0;
}

static void t_qstr_destroy(struct uconf_entry *entry)
{
	cstr_free(entry->v.qstr);
}

static int t_sref_init(struct uconf_entry *entry)
{
	entry->v.sref = NULL;
	return 0;
}

static void t_sref_destroy(struct uconf_entry *entry)
{
	cstr_free(entry->v.sref);
}

static int t_ref_init(struct uconf_entry *entry)
{
	entry->v.ref = NULL;
	return 0;
}

static void t_ref_destroy(struct uconf_entry *entry)
{
	uconf_entry_unref(entry->v.ref);
}

static struct uconf_entry_type type_list[] = {
	[UCONF_ENTRY_NULL] = {
		.init = NULL,
		.destroy = NULL,
		.set = NULL,
	},
	[UCONF_ENTRY_LIST] = {
		.init = t_list_init,
		.destroy = t_list_destroy,
		.set = NULL,
	},
	[UCONF_ENTRY_QSTR] = {
		.init = t_qstr_init,
		.destroy = t_qstr_destroy,
		.set = NULL,
	},
	[UCONF_ENTRY_QINT] = {
		.init = NULL,
		.destroy = NULL,
		.set = NULL,
	},
	[UCONF_ENTRY_QFLOAT] = {
		.init = NULL,
		.destroy = NULL,
		.set = NULL,
	},
	[UCONF_ENTRY_SREF] = {
		.init = t_sref_init,
		.destroy = t_sref_destroy,
		.set = NULL,
	},
	[UCONF_ENTRY_REF] = {
		.init = t_ref_init,
		.destroy = t_ref_destroy,
		.set = NULL,
	},
};

#define TYPE_INIT(entry) (type_list[(entry)->type].init(entry))
#define TYPE_DESTROY(entry) (type_list[(entry)->type].destroy(entry))
#define TYPE_SET(entry, arg) (type_list[(entry)->type].set((entry), (arg)))

/* initialize \entry and set to type *_NULL */
void uconf_entry_init(struct uconf_entry *entry)
{
	memset(entry, 0, sizeof(*entry));
	uconf_entry_ref(entry);
}

/* unref and deinitialize \entry and free all allocated memory */
void uconf_entry_destroy(struct uconf_entry *entry)
{
	assert(entry->refcount);

	if (--entry->refcount)
		return;

	uconf_entry_set_type(entry, UCONF_ENTRY_NULL);
	uconf_entry_unlink(entry);
	cstr_free(entry->name);
	cstr_free(entry->anchor);
}

/* same as uconf_entry_init() but allocate dynamically */
struct uconf_entry *uconf_entry_new()
{
	struct uconf_entry *entry;

	entry = malloc(sizeof(*entry));
	if (!entry)
		return NULL;

	uconf_entry_init(entry);
	return entry;
}

struct uconf_entry *uconf_entry_ref(struct uconf_entry *entry)
{
	entry->refcount++;
	assert(entry->refcount);
	return entry;
}

void uconf_entry_unref(struct uconf_entry *entry)
{
	if (!entry)
		return;

	uconf_entry_destroy(entry);
	if (!entry->refcount)
		free(entry);
}

/* set type of \entry to \type */
int uconf_entry_set_type(struct uconf_entry *entry, unsigned int type)
{
	int ret = 0;

	assert(type < UCONF_ENTRY_NUM);

	if (entry->type == type)
		return 0;

	if (type_list[entry->type].destroy)
		TYPE_DESTROY(entry);

	entry->type = type;
	if (type_list[entry->type].init) {
		ret = TYPE_INIT(entry);
		if (ret)
			entry->type = UCONF_ENTRY_NULL;
	}

	return ret;
}

/* set value of entry */
int uconf_entry_set(struct uconf_entry *entry, ...)
{
	va_list list;
	int ret = 0;

	if (type_list[entry->type].set) {
		va_start(list, entry);
		ret = TYPE_SET(entry, list);
		va_end(list);
	}

	return ret;
}

/*
 * Link \entry as child of \parent before \rel. If \rel is NULL then \entry is
 * linked as first child in the list of childs of \parent.
 * This calls uconf_entry_ref(entry) to take a reference so the caller may drop
 * his reference.
 *
 * If \entry is statically initialized and/or is associated with other allocated
 * data than you must go sure to unlink \entry from the parent before removing
 * the parent.
 * If the parent gets destroyed, then all childs are simply unrefed with
 * uconf_entry_unref() so take that into account!
 */
void uconf_entry_link(struct uconf_entry *parent, struct uconf_entry *rel,
						struct uconf_entry *entry)
{
	assert(parent->type == UCONF_ENTRY_LIST);
	assert(!entry->parent && !entry->next && !entry->prev);
	assert(!rel || rel->parent == parent);

	entry = uconf_entry_ref(entry);
	entry->parent = parent;

	if (rel) {
		/* insert behind \rel */
		entry->prev = rel;
		entry->next = rel->next;
		if (rel->next)
			rel->next->prev = entry;
		rel->next = entry;
	} else {
		/* insert first */
		rel = parent->v.list.first;
		if (rel) {
			/* insert before \rel */
			assert(!rel->prev);

			entry->prev = NULL;
			entry->next = rel;
			rel->prev = entry;
		} else {
			/* empty list */
			entry->next = NULL;
			entry->prev = NULL;
		}
	}

	if (!entry->next)
		parent->v.list.last = entry;
	if (!entry->prev)
		parent->v.list.first = entry;
}

/*
 * Unlink \entry from its parent, this calls uconf_entry_unref() so you should
 * call uconf_entry_ref() if you want to keep the entry alive and don't own
 * another reference!
 */
void uconf_entry_unlink(struct uconf_entry *entry)
{
	struct uconf_entry *parent = entry->parent;

	if (parent) {
		if (entry->next)
			entry->next->prev = entry->prev;
		if (entry->prev)
			entry->prev->next = entry->next;
		if (parent->v.list.first == entry)
			parent->v.list.first = entry->next;
		if (parent->v.list.last == entry)
			parent->v.list.last = entry->prev;
		entry->next = NULL;
		entry->prev = NULL;
		entry->parent = NULL;
		uconf_entry_unref(entry);
	} else {
		assert(!entry->next);
		assert(!entry->prev);
	}
}

/*
 * Unlink all childs of \parent and call \func on each child before it is
 * unlinked.
 */
void uconf_entry_unlink_all(struct uconf_entry *parent,
					void (*func) (struct uconf_entry*))
{
	struct uconf_entry *iter, *tmp;

	assert(parent->type == UCONF_ENTRY_LIST);

	iter = parent->v.list.first;
	while (iter) {
		tmp = iter;
		iter = iter->next;

		if (func)
			func(tmp);
		uconf_entry_unlink(tmp);
	}
}

/* reset parent-pointer of all childs from \e to \e */
static void fix_parentp(struct uconf_entry *e)
{
	struct uconf_entry *iter;

	if (e->type == UCONF_ENTRY_LIST) {
		iter = e->v.list.first;
		while (iter) {
			iter->parent = e;
			iter = iter->next;
		}
	}
}

/*
 * Swap two entries and make sure the child's \parent pointers are also moved.
 * This only checks for list-entries. If other values also have parent pointers
 * or even more, this function should be fixed.
 */
void uconf_entry_swap(struct uconf_entry *e1, struct uconf_entry *e2)
{
	struct uconf_entry tmp;

	memcpy(&tmp, e1, sizeof(tmp));
	memcpy(e1, e2, sizeof(tmp));
	memcpy(e2, &tmp, sizeof(tmp));

	fix_parentp(e1);
	fix_parentp(e2);
}

/*
 * This removes all childs from \src and adds them as new childs to \dest. \dest
 * is forced to type UCONF_ENTRY_LIST, but previous childs are preserved. If
 * \src is not of type UCONF_ENTRY_LIST, nothing is done.
 * The entries are added at the end of \dest but the order is preserved.
 */
void uconf_entry_merge(struct uconf_entry *dest, struct uconf_entry *src)
{
	struct uconf_entry *tmp;

	uconf_entry_set_type(dest, UCONF_ENTRY_LIST);
	if (src->type != UCONF_ENTRY_LIST)
		return;

	while (src->v.list.first) {
		tmp = src->v.list.first;
		uconf_entry_ref(tmp);
		uconf_entry_unlink(tmp);
		uconf_entry_link_last(dest, tmp);
		uconf_entry_unref(tmp);
	}

	fix_parentp(dest);
}

/*
 * Find single destination of soft reference \sref. This searches the tree \tree
 * recursively (only downwards).
 * Returns NULL if the entry cannot be found.
 */
static struct uconf_entry *find_sref(struct uconf_entry *tree, const cstr *sref)
{
	struct uconf_entry *iter, *res;

	if (tree->anchor && cstr_cmp(tree->anchor, sref))
		return tree;

	if (tree->type != UCONF_ENTRY_LIST)
		return NULL;

	for (iter = tree->v.list.first; iter; iter = iter->next) {
		res = find_sref(iter, sref);
		if (res)
			return res;
	}

	return NULL;
}

/*
 * This finds the root of \entry.
 */
static struct uconf_entry *find_root(struct uconf_entry *entry)
{
	while (entry->parent)
		entry = entry->parent;

	return entry;
}

/*
 * If \entry is a hard reference, it is returned. If it is a soft reference, the
 * soft reference is resolved to a hard reference and returned. If that fails,
 * NULL is returned. If it is no reference at all, it is returned.
 *
 * This is recursive. If the destination is again a reference, it is also
 * resolved. Max recursion depth is limited, though, so no endless references
 * are possible (loops etc.).
 */
static struct uconf_entry *solve_ref(struct uconf_entry *entry, size_t depth)
{
	if (!entry || entry->type != UCONF_ENTRY_SREF)
		return entry;

	if (depth > 10)
		return NULL;

	return solve_ref(find_sref(find_root(entry), entry->v.sref), depth + 1);
}

/* see \solve_ref() */
struct uconf_entry *uconf_entry_follow(struct uconf_entry *entry)
{
	return solve_ref(entry, 0);
}
