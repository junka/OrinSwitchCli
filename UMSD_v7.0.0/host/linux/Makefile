#Makefile for Topaz API test program

include $(MSD_USER_BASE)/host/linux/makedef

CFLAGS += $(MSD_INCLUDE) -g -fno-omit-frame-pointer -DUSE_SEMAPHORE $(TOPAZ) $(PERIDOT) $(AGATE) $(PEARL) $(AMETHYST) $(OAK) $(SPRUCE) $(BONSAIZ1) $(FIR) $(BONSAI) $(DEBUG_DEFINES) $(LIBPCAP_DEFINES) 
LDFLAGS += -fPIC -L$(MSD_PATH)/host/linux/libMRegAccess -L$(MSD_PATH)/host/lib/libcli -L$(MSD_PATH)/host/lib/libusb/libusb/.libs -lMRegAccess -lusb-1.0 -lcli -pthread -lstdc++

ifeq ($(LIBPCAP), YES)
LDFLAGS += -L$(MSD_PATH)/host/lib/libpcap -lpcap
endif

HOST?=x86_64-linux-gnu
EXEC = UMSD_MCLI
MSD_LIB = $(MSD_PATH)/library/$(MSD_PROJ_NAME).o

OBJECTS = $(SAMPLE_OBJS) $(UNITTEST_OBJS) $(SWTEST_OBJS)

all: $(EXEC) $(MSD_LIB)

ifeq ($(LIBPCAP), YES)
build_libpcap:
	cd $(MSD_PATH)/host/lib/libpcap && \
    if [ -f configure ]; then ./configure --disable-dbus --disable-rdma --enable-usb=no --disable-bluetooth --without-ibverbs --with-pcap=linux --host=$(HOST); \
    elif [ -f autogen.sh ]; then ./autogen.sh && ./configure --disable-dbus --disable-rdma --enable-usb=no --disable-bluetooth --without-ibverbs --with-pcap=linux --host=$(HOST); \
    fi; \
    make;
endif

$(MSD_LIB):
	mkdir -p $(MSD_PATH)/library
	make -C $(MSD_PATH)

ifeq ($(LIBPCAP), YES)
$(EXEC): build_libpcap
endif

$(EXEC): $(OBJECTS) $(MSD_LIB)
	cd $(MSD_PATH)/host/lib/libusb && \
	if [ -f configure ]; then ./configure --host=$(HOST) --disable-udev; \
	elif [ -f autogen.sh ]; then ./autogen.sh && ./configure --host=$(HOST) --disable-udev; \
	fi; \
	make;
	make -C $(MSD_PATH)/host/linux/libMRegAccess;
	make -C $(MSD_PATH)/host/lib/libcli;
	$(CC) $(CFLAGS) -o $(EXEC) $(MSD_LIB) $(addprefix obj/,$(notdir $(OBJECTS))) $(LDFLAGS)
	@cp ../../sample/IMPSampleFiles/*.ihx .

%.o: %.c	
	@if [ ! -d obj ]; then mkdir obj; fi
	$(CC) $(CFLAGS) -c $< -o obj/$(notdir $@)
	
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

