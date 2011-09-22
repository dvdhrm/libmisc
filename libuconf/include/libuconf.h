/*
 * Universal Config Files Library
 * Written 2011 by David Herrmann
 * Dedicated to the Public Domain
 */

#ifndef UCONF_LIBUCONF_H
#define UCONF_LIBUCONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <libcstr.h>

/*
 * File Management
 * This helps creating, truncating, opening und closing files and provides basic
 * I/O on opened files.
 */

struct uconf_file;

#define UCONF_FILE_READ		0x01	/* open file for reading */
#define UCONF_FILE_WRITE	0x02	/* open file for writing */
#define UCONF_FILE_CREATE	0x04	/* create file if unavailable */
#define UCONF_FILE_EXCLUSIVE	0x08	/* fail if file is available (implies *_CREATE) */
#define UCONF_FILE_TRUNCATE	0x10	/* truncate file before opening */

extern void uconf_file_init(struct uconf_file*);
extern void uconf_file_destroy(struct uconf_file*);
extern struct uconf_file *uconf_file_new();
extern void uconf_file_free(struct uconf_file*);

extern int uconf_file_open(struct uconf_file*, const cstr*, unsigned int);
extern void uconf_file_close(struct uconf_file*);
extern bool uconf_file_opened(struct uconf_file*);
extern const cstr *uconf_file_get_name(struct uconf_file*);
extern int uconf_file_get_fd(struct uconf_file*);

/*
 * Data and Memory Handling
 * This is the actual in memory structure that holds the data. It allows to add,
 * modify and remove any datasets.
 */

struct uconf_entry_list {
	size_t num;
	struct uconf_entry *first;
	struct uconf_entry *last;
};

struct uconf_entry {
	size_t refcount;
	struct uconf_entry *parent;
	struct uconf_entry *next;
	struct uconf_entry *prev;

	cstr *anchor;
	cstr *name;
	unsigned int type;

	union uconf_entry_value {
		struct uconf_entry_list list;
		cstr *qstr;
		int64_t qint;
		long double qfloat;
		cstr *sref;
		struct uconf_entry *ref;
	} v;
};

struct uconf_entry_type {
	int (*init) (struct uconf_entry*);
	void (*destroy) (struct uconf_entry*);
	int (*set) (struct uconf_entry*, va_list);
};

enum uconf_entry_types {
	UCONF_ENTRY_NULL,
	UCONF_ENTRY_LIST,
	UCONF_ENTRY_QSTR,
	UCONF_ENTRY_QINT,
	UCONF_ENTRY_QFLOAT,
	UCONF_ENTRY_SREF,
	UCONF_ENTRY_REF,
	UCONF_ENTRY_NUM
};

extern void uconf_entry_init(struct uconf_entry*);
extern void uconf_entry_destroy(struct uconf_entry*);
extern struct uconf_entry *uconf_entry_new();
extern struct uconf_entry *uconf_entry_ref(struct uconf_entry*);
extern void uconf_entry_unref(struct uconf_entry*);

extern int uconf_entry_set_type(struct uconf_entry*, unsigned int);
extern int uconf_entry_set(struct uconf_entry*, ...);

extern void uconf_entry_link(struct uconf_entry*, struct uconf_entry*,
							struct uconf_entry*);
extern void uconf_entry_unlink(struct uconf_entry*);
extern void uconf_entry_unlink_all(struct uconf_entry*,
					void (*func) (struct uconf_entry*));
extern void uconf_entry_swap(struct uconf_entry*, struct uconf_entry*);
extern void uconf_entry_merge(struct uconf_entry*, struct uconf_entry*);
extern struct uconf_entry *uconf_entry_follow(struct uconf_entry*);

static inline struct uconf_entry *uconf_entry_new_type(unsigned int type)
{
	struct uconf_entry *entry;

	entry = uconf_entry_new();
	if (!entry)
		return NULL;

	if (uconf_entry_set_type(entry, type)) {
		uconf_entry_unref(entry);
		return NULL;
	}

	return entry;
}

static inline void uconf_entry_link_first(struct uconf_entry *parent,
						struct uconf_entry *entry)
{
	uconf_entry_link(parent, NULL, entry);
}

static inline void uconf_entry_link_last(struct uconf_entry *parent,
						struct uconf_entry *entry)
{
	uconf_entry_link(parent, parent->v.list.last, entry);
}

static inline bool uconf_entry_is_list(const struct uconf_entry *entry)
{
	return entry->type == UCONF_ENTRY_LIST;
}

#define UCONF_ENTRY_FOR(parentp, iter) \
		for (iter = (parentp)->v.list.first; iter; iter = iter->next)

/*
 * Parser
 * This parses a file into a tree of uconf_entry structures.
 */

extern int uconf_parse(struct uconf_entry*, struct uconf_file*);
extern int uconf_write(const struct uconf_entry*, struct uconf_file*);

#ifdef __cplusplus
}
#endif

#endif /* UCONF_LIBUCONF_H */
