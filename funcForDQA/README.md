Usage Example:

If want to set PHY to master/slave mode, use ". sw_func setPhy {PHY address} {mode}"
e.g: . sw_func setPhy 1 master => PHY address 1 set to master mode

If want to set VLAN for DQAC, use ". sw_func setVlan {Port1} {Port2}
e.g: . sw_func setVlan 1 3 => Port 1 and Port 3 set the same VLAN 
