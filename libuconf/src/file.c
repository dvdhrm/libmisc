/*
 * Universal Config Files Library
 * Written 2011 by David Herrmann
 * Dedicated to the Public Domain
 */

/*
 * File Management
 * Implements opening and closing files and raw I/O.
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <libcstr.h>
#include <unistd.h>

#include "uconf.h"
#include "libuconf.h"

struct uconf_file {
	signed int fd;
	cstr *name;
	unsigned int flags;
};

void uconf_file_init(struct uconf_file *file)
{
	file->fd = -1;
	file->name = NULL;
	file->flags = 0;
}

void uconf_file_destroy(struct uconf_file *file)
{
	uconf_file_close(file);
}

struct uconf_file *uconf_file_new()
{
	struct uconf_file *file;

	file = malloc(sizeof(*file));
	if (!file)
		return NULL;

	uconf_file_init(file);
	return file;
}

void uconf_file_free(struct uconf_file *file)
{
	uconf_file_destroy(file);
	free(file);
}

int uconf_file_open(struct uconf_file *file, const cstr *path,
							unsigned int flags)
{
	const int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	int raw_flags = O_CLOEXEC | O_NONBLOCK;
	int ret;

	uconf_file_close(file);

	/* one of *_READ or *_WRITE must be given */
	if (flags & UCONF_FILE_READ) {
		if (flags & UCONF_FILE_WRITE)
			raw_flags |= O_RDWR;
		else
			raw_flags |= O_RDONLY;
	} else if (flags & UCONF_FILE_WRITE) {
		raw_flags |= O_WRONLY;
	} else {
		return -EINVAL;
	}

	if (flags & UCONF_FILE_CREATE)
		raw_flags |= O_CREAT;
	if (flags & UCONF_FILE_EXCLUSIVE)
		raw_flags |= O_CREAT | O_EXCL;
	if (flags & UCONF_FILE_TRUNCATE)
		raw_flags |= O_TRUNC;

	file->name = cstr_cdup(path);
	if (!file->name)
		return -ENOMEM;

	file->fd = open(CSTR_CHAR(path), raw_flags, mode);
	if (file->fd < 0) {
		ret = -errno;
		goto err;
	}

	file->flags = flags;

	return 0;

err:
	free(file->name);
	file->name = NULL;
	return ret;
}

void uconf_file_close(struct uconf_file *file)
{
	if (uconf_file_opened(file)) {
		UCONF_NO_EINTR(close(file->fd));
		file->fd = -1;
		cstr_free(file->name);
		file->name = NULL;
		file->flags = 0;
	}
}

bool uconf_file_opened(struct uconf_file *file)
{
	return file->fd >= 0;
}

const cstr *uconf_file_get_name(struct uconf_file *file)
{
	return file->name;
}

int uconf_file_get_fd(struct uconf_file *file)
{
	return file->fd;
}

unsigned int uconf_file_get_flags(struct uconf_file *file)
{
	return file->flags;
}
