include $(MSD_USER_BASE)/host/linux/makedef

CFLAGS = -static
CFLAGS += $(MSD_INCLUDE)

LIBS = -L$(MSD_PATH)/host/lib/libusb/libusb/.libs -lusb-1.0

cyobject = libcyusb.o
cysource= libcyusb.c
output = libMRegAccess.so
objects = libMRegAccess.o MRegAccess_v61xx.o MRegAccess_v6.o MRegAccess_v5.o Wrapper.o $(cyobject)

all: $(output) 
$(output): $(objects)
	# $(CXX) $(CFLAGS)  $(objects) $(LIBS) -o $(output) -pthread
	$(AR) rcs $(output) $(objects)

%.o:%.cpp
	$(CXX) $(CFLAGS) -c $<
$(cyobject): $(cysource)
	$(CXX) $(CFLAGS) $(LIBS) -o $(cyobject) -c $(cysource)
clean:
	-@$(RM) $(output1) $(output) $(objects) 
	
.PHONY: clean

