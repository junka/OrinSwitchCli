#Makefile for Topaz API test program

include $(MSD_USER_BASE)/host/linux/makedef

CFLAGS += $(MSD_INCLUDE) -DUSE_SEMAPHORE $(TOPAZ) $(PERIDOT) $(AGATE) $(PEARL) $(AMETHYST) $(OAK) $(SPRUCE) $(BONSAIZ1) $(FIR) $(BONSAI) $(DEBUG_DEFINES)
LDFLAGS += -fPIC -L$(MSD_PATH)/host/linux/libMRegAccess -L$(MSD_PATH)/host/lib/libcli -L$(MSD_PATH)/host/lib/libpcap -L$(MSD_PATH)/host/lib/libusb/libusb/.libs -lMRegAccess -lusb-1.0 -lpcap -lcli -pthread -lstdc++

HOST=aarch64-linux-gnu
EXEC = UMSD_MCLI
MSD_LIB = $(MSD_PATH)/library/$(MSD_PROJ_NAME).o

OBJECTS = $(SAMPLE_OBJS) $(UNITTEST_OBJS) $(SWTEST_OBJS)

all: $(EXEC) $(MSD_LIB)

$(MSD_LIB):
	mkdir -p $(MSD_PATH)/library
	make -C $(MSD_PATH)

$(EXEC): $(OBJECTS) $(MSD_LIB)
	cd $(MSD_PATH)/host/lib/libpcap && \
	if [ ! -f Makefile ]; then \
		if [ -f configure ]; then ./configure --with-pcap=linux --host=$(HOST); \
		elif [ -f autogen.sh ]; then ./autogen.sh && ./configure --with-pcap=linux --host=$(HOST); \
		fi; \
	fi; \
	make;
	cd $(MSD_PATH)/host/lib/libusb && \
	if [ ! -f Makefile ]; then \
		if [ -f configure ]; then ./configure --host=$(HOST) --disable-udev; \
		elif [ -f autogen.sh ]; then ./autogen.sh && ./configure --host=$(HOST) --disable-udev; \
		fi; \
	fi; \
	make;
	make -C $(MSD_PATH)/host/linux/libMRegAccess;
	make -C $(MSD_PATH)/host/lib/libcli;
	$(CC) $(CFLAGS) -o $(EXEC) $(MSD_LIB) $(addprefix obj/,$(notdir $(OBJECTS))) $(LDFLAGS)
	@cp ../../sample/IMPSampleFiles/*.ihx .

%.o: %.c	
	@if [ ! -d obj ]; then mkdir obj; fi
	$(CC) $(CFLAGS) -c -O3 $< -o obj/$(notdir $@)
	
clean:
	make -C $(MSD_PATH)/host/linux/libMRegAccess clean;
	make -C $(MSD_PATH)/host/lib/libcli clean;
	if [ -f $(MSD_PATH)/host/lib/libpcap/Makefile ]; then \
		make -C $(MSD_PATH)/host/lib/libpcap clean; \
	fi; \
	if [ -f $(MSD_PATH)/host/lib/libusb/Makefile ]; then \
		make -C $(MSD_PATH)/host/lib/libusb clean; \
	fi;
	-@rm -f $(EXEC) *.o
	-@rm -rf obj/
	-@rm -f *.ihx

