sudo systemctl disable NetworkManager
sudo systemctl stop NetworkManager
sudo systemctl disable avahi-daemon
sudo systemctl stop avahi-daemon
sudo systemctl stop wpa_supplicant

sudo airmon-ng start wlp1s0
