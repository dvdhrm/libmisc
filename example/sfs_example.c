/*
 * Sysfs Helper Examples
 * Written 2011 by David Herrmann
 * Dedicated to the Public Domain
 */

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "libsfs.h"

static int example_dir_foreach(const char *path, const struct dirent *ent,
								void *extra)
{
	printf("entry: %s\n", ent->d_name);

	return 0;
}

/*
 * Show how to use sfs_dir_*() functions.
 * Trivial examples for the easy helper functions to traverse sysfs directories.
 * This can be used for any other directory, too, and is not limited to sysfs.
 */
static int example_dir()
{
	int ret;
	const char *path = "/sys";

	printf("sfs_dir_foreach(\"%s\"):\n", path);
	ret = sfs_dir_foreach(path, example_dir_foreach, NULL);
	printf("return: %d\n", ret);

	return 0;
}

static int example_input_foreach(struct sfs_input_dev *dev, void *extra)
{
	printf("input path: %s event: %s name: %s\n", dev->path, dev->event,
								dev->name);

	return 0;
}

/*
 * Show how to use sys_input_*() functions.
 * This actually requires a path to a device with registered input devices. My
 * notebook has a serial keyboard input in /sys/bus/serio/devices so I use it
 * here. If this is empty on your system, then please adjust the path.
 * You can find all devices with registered input devices in /sys/class/input.
 * However, some of the functions here require a base path to the device that
 * has the input device registered so often you need to use:
 *	/sys/class/input/<some_device>/device/device
 * as base path.
 */
static int example_input()
{
	int ret;
	const char *path = "/sys/bus/serio/devices/serio0";
	struct sfs_input_dev *list, *iter;

	printf("sfs_input_foreach(\"%s\"):\n", path);
	ret = sfs_input_foreach(path, example_input_foreach, NULL);
	printf("return: %d\n", ret);

	printf("sfs_input_list(\"%s\"):\n", path);
	ret = sfs_input_list(path, &list);
	if (!ret) {
		for (iter = list; iter; iter = iter->next)
			example_input_foreach(iter, NULL);
		sfs_input_unref(list);
	}
	printf("return: %d\n", ret);

	return 0;
}

/* call each example and abort if one example fails */
int main(int argc, char **argv)
{
	int ret;

	printf("sys_dir_*() examples:\n");
	ret = example_dir();
	if (ret) {
		printf("sys_dir_*() example failed\n");
		return -ret;
	}
	printf("\n");

	printf("sys_input_*() examples:\n");
	ret = example_input();
	if (ret) {
		printf("sys_input_*() example failed\n");
		return -ret;
	}
	printf("\n");

	printf("all examples successful\n");

	return EXIT_SUCCESS;
}
