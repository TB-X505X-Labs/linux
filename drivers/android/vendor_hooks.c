// SPDX-License-Identifier: GPL-2.0-only
/* vendor_hook.c
 *
 * Android Vendor Hook Support
 *
 * Copyright 2020 Google LLC
 */

#define CREATE_TRACE_POINTS
#include <trace/hooks/vendor_hooks.h>
#include <linux/tracepoint.h>
#include <trace/hooks/mpam.h>
#include <trace/hooks/cpuidle.h>
#include <trace/hooks/debug.h>
#include <trace/hooks/iommu.h>
#include <trace/hooks/vmscan.h>
#include <trace/hooks/avc.h>
#include <trace/hooks/selinux.h>
#include <trace/hooks/syscall_check.h>
#include <trace/hooks/sysrqcrash.h>
#include <trace/hooks/fpsimd.h>

/*
#include <trace/hooks/pm_domain.h>
#include <trace/hooks/cpuidle_psci.h>
 * Export tracepoints that act as a bare tracehook (ie: have no trace event
 * associated with them) to allow external modules to probe them.
 */
EXPORT_TRACEPOINT_SYMBOL_GPL(android_vh_mpam_set);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_vh_ipi_stop);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_rvh_iommu_setup_dma_ops);
#include <trace/hooks/gic.h>
#include <trace/hooks/gic_v3.h>
EXPORT_TRACEPOINT_SYMBOL_GPL(android_rvh_set_balance_anon_file_reclaim);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_vh_check_mmap_file);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_vh_is_fpsimd_save);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_vh_iommu_iovad_alloc_iova);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_vh_iommu_iovad_free_iova);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_rvh_selinux_avc_insert);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_vh_cpu_idle_enter);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_vh_cpu_idle_exit);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_rvh_selinux_avc_node_delete);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_rvh_selinux_avc_node_replace);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_rvh_selinux_avc_lookup);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_rvh_selinux_is_initialized);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_vh_check_file_open);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_vh_allow_domain_state);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_vh_cpuidle_psci_enter);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_vh_cpuidle_psci_exit);
EXPORT_TRACEPOINT_SYMBOL_GPL(android_vh_gic_set_affinity);
