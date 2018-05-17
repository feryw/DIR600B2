#
# Board dependent Makefile for WRG-N18
#

MYNAME:=WRG-N23
Q?=@

#############################################################################
# build root file system image

#############################################################################
# This one will be make in fakeroot.
rootfs_image:
	@rm -f fakeroot.rootfs.img
	@./progs.board/template.gw.wifi/makedevnodes rootfs
	@./tools/sqlzma/sqlzma-3.2-443-r2/mksquashfs rootfs fakeroot.rootfs.img

.PHONY: rootfs_image

#############################################################################


build_tools:
	$(Q)[ -d images ] || mkdir -p images
	$(Q)make -C ./tools/sqlzma/sqlzma-3.2-443-r2
	$(Q)make -C ./tools/buildimg
	$(Q)make -C ./tools/mkimage
	$(Q)make -C ./tools/lzma

squashfs: build_tools clean_CVS remove_fsimg
	@echo -e "\033[32m$(MYNAME): building squashfs!\033[0m"
	$(Q)fakeroot make -f progs.board/template.gw.wifi/config.mk rootfs_image
	$(Q)mv fakeroot.rootfs.img $(ROOTFS_IMG)

squashfs_lzma: build_tools clean_CVS remove_fsimg
	@echo -e "\033[32m$(MYNAME): building squashfs (LZMA)!\033[0m"
	$(Q)fakeroot make -f progs.board/template.gw.wifi/config.mk rootfs_image
	$(Q)mv fakeroot.rootfs.img $(ROOTFS_IMG)
	$(Q)chmod 664 $(ROOTFS_IMG)

.PHONY: build_tools squashfs squashfs_lzma

#############################################################################
# build kernel image

kernel_image:
	@echo -e "\033[32m$(MYNAME): building kernel image file ...\033[0m"
	$(Q)rm -f $(KERNEL_IMG)
	$(Q)mipsel-linux-objcopy -O binary -R .note -R .comment -S $(KERNELDIR)/vmlinux vmlinux.bin
	$(Q)gzip vmlinux.bin
	$(Q)mv vmlinux.bin.gz $(KERNEL_IMG)

kernel_image_lzma: build_tools
	@echo -e "\033[32m$(MYNAME): building kernel image (LZMA)...\033[0m"
	$(Q)rm -f vmlinux.bin vmlinux.bin.lzma $(KERNEL_IMG)
	$(Q)mipsel-linux-objcopy -O binary -R .note -R .comment -S $(KERNELDIR)/vmlinux vmlinux.bin
	$(Q)./tools/lzma/lzma -9 -f -S .lzma vmlinux.bin
	$(Q)./tools/mkimage/mkimage -A mips -O linux -T kernel -C lzma -a 80000000 -e $(shell readelf -h $(KERNELDIR)/vmlinux | grep "Entry" | awk '{print $$4}') -n "Linux Kernel Image" -d vmlinux.bin.lzma $(KERNEL_IMG) 
	$(Q)rm -f vmlinux.bin

.PHONY: kernel_image kernel_image_lzma

##########################################################################
#
# Common target
#

kernel_clean:
	@echo -e "\033[32m$(MYNAME): cleaning kernel ...\033[0m"
	$(Q)make -C kernel mrproper

kernel: kernel_clean
	echo -e "\033[32m$(MYNAME) Building kernel ...\033[0m"; \
	KERNEL_CONFIG=$(shell echo progs.board/kernel.$(ELBOX_TEMPLATE_NAME).$(ELBOX_BRAND_NAME)_$(ELBOX_MODEL_NAME).config);\
	if [ ! -f $$KERNEL_CONFIG ]; then \
	KERNEL_CONFIG=$(shell echo progs.board/kernel.$(ELBOX_TEMPLATE_NAME).config); \
	cp $$KERNEL_CONFIG kernel/.config; \
	fi; \
	echo -e "\033[32m$(MYNAME) Building kernel with $$KERNEL_CONFIG...\033[0m"; \
	cp $$KERNEL_CONFIG kernel/.config;
	$(Q)make -C kernel oldconfig
	$(Q)make -C kernel dep
	$(Q)make -C kernel

ifeq (buildno, $(wildcard buildno))

ifdef PREDEFINE_RELIMAGE
RELIMAGE:=$(PREDEFINE_RELIMAGE)
else
BUILDNO:=$(shell cat buildno)
RELIMAGE:=$(shell echo $(ELBOX_MODEL_NAME)_v$(ELBOX_FIRMWARE_VERSION)_$(BUILDNO))
endif

.PHONY: lang_pack
lang_pack:
ifeq ($(strip $(ELBOX_CREATE_LANGUAGE_PACK)),y)
	$(Q)for i in $(LANGUAGE); do \
		./tools/sqlzma/sqlzma-3.2-443-r2/mksquashfs language/$$i langpack.bin; \
		chmod 664 langpack.bin ; \
		./tools/buildimg/v2image -v -i langpack.bin -o images/$(RELIMAGE)_langpack_$$i.bin \
			-d /dev/mtdblock/5 -s $(ELBOX_SIGNATURE) -c; \
		rm -f langpack.bin ; \
		done
endif

release: kernel_image_lzma squashfs lang_pack
	@echo -e "\033[32m$(MYNAME) make F/W image (release image)!\033[0m"
	$(Q)./tools/buildimg/packimgs -v -o raw.img -i $(KERNEL_IMG) -i $(ROOTFS_IMG)
	$(Q)./tools/buildimg/v2image -v -i raw.img -o web.img -d /dev/mtdblock/2 -s $(ELBOX_SIGNATURE) -c
	$(Q)rm -f raw.img
	$(Q)./tools/release.sh web.img $(RELIMAGE).bin

magic_release: kernel_image_lzma squashfs
	@echo -e "\033[32m$(MYNAME) make F/W image (magic release image)!\033[0m"
	$(Q)./tools/buildimg/packimgs -v -o raw.img -i $(KERNEL_IMG) -i $(ROOTFS_IMG)
	$(Q)./tools/buildimg/v2image -v -i raw.img -o web.img -d /dev/mtdblock/2 -s wrgn23_aLpHa -c
	$(Q)rm -f raw.img
	$(Q)./tools/release.sh web.img $(RELIMAGE).magic.bin

tftpimage: kernel_image_lzma squashfs
	@echo -e "\033[32m$(MYNAME) make F/W image (tftp image)!\033[0m"
	$(Q)rm -f $(TFTPIMG)
	$(Q)./tools/buildimg/packimgs -v -o raw.img -i $(KERNEL_IMG) -i $(ROOTFS_IMG)
	$(Q)./tools/tftpimage.sh $(TFTPIMG)

else
release tftpimage:
	@echo -e "\033[32m$(MYNAME): Can not build image, ROOTFS is not created yet !\033[0m"
endif

.PHONY: kernel release tftpimage
