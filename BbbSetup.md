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
6. apply power


### Set the time zone
```
cd /etc
sudo mv localtime localtime.org
sudo ln -s /usr/share/zoneinfo/America/Los_Angeles /etc/localtime
```
This can probably be done better simply with:
```
  sudo dpkg-reconfigure tzdata
```

### Update the distro
```
sudo apt-get update
sudo apt-get dist-upgrade
```

###Install extra packages
```
emacs24-nox
```

### De-bloat
Inspired from:
  * http://kacangbawang.com/beagleboneblack-revc-debloat-part-1/
  * https://hacks.pmf.io/2015/06/21/securing-the-beaglebone-black/

```
sudo apt-get purge xrpd
sudo apt-get purge c9-core-installer
sudo apt-get purge bonescript
sudo apt-get purge apache2 apache2-bin apache2-data apache2-utils
sudo apt-get autoremove
```

### Secure
```
passwd debian
passwd root
```

### Configure
```
sudo apt-get install mosquitto
```

