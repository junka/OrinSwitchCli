Usage Example:

!!! Please check the name of the network interface card (NIC) you are using, and modify the "NIC_NAME" item in script file "sw_func" !!!

If want to set PHY to master/slave mode, use ". sw_func setPhy {PHY address} {mode} {PHY address} {mode} ... "
e.g: . sw_func setPhy 1 master => PHY address 1 set to master mode
     . sw_func setPhy 1 master 2 slave 3 master => set mulitple PHYs mode at same time

If want to set VLAN for DQAC, use ". sw_func setVlan {Port1} {Port2} {Port3} {Port4}
e.g: . sw_func setVlan 1 3 => Port 1 and Port 3 set the same VLAN 
     . sw_func setVlan 1 2 3 4 5 6 => set Port 1 Port 2 same VLAN, Port 3 Port 4 same VLAN, 
                                      Port 5 Port 6 same VLAN
Reset all VLAN status : . sw_func resetVlan
