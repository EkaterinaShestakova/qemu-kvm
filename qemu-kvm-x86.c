/*
 * qemu/kvm integration, x86 specific code
 *
 * Copyright (C) 2006-2008 Qumranet Technologies
 *
 * Licensed under the terms of the GNU GPL version 2 or higher.
 */

#include "config.h"
#include "config-host.h"

#include <string.h>
#include "hw/hw.h"
#include "gdbstub.h"
#include <sys/io.h>

#include "qemu-kvm.h"
#include <pthread.h>
#include <sys/utsname.h>
#include <linux/kvm_para.h>
#include <sys/ioctl.h>

#include "kvm.h"
#include "hw/apic.h"

int kvm_create_pit(KVMState *s)
{
    int r;

    if (kvm_irqchip_in_kernel()) {
        r = kvm_vm_ioctl(s, KVM_CREATE_PIT);
        if (r < 0) {
            fprintf(stderr, "Create kernel PIC irqchip failed\n");
            return r;
        }
        if (!kvm_pit_reinject) {
            r = kvm_reinject_control(s, 0);
            if (r < 0) {
                fprintf(stderr,
                        "failure to disable in-kernel PIT reinjection\n");
                return r;
            }
        }
    }
    return 0;
}

int kvm_handle_tpr_access(CPUState *env)
{
    struct kvm_run *run = env->kvm_run;
    kvm_tpr_access_report(env,
                          run->tpr_access.rip,
                          run->tpr_access.is_write);
    return 1;
}


int kvm_enable_vapic(CPUState *env, uint64_t vapic)
{
    struct kvm_vapic_addr va = {
        .vapic_addr = vapic,
    };

    return kvm_vcpu_ioctl(env, KVM_SET_VAPIC_ADDR, &va);
}

int kvm_get_pit(KVMState *s, struct kvm_pit_state *pit_state)
{
    if (!kvm_irqchip_in_kernel()) {
        return 0;
    }
    return kvm_vm_ioctl(s, KVM_GET_PIT, pit_state);
}

int kvm_set_pit(KVMState *s, struct kvm_pit_state *pit_state)
{
    if (!kvm_irqchip_in_kernel()) {
        return 0;
    }
    return kvm_vm_ioctl(s, KVM_SET_PIT, pit_state);
}

int kvm_get_pit2(KVMState *s, struct kvm_pit_state2 *ps2)
{
    if (!kvm_irqchip_in_kernel()) {
        return 0;
    }
    return kvm_vm_ioctl(s, KVM_GET_PIT2, ps2);
}

int kvm_set_pit2(KVMState *s, struct kvm_pit_state2 *ps2)
{
    if (!kvm_irqchip_in_kernel()) {
        return 0;
    }
    return kvm_vm_ioctl(s, KVM_SET_PIT2, ps2);
}

static int kvm_enable_tpr_access_reporting(CPUState *env)
{
    int r;
    struct kvm_tpr_access_ctl tac = { .enabled = 1 };

    r = kvm_ioctl(env->kvm_state, KVM_CHECK_EXTENSION, KVM_CAP_VAPIC);
    if (r <= 0) {
        return -ENOSYS;
    }
    return kvm_vcpu_ioctl(env, KVM_TPR_ACCESS_REPORTING, &tac);
}

static int _kvm_arch_init_vcpu(CPUState *env)
{
    kvm_arch_reset_vcpu(env);

    kvm_enable_tpr_access_reporting(env);

    return kvm_update_ioport_access(env);
}

#ifdef CONFIG_KVM_DEVICE_ASSIGNMENT
int kvm_arch_set_ioport_access(unsigned long start, unsigned long size,
                               bool enable)
{
    if (ioperm(start, size, enable) < 0) {
        return -errno;
    }
    return 0;
}
#endif