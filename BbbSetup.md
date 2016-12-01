## Setup the Beagle Bone Black

### Prepare the image
Download:<br>
  `bone-debian-8.6-iot-armhf-2016-11-06-4gb.img.xz`
  
From:<br>
  https://debian.beagleboard.org/images/

Verify checksum:
```
  sha256sum.exe bone-debian-8.6-iot-armhf-2016-11-06-4gb.img.xz
```
against:
```
  bone-debian-8.6-iot-armhf-2016-11-06-4gb.img.xz.sha256sum
```

Burn to SD card using Win32DiskImager (if on Windows…)

### Boot from the SD card
1. Card not powered
2. Insert SD card
3. Press the boot button 
4. Apply power with USB from computer
5. Let boot
6. Log on the board: ssh debian@192.168.7.2 (temppwd)


### Flash eMMC
1. `cd /opt/scripts/tools/eMMC`
2. `sudo bash`
3. `./init-eMMC-flasher-v3.sh`
4. remove power
5. remove SD-card
6. connect ethernet cable
7. apply power


### De-bloat
Inspired from:
  * http://kacangbawang.com/beagleboneblack-revc-debloat-part-1/
  * https://hacks.pmf.io/2015/06/21/securing-the-beaglebone-black/

```
sudo apt-get purge xrdp
sudo apt-get purge bone101
sudo apt-get purge c9-core-installer
sudo apt-get purge bb-node-red-installer
sudo apt-get purge bonescript
sudo apt-get purge apache2 apache2-bin apache2-data apache2-utils
sudo apt-get autoremove
```

### Update the distro
```
sudo apt-get update
sudo apt-get dist-upgrade
```

### Set the time zone
Select the right time zone
```
sudo dpkg-reconfigure tzdata
```

Old manual method, just for reference:
```
cd /etc
sudo mv localtime localtime.org
sudo ln -s /usr/share/zoneinfo/America/Los_Angeles /etc/localtime
```
### Change the hostname
```
sudo vi /etc/hostname
sudo vi /etc/hosts
```

### Install extra packages
```
emacs24-nox
byobu
```

### Secure
```
passwd debian
passwd root
```

### Configure
```
sudo apt-get install mosquitto mosquitto-clients

```

### Python MQTT client
Install Paho: https://pypi.python.org/pypi/paho-mqtt
```
sudo pip install paho-mqtt
```

### Pushbullet Python
```
sudo pip install pushbullet.py
```

### SMTP server (not using, forget about it for now)
```
sudo apt-get install postfix mailutils
```
Select:
 - setup: Internet Site
 - system mail name: bbb.flupes.org

Edit: `/etc/postfix/main.cf`

Reload: `sudo systemctl restart postfix`

