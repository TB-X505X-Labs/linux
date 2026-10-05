/* SPDX-License-Identifier: GPL-2.0 */
/* x505: minimal ashmem ioctl defs for the memfd ashmem shim (from Android common kernel) */
#ifndef _MM_X505_ASHMEM_H
#define _MM_X505_ASHMEM_H
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/compat.h>
/* SPDX-License-Identifier: GPL-2.0 OR Apache-2.0 */
/*
 * Copyright 2008 Google Inc.
 * Author: Robert Love
 */



#define ASHMEM_NAME_LEN		256

#define ASHMEM_NAME_DEF		"dev/ashmem"

/* Return values from ASHMEM_PIN: Was the mapping purged while unpinned? */
#define ASHMEM_NOT_PURGED	0
#define ASHMEM_WAS_PURGED	1

/* Return values from ASHMEM_GET_PIN_STATUS: Is the mapping pinned? */
#define ASHMEM_IS_UNPINNED	0
#define ASHMEM_IS_PINNED	1

struct ashmem_pin {
	__u32 offset;	/* offset into region, in bytes, page-aligned */
	__u32 len;	/* length forward from offset, in bytes, page-aligned */
};

#define __ASHMEMIOC		0x77

enum {
	ASHMEM_SET_NAME		= _IOW(__ASHMEMIOC, 1, char[ASHMEM_NAME_LEN]),
	ASHMEM_GET_NAME		= _IOR(__ASHMEMIOC, 2, char[ASHMEM_NAME_LEN]),
	ASHMEM_SET_SIZE		= _IOW(__ASHMEMIOC, 3, size_t),
	ASHMEM_GET_SIZE		= _IO(__ASHMEMIOC, 4),
	ASHMEM_SET_PROT_MASK	= _IOW(__ASHMEMIOC, 5, unsigned long),
	ASHMEM_GET_PROT_MASK	= _IO(__ASHMEMIOC, 6),
	ASHMEM_PIN		= _IOW(__ASHMEMIOC, 7, struct ashmem_pin),
	ASHMEM_UNPIN		= _IOW(__ASHMEMIOC, 8, struct ashmem_pin),
	ASHMEM_GET_PIN_STATUS	= _IO(__ASHMEMIOC, 9),
	ASHMEM_PURGE_ALL_CACHES	= _IO(__ASHMEMIOC, 10),
	ASHMEM_GET_FILE_ID	= _IOR(__ASHMEMIOC, 11, unsigned long),
};





/* support of 32bit userspace on 64bit platforms */
#ifdef CONFIG_COMPAT
enum {
	COMPAT_ASHMEM_SET_SIZE		=	_IOW(__ASHMEMIOC, 3, compat_size_t),
	COMPAT_ASHMEM_SET_PROT_MASK	=	_IOW(__ASHMEMIOC, 5, unsigned int),
	COMPAT_ASHMEM_GET_FILE_ID	=	_IOR(__ASHMEMIOC, 11, compat_uptr_t),
};
#endif

long ashmem_memfd_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#endif
