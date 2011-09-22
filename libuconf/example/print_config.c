/*
 * Universal Config Files Library
 * Written 2011 by David Herrmann
 * Dedicated to the Public Domain
 */

/*
 * UConf Example
 * This takes as first argument a filename which is opened and parsed with the
 * uconf library. The parsed content is printed to stdout.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libcstr.h>
#include <libuconf.h>

static const char *get_name(const struct uconf_entry *entry)
{
	if (entry->name)
		return CSTR_CHAR(entry->name);
	else
		return "<anon>";
}

static void print_rec(struct uconf_entry *entry, const cstr *p)
{
	struct uconf_entry *iter, *ref;
	cstr *n;

	printf("%sEntry: %s", p->buf, get_name(entry));
	if (entry->anchor)
		printf(" (anchor: %s)", CSTR_CHAR(entry->anchor));
	printf(" = ");

	switch (entry->type) {
		case UCONF_ENTRY_NULL:
			printf("(null)\n");
			break;
		case UCONF_ENTRY_QINT:
			printf("%ld (qint)\n", entry->v.qint);
			break;
		case UCONF_ENTRY_QFLOAT:
			printf("%Lf (qfloat)\n", entry->v.qfloat);
			break;
		case UCONF_ENTRY_QSTR:
			printf("%s (qstr: %ld)\n", entry->v.qstr->buf,
							entry->v.qstr->len);
			break;
		case UCONF_ENTRY_SREF:
			ref = uconf_entry_follow(entry);
			printf("%s (sref: %s)\n",
				ref ? get_name(ref) : "<none>",
							entry->v.sref->buf);
			break;
		case UCONF_ENTRY_LIST:
			printf("(list: %lu)\n", entry->v.list.num);
			n = cstr_dup(p);
			if (!n || !cstr_strccat(n, -1, "  ")) {
				printf("Error: Memory allocation failed\n");
				return;
			}
			for (iter = entry->v.list.first; iter;
							iter = iter->next)
				print_rec(iter, n);
			cstr_free(n);
			break;
		default:
			printf("(unknown)\n");
			break;
	}
}

static void print_root(struct uconf_entry *entry)
{
	cstr *p;

	p = cstr_strcdup(-1, "  ");
	if (!p) {
		printf("Error: Memory allocation failed\n");
		return;
	}

	printf("Print root entry:\n");
	print_rec(entry, p);
}

static void parse_file(struct uconf_file *file)
{
	struct uconf_entry *root;
	int ret;

	root = uconf_entry_new();
	if (!root) {
		printf("Error: Memory allocation failed\n");
		return;
	}

	ret = uconf_parse(root, file);
	if (ret)
		printf("Error: Parser failed %d\n", ret);
	else
		print_root(root);

	uconf_entry_unref(root);
}

int main(int argc, char **argv)
{
	struct uconf_file *file;
	int ret;
	cstr *path;

	if (argc < 2) {
		printf("Usage: %s <configfile>\n",
					argc ? argv[0] : "./print_config");
		return EXIT_FAILURE;
	}

	file = uconf_file_new();
	if (!file) {
		printf("Error: Memory allocation failed\n");
		return EXIT_FAILURE;
	}

	path = cstr_strcdup(-1, argv[1]);
	if (!path) {
		uconf_file_free(file);
		printf("Error: Memory allocation failed\n");
		return EXIT_FAILURE;
	}

	ret = uconf_file_open(file, path, UCONF_FILE_READ);
	if (ret)
		printf("Error: Cannot open file %s (%d)\n", path->buf, ret);
	else
		parse_file(file);

	uconf_file_free(file);
	return EXIT_SUCCESS;
}
