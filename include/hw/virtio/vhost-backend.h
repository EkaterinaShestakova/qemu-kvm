/*
 * vhost-backend
 *
 * Copyright (c) 2013 Virtual Open Systems Sarl.
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 *
 */

#ifndef VHOST_BACKEND_H_
#define VHOST_BACKEND_H_

#include <stdbool.h>

typedef enum VhostBackendType {
    VHOST_BACKEND_TYPE_NONE = 0,
    VHOST_BACKEND_TYPE_KERNEL = 1,
    VHOST_BACKEND_TYPE_USER = 2,
    VHOST_BACKEND_TYPE_MAX = 3,
} VhostBackendType;

struct vhost_dev;
struct vhost_log;

typedef int (*vhost_call)(struct vhost_dev *dev, unsigned long int request,
             void *arg);
typedef int (*vhost_backend_init)(struct vhost_dev *dev, void *opaque);
typedef int (*vhost_backend_cleanup)(struct vhost_dev *dev);
typedef int (*vhost_backend_get_vq_index)(struct vhost_dev *dev, int idx);
typedef int (*vhost_backend_set_vring_enable)(struct vhost_dev *dev, int enable);

typedef int (*vhost_set_log_base_op)(struct vhost_dev *dev, uint64_t base,
                                     struct vhost_log *log);
typedef bool (*vhost_requires_shm_log_op)(struct vhost_dev *dev);
typedef int (*vhost_migration_done_op)(struct vhost_dev *dev,
                                       char *mac_addr);
typedef bool (*vhost_backend_can_merge_op)(struct vhost_dev *dev,
                                           uint64_t start1, uint64_t size1,
                                           uint64_t start2, uint64_t size2);

typedef struct VhostOps {
    VhostBackendType backend_type;
    vhost_call vhost_call;
    vhost_backend_init vhost_backend_init;
    vhost_backend_cleanup vhost_backend_cleanup;
    vhost_backend_get_vq_index vhost_backend_get_vq_index;
    vhost_backend_set_vring_enable vhost_backend_set_vring_enable;
    vhost_set_log_base_op vhost_set_log_base;
    vhost_requires_shm_log_op vhost_requires_shm_log;
    vhost_migration_done_op vhost_migration_done;
    vhost_backend_can_merge_op vhost_backend_can_merge;
} VhostOps;

extern const VhostOps user_ops;

int vhost_set_backend_type(struct vhost_dev *dev,
                           VhostBackendType backend_type);

#endif /* VHOST_BACKEND_H_ */
