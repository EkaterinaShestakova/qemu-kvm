/*
 * qemu/kvm integration
 *
 * Copyright (C) 2006-2008 Qumranet Technologies
 *
 * Licensed under the terms of the GNU GPL version 2 or higher.
 */
#ifndef THE_ORIGINAL_AND_TRUE_QEMU_KVM_H
#define THE_ORIGINAL_AND_TRUE_QEMU_KVM_H

#include "cpu.h"

#include <signal.h>
#include <stdlib.h>

#ifdef CONFIG_KVM

#if defined(__s390__)
#include <asm/ptrace.h>
#endif

#include <stdint.h>

#ifndef __user
#define __user       /* temporary, until installed via make headers_install */
#endif

#include <linux/kvm.h>

#include <signal.h>

/* FIXME: share this number with kvm */
/* FIXME: or dynamically alloc/realloc regions */
#ifdef __s390__
#define KVM_MAX_NUM_MEM_REGIONS 1u
#define MAX_VCPUS 64
#define LIBKVM_S390_ORIGIN (0UL)
#elif defined(__ia64__)
#define KVM_MAX_NUM_MEM_REGIONS 32u
#define MAX_VCPUS 256
#else
#define KVM_MAX_NUM_MEM_REGIONS 32u
#define MAX_VCPUS 16
#endif

/**
 * \brief The KVM context
 *
 * The verbose KVM context
 */

struct kvm_context {
    /// ioctl to use to inject interrupts
    int irqchip_inject_ioctl;
#ifdef KVM_CAP_IRQ_ROUTING
    struct kvm_irq_routing *irq_routes;
    int nr_allocated_irq_routes;
#endif
    void *used_gsi_bitmap;
    int max_gsi;
};

typedef struct kvm_context *kvm_context_t;

#include "kvm.h"

int kvm_arch_run(CPUState *env);

int kvm_create_irqchip(kvm_context_t kvm);

/*!
 * \brief Check if a vcpu is ready for interrupt injection
 *
 * This checks if vcpu interrupts are not masked by mov ss or sti.
 *
 * \param kvm Pointer to the current kvm_context
 * \param vcpu Which virtual CPU should get dumped
 * \return boolean indicating interrupt injection readiness
 */
int kvm_is_ready_for_interrupt_injection(CPUState *env);

#if defined(__i386__) || defined(__x86_64__)
/*!
 * \brief Simulate an external vectored interrupt
 *
 * This allows you to simulate an external vectored interrupt.
 *
 * \param kvm Pointer to the current kvm_context
 * \param vcpu Which virtual CPU should get dumped
 * \param irq Vector number
 * \return 0 on success
 */
int kvm_inject_irq(CPUState *env, unsigned irq);
#endif

int kvm_set_irq_level(kvm_context_t kvm, int irq, int level, int *status);

#ifdef KVM_CAP_IRQCHIP
/*!
 * \brief Dump in kernel IRQCHIP contents
 *
 * Dump one of the in kernel irq chip devices, including PIC (master/slave)
 * and IOAPIC into a kvm_irqchip structure
 *
 * \param kvm Pointer to the current kvm_context
 * \param chip The irq chip device to be dumped
 */
int kvm_get_irqchip(kvm_context_t kvm, struct kvm_irqchip *chip);

/*!
 * \brief Set in kernel IRQCHIP contents
 *
 * Write one of the in kernel irq chip devices, including PIC (master/slave)
 * and IOAPIC
 *
 *
 * \param kvm Pointer to the current kvm_context
 * \param chip THe irq chip device to be written
 */
int kvm_set_irqchip(kvm_context_t kvm, struct kvm_irqchip *chip);

#if defined(__i386__) || defined(__x86_64__)
/*!
 * \brief Get in kernel local APIC for vcpu
 *
 * Save the local apic state including the timer of a virtual CPU
 *
 * \param kvm Pointer to the current kvm_context
 * \param vcpu Which virtual CPU should be accessed
 * \param s Local apic state of the specific virtual CPU
 */
int kvm_get_lapic(CPUState *env, struct kvm_lapic_state *s);

/*!
 * \brief Set in kernel local APIC for vcpu
 *
 * Restore the local apic state including the timer of a virtual CPU
 *
 * \param kvm Pointer to the current kvm_context
 * \param vcpu Which virtual CPU should be accessed
 * \param s Local apic state of the specific virtual CPU
 */
int kvm_set_lapic(CPUState *env, struct kvm_lapic_state *s);

#endif

/*!
 * \brief Simulate an NMI
 *
 * This allows you to simulate a non-maskable interrupt.
 *
 * \param kvm Pointer to the current kvm_context
 * \param vcpu Which virtual CPU should get dumped
 * \return 0 on success
 */
int kvm_inject_nmi(CPUState *env);

#endif

#ifdef KVM_CAP_PIT

#if defined(__i386__) || defined(__x86_64__)
/*!
 * \brief Get in kernel PIT of the virtual domain
 *
 * Save the PIT state.
 *
 * \param kvm Pointer to the current kvm_context
 * \param s PIT state of the virtual domain
 */
int kvm_get_pit(kvm_context_t kvm, struct kvm_pit_state *s);

/*!
 * \brief Set in kernel PIT of the virtual domain
 *
 * Restore the PIT state.
 * Timer would be retriggerred after restored.
 *
 * \param kvm Pointer to the current kvm_context
 * \param s PIT state of the virtual domain
 */
int kvm_set_pit(kvm_context_t kvm, struct kvm_pit_state *s);

int kvm_reinject_control(kvm_context_t kvm, int pit_reinject);

#ifdef KVM_CAP_PIT_STATE2
/*!
 * \brief Check for kvm support of kvm_pit_state2
 *
 * \param kvm Pointer to the current kvm_context
 * \return 0 on success
 */
int kvm_has_pit_state2(kvm_context_t kvm);

/*!
 * \brief Set in kernel PIT state2 of the virtual domain
 *
 *
 * \param kvm Pointer to the current kvm_context
 * \param ps2 PIT state2 of the virtual domain
 * \return 0 on success
 */
int kvm_set_pit2(kvm_context_t kvm, struct kvm_pit_state2 *ps2);

/*!
 * \brief Get in kernel PIT state2 of the virtual domain
 *
 *
 * \param kvm Pointer to the current kvm_context
 * \param ps2 PIT state2 of the virtual domain
 * \return 0 on success
 */
int kvm_get_pit2(kvm_context_t kvm, struct kvm_pit_state2 *ps2);

#endif
#endif
#endif

#ifdef KVM_CAP_VAPIC

int kvm_enable_vapic(CPUState *env, uint64_t vapic);

#endif

#ifdef KVM_CAP_DEVICE_ASSIGNMENT
/*!
 * \brief Notifies host kernel about a PCI device to be assigned to a guest
 *
 * Used for PCI device assignment, this function notifies the host
 * kernel about the assigning of the physical PCI device to a guest.
 *
 * \param kvm Pointer to the current kvm_context
 * \param assigned_dev Parameters, like bus, devfn number, etc
 */
int kvm_assign_pci_device(kvm_context_t kvm,
                          struct kvm_assigned_pci_dev *assigned_dev);

/*!
 * \brief Assign IRQ for an assigned device
 *
 * Used for PCI device assignment, this function assigns IRQ numbers for
 * an physical device and guest IRQ handling.
 *
 * \param kvm Pointer to the current kvm_context
 * \param assigned_irq Parameters, like dev id, host irq, guest irq, etc
 */
int kvm_assign_irq(kvm_context_t kvm, struct kvm_assigned_irq *assigned_irq);

#ifdef KVM_CAP_ASSIGN_DEV_IRQ
/*!
 * \brief Deassign IRQ for an assigned device
 *
 * Used for PCI device assignment, this function deassigns IRQ numbers
 * for an assigned device.
 *
 * \param kvm Pointer to the current kvm_context
 * \param assigned_irq Parameters, like dev id, host irq, guest irq, etc
 */
int kvm_deassign_irq(kvm_context_t kvm, struct kvm_assigned_irq *assigned_irq);
#endif
#endif

#ifdef KVM_CAP_DEVICE_DEASSIGNMENT
/*!
 * \brief Notifies host kernel about a PCI device to be deassigned from a guest
 *
 * Used for hot remove PCI device, this function notifies the host
 * kernel about the deassigning of the physical PCI device from a guest.
 *
 * \param kvm Pointer to the current kvm_context
 * \param assigned_dev Parameters, like bus, devfn number, etc
 */
int kvm_deassign_pci_device(kvm_context_t kvm,
                            struct kvm_assigned_pci_dev *assigned_dev);
#endif

/*!
 * \brief Clears the temporary irq routing table
 *
 * Clears the temporary irq routing table.  Nothing is committed to the
 * running VM.
 *
 */
int kvm_clear_gsi_routes(void);

/*!
 * \brief Adds an irq route to the temporary irq routing table
 *
 * Adds an irq route to the temporary irq routing table.  Nothing is
 * committed to the running VM.
 */
int kvm_add_irq_route(int gsi, int irqchip, int pin);

/*!
 * \brief Removes an irq route from the temporary irq routing table
 *
 * Adds an irq route to the temporary irq routing table.  Nothing is
 * committed to the running VM.
 */
int kvm_del_irq_route(int gsi, int irqchip, int pin);

struct kvm_irq_routing_entry;
/*!
 * \brief Adds a routing entry to the temporary irq routing table
 *
 * Adds a filled routing entry to the temporary irq routing table. Nothing is
 * committed to the running VM.
 */
int kvm_add_routing_entry(struct kvm_irq_routing_entry *entry);

/*!
 * \brief Removes a routing from the temporary irq routing table
 *
 * Remove a routing to the temporary irq routing table.  Nothing is
 * committed to the running VM.
 */
int kvm_del_routing_entry(struct kvm_irq_routing_entry *entry);

/*!
 * \brief Updates a routing in the temporary irq routing table
 *
 * Update a routing in the temporary irq routing table
 * with a new value. entry type and GSI can not be changed.
 * Nothing is committed to the running VM.
 */
int kvm_update_routing_entry(struct kvm_irq_routing_entry *entry,
                             struct kvm_irq_routing_entry *newentry);


#ifdef KVM_CAP_DEVICE_MSIX
int kvm_assign_set_msix_nr(kvm_context_t kvm,
                           struct kvm_assigned_msix_nr *msix_nr);
int kvm_assign_set_msix_entry(kvm_context_t kvm,
                              struct kvm_assigned_msix_entry *entry);
#endif

#else                           /* !CONFIG_KVM */

typedef struct kvm_context *kvm_context_t;

struct kvm_pit_state {
};

#endif                          /* !CONFIG_KVM */


/*!
 * \brief Create new KVM context
 *
 * This creates a new kvm_context. A KVM context is a small area of data that
 * holds information about the KVM instance that gets created by this call.\n
 * This should always be your first call to KVM.
 *
 * \param opaque Not used
 * \return NULL on failure
 */
int kvm_init(void);

int kvm_main_loop(void);
int kvm_init_ap(void);
void kvm_save_lapic(CPUState *env);
void kvm_load_lapic(CPUState *env);

void kvm_hpet_enable_kpit(void);
void kvm_hpet_disable_kpit(void);

void on_vcpu(CPUState *env, void (*func)(void *data), void *data);
void kvm_update_interrupt_request(CPUState *env);

int kvm_arch_has_work(CPUState *env);
void kvm_arch_process_irqchip_events(CPUState *env);
int kvm_arch_try_push_interrupts(void *opaque);
void kvm_arch_push_nmi(void);
int kvm_set_boot_cpu_id(uint32_t id);

void kvm_tpr_access_report(CPUState *env, uint64_t rip, int is_write);

int kvm_arch_init_irq_routing(void);

#ifdef CONFIG_KVM_DEVICE_ASSIGNMENT
struct ioperm_data;

void kvm_ioperm(CPUState *env, void *data);
void kvm_add_ioperm_data(struct ioperm_data *data);
void kvm_remove_ioperm_data(unsigned long start_port, unsigned long num);
void kvm_arch_do_ioperm(void *_data);
#endif

#ifdef CONFIG_KVM
#include "qemu-queue.h"

extern int kvm_irqchip;
extern int kvm_pit;
extern int kvm_pit_reinject;
extern int kvm_nested;
extern kvm_context_t kvm_context;
extern unsigned int kvm_shadow_memory;

struct ioperm_data {
    unsigned long start_port;
    unsigned long num;
    int turn_on;
    QLIST_ENTRY(ioperm_data) entries;
};

int kvm_arch_halt(CPUState *env);
int handle_tpr_access(void *opaque, CPUState *env, uint64_t rip,
                      int is_write);

#ifdef TARGET_I386
#define qemu_kvm_has_pit_state2() kvm_has_pit_state2(kvm_context)
#endif
#else
#define kvm_nested 0
#ifdef TARGET_I386
#define qemu_kvm_has_pit_state2() (0)
#endif
#endif

#ifdef CONFIG_KVM

typedef struct KVMSlot {
    target_phys_addr_t start_addr;
    ram_addr_t memory_size;
    ram_addr_t phys_offset;
    int slot;
    int flags;
} KVMSlot;

typedef struct kvm_dirty_log KVMDirtyLog;

struct KVMState {
    KVMSlot slots[32];
    int fd;
    int vmfd;
    int coalesced_mmio;
#ifdef KVM_CAP_COALESCED_MMIO
    struct kvm_coalesced_mmio_ring *coalesced_mmio_ring;
#endif
    int broken_set_mem_region;
    int migration_log;
    int vcpu_events;
    int robust_singlestep;
    int debugregs;
#ifdef KVM_CAP_SET_GUEST_DEBUG
    QTAILQ_HEAD(, kvm_sw_breakpoint) kvm_sw_breakpoints;
#endif
    int irqchip_in_kernel;
    int pit_in_kernel;
    int xsave, xcrs;
    int many_ioeventfds;

    struct kvm_context kvm_context;
};

int kvm_tpr_enable_vapic(CPUState *env);

unsigned long kvm_get_thread_id(void);

#endif

#endif