## Query and configure marvell switch 88q6113 and 88q5017 on orin

This could be run on Host computer or on orin directly.

## Build for x86 remote
```
git submodule update --init
cd UMSD_v7.0.0
source setenv
make
```

## Build for onboard arm
```
 git submodule update --init
 export CC=aarch64-linux-gnu-gcc
 export CXX=aarch64-linux-gnu-g++
 export LD=aarch64-linux-gnu-ld
 export HOST=aarch64-linux-gnu
 cd UMSD_v7.0.0
 source setenv
 make
```

 UMSD_MCLI application be created in UMSD_v7.0.0/host/linux

## usage
- modify umsd.cfg nic_name with the correct interface name
- run ```sudo ./USMD_CLI -f umsd.cfg ```
- telnet 0 8888
- use ```?``` to see more command help information and show commands

> for example, see Port Slave/Master mode:\
> UMSD_MCLI> getPhyMode 0 1 2 3 4 5 6 7 8 9

>or show vlan configuration:\
>UMSD_MCLI> vlan dump
