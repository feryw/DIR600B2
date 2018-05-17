#
# Created by David Hsieh <david_hsieh@alphanetworks.com>
#

-include ../../path.mk
-include ../../.config

TARGET	:= buildimg v2image packimgs genbn bcat alpha_pack

HGCC	:= gcc
HLD		:= gcc
HCFLAGS	:= -Wall -I$(TOPDIR)/include
HLDFKAGS:=


all:

###################################################################
# execution

packimgs: packimgs.o
	$(HLD) $(HLDFLAGS) packimgs.o -o $@

buildimg: buildimg.o md5c.o
	$(HLD) $(HLDFLAGS) buildimg.o md5c.o -o $@

v2image: v2image.o md5c.o
	$(HLD) $(HLDFLAGS) v2image.o md5c.o -o $@

genbn: genbn.o
	$(HLD) $(HLDFLAGS) genbn.o -o $@

bcat: bcat.o
	$(HLD) $(HLDFLAGS) bcat.o -o $@

alpha_pack: alpha_pack.o
	$(HLD) $(HLDFLAGS) alpha_pack.o -o $@

###################################################################
# objects

packimgs.o: packimgs.c
	$(HGCC) $(HCFLAGS) -c packimgs.c

buildimg.o: buildimg.c imghdr.h md5.h
	$(HGCC) $(HCFLAGS) -c buildimg.c

v2image.o: v2image.c imghdr.h md5.h
	$(HGCC) $(HCFLAGS) -c v2image.c

md5c.o: md5c.c md5.h
	$(HGCC) $(HCFLAGS) -c md5c.c

genbn.o: genbn.c
	$(HGCC) $(HCFLAGS) -c genbn.c

bcat.o: bcat.c
	$(HGCC) $(HCFLAGS) -c bcat.c

alpha_pack.o: alpha_pack.c
	$(HGCC) $(HCFLAGS) -c alpha_pack.c

###################################################################
# clean
clean:
	rm -f *.o *.gdb core
	rm -f buildno

.PHONY: all clean
