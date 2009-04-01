#ifndef _FS_CEPH_TYPES_H
#define _FS_CEPH_TYPES_H

/* needed before including ceph_fs.h */
#include <linux/in.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/string.h>

#include "ceph_fs.h"

/*
 * Identify inodes by both their ino and snapshot id (a u64).
 */
struct ceph_vino {
	u64 ino;
	u64 snap;
};


/* context for the caps reservation mechanism */
struct ceph_caps_reservation {
	int count;
};


#endif
