cmd_/home/kwest/release.GPL/dir600b_v2.03/progs.priv/turbonat.26/sw_tcpip.o := mipsel-linux-gcc -Wp,-MD,/home/kwest/release.GPL/dir600b_v2.03/progs.priv/turbonat.26/.sw_tcpip.o.d  -nostdinc -isystem /spaces/opt/mipsel_gcc-3.3.5_uclibc-0.9.28/bin/../lib/gcc-lib/mipsel-linux-uclibc/3.3.5/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h  -Iinclude/asm/mach-generic/ -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Os  -mabi=32 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -ffreestanding  -march=mips32 -Wa,-mips32 -Wa,--trap -Iinclude/asm-mips/rt2880 -Iinclude/asm-mips/mach-generic -fomit-frame-pointer -gdwarf-2    -Os -Wall -D__UCLIBC_HAS_MMU__ -D__UCLIBC__ -DLOGNUM=1   -DMODULE -mlong-calls -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(sw_tcpip)"  -D"KBUILD_MODNAME=KBUILD_STR(sw_tcpip)" -c -o /home/kwest/release.GPL/dir600b_v2.03/progs.priv/turbonat.26/sw_tcpip.o /home/kwest/release.GPL/dir600b_v2.03/progs.priv/turbonat.26/sw_tcpip.c

deps_/home/kwest/release.GPL/dir600b_v2.03/progs.priv/turbonat.26/sw_tcpip.o := \
  /home/kwest/release.GPL/dir600b_v2.03/progs.priv/turbonat.26/sw_tcpip.c \
    $(wildcard include/config/qos/turbonat.h) \
    $(wildcard include/config/skip/turbonat.h) \
  include/linux/hw_tcpip.h \
    $(wildcard include/config/software/turbo.h) \
  include/linux/module.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/module/unload.h) \
    $(wildcard include/config/kallsyms.h) \
    $(wildcard include/config/sysfs.h) \
  include/linux/spinlock.h \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
  include/linux/thread_info.h \
  include/linux/bitops.h \
  include/asm/types.h \
    $(wildcard include/config/highmem.h) \
    $(wildcard include/config/64bit/phys/addr.h) \
    $(wildcard include/config/64bit.h) \
  include/asm/bitops.h \
    $(wildcard include/config/cpu/mipsr2.h) \
    $(wildcard include/config/cpu/mips32.h) \
    $(wildcard include/config/cpu/mips64.h) \
  include/linux/compiler.h \
    $(wildcard include/config/enable/must/check.h) \
  include/linux/compiler-gcc3.h \
  include/linux/compiler-gcc.h \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
    $(wildcard include/config/x86.h) \
  include/asm/irqflags.h \
    $(wildcard include/config/mips/mt/smtc.h) \
    $(wildcard include/config/irq/cpu.h) \
    $(wildcard include/config/mips/mt/smtc/instant/replay.h) \
  include/asm/hazards.h \
    $(wildcard include/config/cpu/r10000.h) \
    $(wildcard include/config/cpu/rm9000.h) \
    $(wildcard include/config/cpu/sb1.h) \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbd.h) \
    $(wildcard include/config/lsf.h) \
    $(wildcard include/config/resources/64bit.h) \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  include/asm/posix_types.h \
  include/asm/sgidefs.h \
  include/asm/barrier.h \
    $(wildcard include/config/cpu/has/sync.h) \
    $(wildcard include/config/cpu/has/wb.h) \
    $(wildcard include/config/weak/ordering.h) \
  include/asm/bug.h \
    $(wildcard include/config/bug.h) \
  include/asm/break.h \
  include/asm-generic/bug.h \
    $(wildcard include/config/debug/bugverbose.h) \
  include/asm/byteorder.h \
    $(wildcard include/config/cpu/mips64/r2.h) \
  include/linux/byteorder/little_endian.h \
  include/linux/byteorder/swab.h \
  include/linux/byteorder/generic.h \
  include/asm/cpu-features.h \
    $(wildcard include/config/32bit.h) \
    $(wildcard include/config/cpu/mipsr2/irq/vi.h) \
    $(wildcard include/config/cpu/mipsr2/irq/ei.h) \
  include/asm/cpu.h \
  include/asm/cpu-info.h \
    $(wildcard include/config/sgi/ip27.h) \
    $(wildcard include/config/mips/mt.h) \
  include/asm/cache.h \
    $(wildcard include/config/mips/l1/cache/shift.h) \
  include/asm/mach-generic/kmalloc.h \
    $(wildcard include/config/dma/coherent.h) \
  include/asm/mach-generic/cpu-feature-overrides.h \
  include/asm/war.h \
    $(wildcard include/config/sgi/ip22.h) \
    $(wildcard include/config/sni/rm.h) \
    $(wildcard include/config/cpu/r5432.h) \
    $(wildcard include/config/sb1/pass/1/workarounds.h) \
    $(wildcard include/config/sb1/pass/2/workarounds.h) \
    $(wildcard include/config/mips/malta.h) \
    $(wildcard include/config/mips/atlas.h) \
    $(wildcard include/config/mips/sead.h) \
    $(wildcard include/config/cpu/tx49xx.h) \
    $(wildcard include/config/momenco/jaguar/atx.h) \
    $(wildcard include/config/pmc/yosemite.h) \
    $(wildcard include/config/basler/excite.h) \
    $(wildcard include/config/momenco/ocelot/3.h) \
  include/asm-generic/bitops/non-atomic.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/ffz.h \
  include/asm-generic/bitops/find.h \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/ext2-non-atomic.h \
  include/asm-generic/bitops/le.h \
  include/asm-generic/bitops/ext2-atomic.h \
  include/asm-generic/bitops/minix.h \
  include/asm/thread_info.h \
    $(wildcard include/config/page/size/4kb.h) \
    $(wildcard include/config/page/size/8kb.h) \
    $(wildcard include/config/page/size/16kb.h) \
    $(wildcard include/config/page/size/64kb.h) \
    $(wildcard include/config/debug/stack/usage.h) \
  include/asm/processor.h \
    $(wildcard include/config/mips/mt/fpaff.h) \
    $(wildcard include/config/cpu/has/prefetch.h) \
  include/linux/cpumask.h \
    $(wildcard include/config/hotplug/cpu.h) \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/numa.h) \
  /spaces/opt/mipsel_gcc-3.3.5_uclibc-0.9.28/lib/gcc-lib/mipsel-linux-uclibc/3.3.5/include/stdarg.h \
  include/linux/linkage.h \
  include/asm/linkage.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/linux/bitmap.h \
  include/linux/string.h \
  include/asm/string.h \
    $(wildcard include/config/cpu/r3000.h) \
  include/asm/cachectl.h \
  include/asm/mipsregs.h \
    $(wildcard include/config/cpu/vr41xx.h) \
  include/asm/prefetch.h \
  include/asm/system.h \
  include/asm/addrspace.h \
    $(wildcard include/config/cpu/r4300.h) \
    $(wildcard include/config/cpu/r4x00.h) \
    $(wildcard include/config/cpu/r5000.h) \
    $(wildcard include/config/cpu/rm7000.h) \
    $(wildcard include/config/cpu/nevada.h) \
    $(wildcard include/config/cpu/r8000.h) \
    $(wildcard include/config/cpu/sb1a.h) \
  include/asm/mach-generic/spaces.h \
    $(wildcard include/config/dma/noncoherent.h) \
  include/asm/dsp.h \
  include/linux/stringify.h \
  include/linux/bottom_half.h \
  include/linux/spinlock_types.h \
  include/linux/lockdep.h \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/generic/hardirqs.h) \
    $(wildcard include/config/prove/locking.h) \
  include/linux/spinlock_types_up.h \
  include/linux/spinlock_up.h \
  include/linux/spinlock_api_up.h \
  include/asm/atomic.h \
  include/asm-generic/atomic.h \
  include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  include/linux/poison.h \
  include/linux/prefetch.h \
  include/linux/stat.h \
  include/asm/stat.h \
  include/linux/time.h \
  include/linux/seqlock.h \
  include/linux/cache.h \
  include/linux/kmod.h \
    $(wildcard include/config/kmod.h) \
  include/linux/errno.h \
  include/asm/errno.h \
  include/asm-generic/errno-base.h \
  include/linux/elf.h \
  include/linux/auxvec.h \
  include/asm/auxvec.h \
  include/linux/elf-em.h \
  include/asm/elf.h \
    $(wildcard include/config/mips32/n32.h) \
    $(wildcard include/config/mips32/o32.h) \
    $(wildcard include/config/mips32/compat.h) \
  include/linux/kobject.h \
    $(wildcard include/config/hotplug.h) \
  include/linux/sysfs.h \
  include/linux/rwsem.h \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  include/linux/rwsem-spinlock.h \
  include/linux/kref.h \
  include/linux/wait.h \
  include/asm/current.h \
  include/linux/moduleparam.h \
  include/linux/init.h \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/acpi/hotplug/memory.h) \
  include/asm/local.h \
  include/linux/percpu.h \
  include/linux/slab.h \
    $(wildcard include/config/slab/debug.h) \
    $(wildcard include/config/slab.h) \
    $(wildcard include/config/debug/slab.h) \
  include/linux/gfp.h \
    $(wildcard include/config/zone/dma.h) \
    $(wildcard include/config/zone/dma32.h) \
  include/linux/mmzone.h \
    $(wildcard include/config/force/max/zoneorder.h) \
    $(wildcard include/config/arch/populates/node/map.h) \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/have/memory/present.h) \
    $(wildcard include/config/need/node/memmap/size.h) \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/sparsemem.h) \
    $(wildcard include/config/have/arch/early/pfn/to/nid.h) \
    $(wildcard include/config/flatmem.h) \
    $(wildcard include/config/sparsemem/extreme.h) \
    $(wildcard include/config/nodes/span/other/nodes.h) \
  include/linux/numa.h \
    $(wildcard include/config/nodes/shift.h) \
  include/linux/nodemask.h \
  include/asm/page.h \
    $(wildcard include/config/build/elf64.h) \
    $(wildcard include/config/limited/dma.h) \
  include/linux/pfn.h \
  include/asm/io.h \
  include/asm-generic/iomap.h \
  include/asm/pgtable-bits.h \
    $(wildcard include/config/cpu/mips32/r1.h) \
    $(wildcard include/config/cpu/tx39xx.h) \
    $(wildcard include/config/mips/uncached.h) \
  include/asm/mach-generic/ioremap.h \
  include/asm/mach-generic/mangle-port.h \
    $(wildcard include/config/swap/io/space.h) \
  include/asm-generic/memory_model.h \
    $(wildcard include/config/out/of/line/pfn/to/page.h) \
  include/asm-generic/page.h \
  include/linux/memory_hotplug.h \
    $(wildcard include/config/have/arch/nodedata/extension.h) \
  include/linux/notifier.h \
  include/linux/mutex.h \
    $(wildcard include/config/debug/mutexes.h) \
  include/linux/srcu.h \
  include/linux/topology.h \
    $(wildcard include/config/sched/smt.h) \
    $(wildcard include/config/sched/mc.h) \
  include/linux/smp.h \
  include/asm/topology.h \
  include/asm/mach-generic/topology.h \
  include/asm-generic/topology.h \
  include/linux/slab_def.h \
  include/linux/kmalloc_sizes.h \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/large/allocs.h) \
  include/asm/percpu.h \
  include/asm-generic/percpu.h \
  include/asm/module.h \
    $(wildcard include/config/cpu/mips32/r2.h) \
    $(wildcard include/config/cpu/mips64/r1.h) \
    $(wildcard include/config/cpu/r6000.h) \
  include/asm/uaccess.h \
  include/asm-generic/uaccess.h \
  include/linux/sched.h \
    $(wildcard include/config/detect/softlockup.h) \
    $(wildcard include/config/split/ptlock/cpus.h) \
    $(wildcard include/config/keys.h) \
    $(wildcard include/config/bsd/process/acct.h) \
    $(wildcard include/config/taskstats.h) \
    $(wildcard include/config/inotify/user.h) \
    $(wildcard include/config/schedstats.h) \
    $(wildcard include/config/task/delay/acct.h) \
    $(wildcard include/config/blk/dev/io/trace.h) \
    $(wildcard include/config/cc/stackprotector.h) \
    $(wildcard include/config/sysvipc.h) \
    $(wildcard include/config/rt/mutexes.h) \
    $(wildcard include/config/task/xacct.h) \
    $(wildcard include/config/cpusets.h) \
    $(wildcard include/config/compat.h) \
    $(wildcard include/config/fault/injection.h) \
  include/asm/param.h \
    $(wildcard include/config/hz.h) \
  include/linux/capability.h \
  include/linux/timex.h \
    $(wildcard include/config/time/interpolation.h) \
    $(wildcard include/config/no/hz.h) \
  include/asm/timex.h \
  include/asm/mach-generic/timex.h \
  include/linux/jiffies.h \
  include/linux/calc64.h \
  include/asm/div64.h \
  include/asm/compiler.h \
  include/linux/rbtree.h \
  include/asm/semaphore.h \
  include/asm/ptrace.h \
    $(wildcard include/config/cpu/has/smartmips.h) \
  include/asm/isadep.h \
  include/asm/mmu.h \
  include/asm/cputime.h \
  include/asm-generic/cputime.h \
  include/linux/sem.h \
  include/linux/ipc.h \
    $(wildcard include/config/ipc/ns.h) \
  include/asm/ipcbuf.h \
  include/asm/sembuf.h \
  include/linux/signal.h \
  include/asm/signal.h \
    $(wildcard include/config/trad/signals.h) \
    $(wildcard include/config/binfmt/irix.h) \
  include/asm-generic/signal.h \
  include/asm/sigcontext.h \
  include/asm/siginfo.h \
  include/asm-generic/siginfo.h \
  include/linux/securebits.h \
  include/linux/fs_struct.h \
  include/linux/completion.h \
  include/linux/pid.h \
  include/linux/rcupdate.h \
  include/linux/seccomp.h \
    $(wildcard include/config/seccomp.h) \
  include/linux/futex.h \
    $(wildcard include/config/futex.h) \
  include/linux/rtmutex.h \
    $(wildcard include/config/debug/rt/mutexes.h) \
  include/linux/plist.h \
    $(wildcard include/config/debug/pi/list.h) \
  include/linux/param.h \
  include/linux/resource.h \
  include/asm/resource.h \
  include/asm-generic/resource.h \
  include/linux/timer.h \
    $(wildcard include/config/timer/stats.h) \
  include/linux/ktime.h \
    $(wildcard include/config/ktime/scalar.h) \
  include/linux/hrtimer.h \
    $(wildcard include/config/high/res/timers.h) \
  include/linux/task_io_accounting.h \
    $(wildcard include/config/task/io/accounting.h) \
  include/linux/aio.h \
  include/linux/workqueue.h \
  include/linux/aio_abi.h \
  include/linux/uio.h \
  include/linux/sysdev.h \
  include/linux/pm.h \
    $(wildcard include/config/software/suspend.h) \
    $(wildcard include/config/pm.h) \
  include/linux/mm.h \
    $(wildcard include/config/sysctl.h) \
    $(wildcard include/config/stack/growsup.h) \
    $(wildcard include/config/debug/vm.h) \
    $(wildcard include/config/shmem.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/proc/fs.h) \
    $(wildcard include/config/debug/pagealloc.h) \
  include/linux/prio_tree.h \
  include/linux/fs.h \
    $(wildcard include/config/dnotify.h) \
    $(wildcard include/config/quota.h) \
    $(wildcard include/config/inotify.h) \
    $(wildcard include/config/security.h) \
    $(wildcard include/config/epoll.h) \
    $(wildcard include/config/auditsyscall.h) \
    $(wildcard include/config/block.h) \
    $(wildcard include/config/fs/xip.h) \
    $(wildcard include/config/migration.h) \
  include/linux/limits.h \
  include/linux/ioctl.h \
  include/asm/ioctl.h \
  include/linux/kdev_t.h \
  include/linux/dcache.h \
    $(wildcard include/config/profiling.h) \
  include/linux/namei.h \
  include/linux/radix-tree.h \
  include/linux/quota.h \
  include/linux/dqblk_xfs.h \
  include/linux/dqblk_v1.h \
  include/linux/dqblk_v2.h \
  include/linux/nfs_fs_i.h \
  include/linux/nfs.h \
  include/linux/sunrpc/msg_prot.h \
  include/linux/fcntl.h \
  include/asm/fcntl.h \
  include/asm-generic/fcntl.h \
  include/linux/err.h \
  include/linux/debug_locks.h \
    $(wildcard include/config/debug/locking/api/selftests.h) \
  include/linux/backing-dev.h \
  include/linux/mm_types.h \
  include/asm/pgtable.h \
  include/asm/pgtable-32.h \
  include/asm/fixmap.h \
  include/asm-generic/pgtable-nopmd.h \
  include/asm-generic/pgtable-nopud.h \
  include/asm-generic/pgtable.h \
  include/linux/page-flags.h \
    $(wildcard include/config/s390.h) \
    $(wildcard include/config/swap.h) \
  include/linux/vmstat.h \
    $(wildcard include/config/vm/event/counters.h) \
  include/linux/socket.h \
  include/asm/socket.h \
  include/asm/sockios.h \
  include/linux/sockios.h \
  include/linux/in.h \
  include/linux/inet.h \
  include/linux/netdevice.h \
    $(wildcard include/config/ax25.h) \
    $(wildcard include/config/tr.h) \
    $(wildcard include/config/net/ipip.h) \
    $(wildcard include/config/net/ipgre.h) \
    $(wildcard include/config/ipv6/sit.h) \
    $(wildcard include/config/ipv6/tunnel.h) \
    $(wildcard include/config/netpoll.h) \
    $(wildcard include/config/net/poll/controller.h) \
    $(wildcard include/config/netpoll/trap.h) \
    $(wildcard include/config/net/dma.h) \
  include/linux/if.h \
  include/linux/hdlc/ioctl.h \
  include/linux/if_ether.h \
  include/linux/skbuff.h \
    $(wildcard include/config/netfilter.h) \
    $(wildcard include/config/bridge/netfilter.h) \
    $(wildcard include/config/vlan/8021q.h) \
    $(wildcard include/config/nf/conntrack.h) \
    $(wildcard include/config/net/sched.h) \
    $(wildcard include/config/net/cls/act.h) \
    $(wildcard include/config/network/secmark.h) \
  include/linux/net.h \
  include/linux/random.h \
  include/linux/sysctl.h \
    $(wildcard include/config/ip/nf/conntrack/clean.h) \
  include/linux/textsearch.h \
  include/net/checksum.h \
  include/asm/checksum.h \
  include/linux/in6.h \
  include/linux/dmaengine.h \
    $(wildcard include/config/dma/engine.h) \
  include/linux/if_packet.h \
  include/linux/device.h \
    $(wildcard include/config/debug/devres.h) \
  include/linux/ioport.h \
  include/linux/klist.h \
  include/asm/device.h \
  include/asm-generic/device.h \
  include/linux/interrupt.h \
    $(wildcard include/config/generic/irq/probe.h) \
  include/linux/irqreturn.h \
  include/linux/hardirq.h \
    $(wildcard include/config/preempt/bkl.h) \
    $(wildcard include/config/virt/cpu/accounting.h) \
  include/linux/smp_lock.h \
    $(wildcard include/config/lock/kernel.h) \
  include/asm/hardirq.h \
  include/linux/irq.h \
    $(wildcard include/config/irq/per/cpu.h) \
    $(wildcard include/config/irq/release/method.h) \
    $(wildcard include/config/generic/pending/irq.h) \
    $(wildcard include/config/irqbalance.h) \
    $(wildcard include/config/auto/irq/affinity.h) \
    $(wildcard include/config/generic/hardirqs/no//do/irq.h) \
  include/asm/irq.h \
    $(wildcard include/config/i8259.h) \
  include/asm/mipsmtregs.h \
  include/asm/mach-generic/irq.h \
    $(wildcard include/config/irq/cpu/rm7k.h) \
    $(wildcard include/config/irq/cpu/rm9k.h) \
  include/asm/irq_regs.h \
  include/asm/hw_irq.h \
  include/linux/profile.h \
  include/linux/irq_cpustat.h \
  include/linux/if_arp.h \
  include/linux/proc_fs.h \
    $(wildcard include/config/proc/devicetree.h) \
    $(wildcard include/config/proc/kcore.h) \
  include/linux/magic.h \
  include/linux/netlink.h \
  include/linux/ip.h \
  include/net/protocol.h \
    $(wildcard include/config/ipv6.h) \
  include/net/route.h \
    $(wildcard include/config/hardware/turbo.h) \
  include/net/dst.h \
    $(wildcard include/config/net/cls/route.h) \
    $(wildcard include/config/xfrm.h) \
  include/linux/rtnetlink.h \
  include/linux/if_link.h \
  include/linux/if_addr.h \
  include/linux/neighbour.h \
  include/net/neighbour.h \
  include/linux/seq_file.h \
  include/net/inetpeer.h \
  include/net/flow.h \
    $(wildcard include/config/ipv6/mip6.h) \
  include/linux/in_route.h \
  include/linux/route.h \
  include/linux/security.h \
    $(wildcard include/config/security/network.h) \
    $(wildcard include/config/security/network/xfrm.h) \
  include/linux/binfmts.h \
  include/linux/shm.h \
  include/asm/shmparam.h \
  include/asm/shmbuf.h \
  include/linux/msg.h \
  include/asm/msgbuf.h \
    $(wildcard include/config/cpu/little/endian.h) \
  include/linux/key.h \
  include/linux/xfrm.h \
  include/linux/tcp.h \
    $(wildcard include/config/tcp/md5sig.h) \
  include/net/sock.h \
    $(wildcard include/config/netdebug.h) \
    $(wildcard include/config/net.h) \
  include/linux/filter.h \
  include/net/inet_connection_sock.h \
  include/linux/poll.h \
  include/asm/poll.h \
  include/net/inet_sock.h \
  include/net/request_sock.h \
  include/net/inet_timewait_sock.h \
  include/net/tcp_states.h \
  include/net/timewait_sock.h \
  include/linux/udp.h \
  include/net/ip_fib.h \
    $(wildcard include/config/ip/route/multipath.h) \
    $(wildcard include/config/ip/route/multipath/cached.h) \
    $(wildcard include/config/ip/multiple/tables.h) \
    $(wildcard include/config/ip/route/multipath/wrandom.h) \
  include/net/fib_rules.h \
  include/linux/fib_rules.h \
  include/net/netlink.h \
  include/linux/netfilter_ipv4/ip_conntrack.h \
    $(wildcard include/config/ip/nf/nat/needed.h) \
    $(wildcard include/config/netfilter/debug.h) \
    $(wildcard include/config/ip/nf/pers.h) \
    $(wildcard include/config/ip/nf/ct/acct.h) \
    $(wildcard include/config/ip/nf/target/masquerade.h) \
    $(wildcard include/config/ip/nf/conntrack/mark.h) \
    $(wildcard include/config/ip/nf/conntrack/secmark.h) \
    $(wildcard include/config/ip/nf/conntrack/events.h) \
  include/linux/netfilter/nf_conntrack_common.h \
    $(wildcard include/config/ip/nf/target/stun.h) \
  include/linux/netfilter_ipv4/ip_conntrack_tuple.h \
    $(wildcard include/config/ip/nf/qos.h) \
  include/linux/netfilter/nf_conntrack_tuple_common.h \
  include/linux/netfilter_ipv4/ip_conntrack_tcp.h \
  include/linux/netfilter/nf_conntrack_tcp.h \
  include/linux/netfilter_ipv4/ip_conntrack_icmp.h \
  include/net/netfilter/ipv4/nf_conntrack_icmp.h \
  include/linux/netfilter_ipv4/ip_conntrack_proto_gre.h \
  include/linux/netfilter_ipv4/ip_conntrack_sctp.h \
  include/linux/netfilter/nf_conntrack_sctp.h \
  include/linux/netfilter_ipv4/ip_conntrack_h323.h \
  include/linux/netfilter/nf_conntrack_h323_asn1.h \
  include/linux/netfilter/nf_conntrack_h323_types.h \
  include/linux/netfilter_ipv4/ip_conntrack_pptp.h \
  include/linux/netfilter_ipv4/ip_conntrack_amanda.h \
  include/linux/netfilter_ipv4/ip_conntrack_ftp.h \
  include/linux/netfilter_ipv4/ip_conntrack_irc.h \
  include/linux/netfilter_ipv4/ip_nat.h \
    $(wildcard include/config/alpha/port/restricted/cone.h) \
    $(wildcard include/config/alpha/restricted/cone.h) \
  include/linux/netfilter_ipv4.h \
  include/linux/netfilter.h \
    $(wildcard include/config/nf/nat/needed.h) \
  include/linux/netfilter_ipv4/ip_nat_pptp.h \
  include/linux/netfilter_ipv4/ip_personality.h \
  include/net/ip.h \
    $(wildcard include/config/inet.h) \
  include/net/snmp.h \
  include/linux/snmp.h \
  include/net/tcp.h \
  include/linux/crypto.h \
    $(wildcard include/config/crypto.h) \
  include/linux/uaccess.h \
  include/net/inet_hashtables.h \
  include/linux/ipv6.h \
    $(wildcard include/config/ipv6/privacy.h) \
    $(wildcard include/config/ipv6/router/pref.h) \
    $(wildcard include/config/ipv6/route/info.h) \
    $(wildcard include/config/ipv6/optimistic/dad.h) \
    $(wildcard include/config/ipv6/mroute.h) \
    $(wildcard include/config/ipv6/subtrees.h) \
  include/linux/icmpv6.h \
  include/net/if_inet6.h \
  include/net/tcp_ecn.h \
  include/net/inet_ecn.h \
  include/net/dsfield.h \
  include/linux/netfilter_ipv4/ip_conntrack_protocol.h \
  include/linux/netfilter/nfnetlink_conntrack.h \
  include/linux/netfilter/nfnetlink.h \
  /home/kwest/release.GPL/dir600b_v2.03/progs.priv/turbonat.26/sw_tcpip.config \
  /home/kwest/release.GPL/dir600b_v2.03/progs.priv/turbonat.26/alpha_forqossupport.cc \
  /home/kwest/release.GPL/dir600b_v2.03/progs.priv/turbonat.26/alpha_forqossupport.hh \
  /home/kwest/release.GPL/dir600b_v2.03/progs.priv/turbonat.26/alpha_forskipsupport.cc \

/home/kwest/release.GPL/dir600b_v2.03/progs.priv/turbonat.26/sw_tcpip.o: $(deps_/home/kwest/release.GPL/dir600b_v2.03/progs.priv/turbonat.26/sw_tcpip.o)

$(deps_/home/kwest/release.GPL/dir600b_v2.03/progs.priv/turbonat.26/sw_tcpip.o):
