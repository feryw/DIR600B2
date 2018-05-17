-include ../../arch.mk
-include ../../path.mk
-include ../../.config

ifeq ($(strip $(ELBOX_PROGS_PRIV_TURBONAT_26)),y)
BIN := sw_tcpip.ko
obj-m += sw_tcpip.o

all:
	@echo -e "\033[32mBuilding fastnat module...\033[0m"

###--------------------------------------------------###
	[ ! -f sw_tcpip.config ] || rm -f sw_tcpip.config
	touch sw_tcpip.config
	echo "#define CONFIG_SKIP_TURBONAT" >> sw_tcpip.config
ifeq ($(strip $(ELBOX_PROGS_PRIV_TURBONAT_QOS)),y)
	echo "#define CONFIG_QOS_TURBONAT" >> sw_tcpip.config
endif
###--------------------------------------------------###
	
	$(Q)make -C $(KERNELDIR) M=`pwd` modules

clean:
	@echo -e "\033[32mCleaning fastnat module...\033[0m"
	$(Q)make -C $(KERNELDIR) M=`pwd` clean
	$(Q)rm -f *~

else
-include ../config.arch
BIN := sw_tcpip.o
CFLAGS = $(KCFLAGS) -I$(KERNELDIR)/include

%.o:	%.c
	@echo -e "\033[32mBuilding fastnat module...\033[0m"
	$(Q)$(KCC) $(CFLAGS) -c $< -o $@

all:    $(BIN)

clean:
	@echo -e "\033[32mCleaning fastnat module...\033[0m"
	$(Q)rm -f *~ *.o

endif

install:
	@echo -e "\033[32mInstalling fastnat module...\033[0m"
	$(Q)[ -d $(TARGET)/lib/modules ] || mkdir -p $(TARGET)/lib/modules
	$(Q)cp -a ./$(BIN) $(TARGET)/lib/modules/

.PHONY: all install clean
