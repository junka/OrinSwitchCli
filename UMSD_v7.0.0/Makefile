MVL_DRV_DIRS := $(MSD_USER_BASE)

all: SW

SW:
	mkdir -p $(MVL_DRV_DIRS)/library
	make -C $(MVL_DRV_DIRS)/src
	make -C $(MVL_DRV_DIRS)/host/linux all

clean :
	make -C $(MVL_DRV_DIRS)/host/linux clean
	make -C $(MVL_DRV_DIRS)/src clean
