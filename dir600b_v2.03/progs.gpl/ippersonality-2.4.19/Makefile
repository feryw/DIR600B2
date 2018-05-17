
include ../../arch.mk
include ../../path.mk
include ../../.config


ifeq ($(strip $(ELBOX_LINUX_MAJOR)),2)
ifeq ($(strip $(ELBOX_LINUX_MINOR)),6)

OBJS := ipt_PERS.ko
obj-m := ipt_PERS.o

ipt_PERS-y := ip_personality_core.o ip_personality_seq.o ip_personality_opts.o ip_personality_win.o ip_personality_decoy.o ip_personality_vm.o ip_personality_udp.o ip_personality_ipid.o ip_personality_app.o


CFLAGS = $(KCFLAGS) -I$(KERNELDIR)/include \
		   -include $(KERNELDIR)/include/linux/autoconf.h \
		   -include $(KERNELDIR)/include/linux/version.h \
		 -I$(KERNELDIR)/include/asm/mach-generic/

all: 
	@echo -e "\033[32mBuild IP Personality ...\033[0m"
	$(Q)make -C $(KERNELDIR) M=`pwd` modules

install:
	@echo -e "\033[32mInstalling IP Personality ...\033[0m"
	$(Q)[ -d $(TARGET)/lib/modules ] || mkdir -p $(TARGET)/lib/modules
	$(Q)cp -a ./$(OBJS) $(TARGET)/lib/modules/$(OBJS)
	$(Q)[ -d $(TARGET)/etc/netsniper ] || mkdir -p $(TARGET)/etc/netsniper
	$(Q)cp -a ./conf/pers.conf $(TARGET)/etc/netsniper
	$(Q)cp -a ./conf/pers_isn.conf $(TARGET)/etc/netsniper
	
clean:
	@echo -e "\033[32mCleanning IP Personality ...\033[0m"
	$(Q)make -C $(KERNELDIR) M=`pwd` clean 
	$(Q)rm -f $(OBJS) *.o
endif
	
ifeq ($(strip $(ELBOX_LINUX_MINOR)),4)

ip_nf_pers-objs := ip_personality_core.o ip_personality_seq.o ip_personality_opts.o ip_personality_win.o ip_personality_decoy.o ip_personality_vm.o ip_personality_udp.o ip_personality_ipid.o ip_personality_app.o

OBJS := ipt_PERS.o

CFLAGS = $(KCFLAGS) -I$(KERNELDIR)/include \
		 -include $(KERNELDIR)/include/linux/version.h 

%.o: %.c
	$(Q)$(KCC) $(CFLAGS) -c $<

ipt_PERS.o: $(ip_nf_pers-objs)
	$(Q)$(LD) -r -o $@ $(ip_nf_pers-objs)

all: $(OBJS)
	@echo -e "\033[32mBuild IP Personality ...\033[0m"

install:
	@echo -e "\033[32mInstalling IP Personality ...\033[0m"
	$(Q)[ -d $(TARGET)/lib/modules ] || mkdir -p $(TARGET)/lib/modules
	$(Q)cp -a ./$(OBJS) $(TARGET)/lib/modules/$(OBJS)
	$(Q)[ -d $(TARGET)/etc/netsniper ] || mkdir -p $(TARGET)/etc/netsniper
	$(Q)cp -a ./conf/pers.conf $(TARGET)/etc/netsniper
	$(Q)cp -a ./conf/pers_isn.conf $(TARGET)/etc/netsniper
	
clean:
	@echo -e "\033[32mCleanning IP Personality ...\033[0m"
	$(Q)rm -f $(OBJS) $(ip_nf_pers-objs)

endif
endif
