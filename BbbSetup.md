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
#sudo apt-get purge xrdp
sudo apt-get purge -y bone101
sudo apt-get purge -y c9-core-installer
sudo apt-get purge -y bb-node-red-installer
sudo apt-get purge -y bonescript
sudo apt-get purge -y apache2 apache2-bin apache2-data apache2-utils
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

### Setup ntp
```
sudo apt-get install -y ntp
```

### Change the hostname
```
sudo vi /etc/hostname
sudo vi /etc/hosts
sudo reboot
```

### Install extra packages
```
sudo apt-get install -y emacs24-nox
sudo apt-get install -y byobu
```

### Secure
```
passwd debian
sudo passwd root
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

### Setup pushbullet credentials
Retrieve your pushbullet password and create a `~/.netrc` file of the sort:
```
machine catdoor
login your_email_address
password your_pushbullet_password
```
The `.netrc` files should be readable only by the user:
```
chmod go-r ~/.netrc
```

### Install service
````
# from the catdoor directory
mkdir logs
cd scripts
# edit the catdoor.service and start-catdoor.bash files to reflect where
# the repo has been checked-out
# make the systemd unit owned by root (otherwise if will not start automatically at boot)
sudo chown root.root catdoor.service
# enable using the full path to the unit
sudo systemctl enable /home/debian/catdoor/scripts/catdoor.service
```
### Some info

#### LEDS
Boot configuration:
  - USR0 is configured at boot to blink in a heartbeat pattern
  - USR1 is configured at boot to light during microSD card accesses
  - USR2 is configured at boot to light during CPU activity
  - USR3 is configured at boot to light during eMMC accesses

http://robotic-controls.com/learn/beaglebone/beaglebone-black-built-leds
