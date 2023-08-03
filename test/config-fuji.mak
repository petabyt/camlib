connect:
	systemctl stop NetworkManager
	iwconfig wlp0s20f3 essid "FUJIFILM-X-A2-5DBC"
	dhclient wlp0s20f3

fix:
	systemctl start NetworkManager

canon:
	systemctl stop NetworkManager
	iwconfig wlp0s20f3 essid "EOST6{-361_Canon0A" key s:19549284 [2]
	dhclient wlp0s20f3
