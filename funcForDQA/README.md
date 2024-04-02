Usage Example:

!!! Please ensure to verify and modify the "NIC_NAME" parameter in the script file "sw_func" according to the network interface card (NIC) you are using !!!

To set the PHY to master/slave mode, utilize the following command:
. sw_func setPhy {PHY address} {mode} {PHY address} {mode} ...
Example usage:
- . sw_func setPhy 1 master => Set PHY address 1 to master mode
- . sw_func setPhy 1 master 2 slave 3 master => Set multiple PHYs to different modes simultaneously

To set VLAN for DQAC, employ the command format:
. sw_func setVlan {Port1} {Port2} {Port3} {Port4}
Example usage:
- . sw_func setVlan 1 3 => Set the same VLAN for Port 1 and Port 3
- . sw_func setVlan 1 2 3 4 5 6 => Set the same VLAN for Port 1 and Port 2, Port 3 and Port 4, Port 5 and Port 6

To reset all VLAN status, use the command:
. sw_func resetVlan

