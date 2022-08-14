# M5-HSD-35510S MIPI Driver for Raspberry pi
## Brief
This is a mipi screen driver for raspberry pi and it has been tested on Raspberry pi 3b+ and Raspberry pi 4b. It uses 2 lanes of MIPI dsi socket on raspberry, and the SCL pin (GPIO45) is served as screen reset, SDA pin (GPIO44) is served as backlight control.
![image](https://github.com/Huskia/M5-HSD-35510S/blob/main/doc/img/20220814_111057.jpg)
## Update
* 2022-8-14 Kernal 5.15 is supported.
## Usage
If you want to build your own screen driver, follow instructions will help.  
First, download raspberry pi official image, in this case the image is 64-bit bullseye version. Burn the image into a TF card and boot your raspberry pi.   
After your raspberry booted, install raspberrypi-kernel-headers.  
```
sudo apt update
sudo apt install raspberrypi-kernel-headers
```
Then create a new folder wherever you want and create 3 files: "Makefile", "panel-xxx-xxx.c", "vc4-kms-dsi-xxx.dts". You can refer to the source code. When you finish these files, run `make` and it will start building automatically. If build success, a "panel-xxx-xxx.ko" file will show in folder.  
Next bulid device tree and copy them to corresponding folder.  
```
dtc -@ -I dts -O dtb -o vc4-kms-dsi-xxx.dtbo vc4-kms-dsi-xxx.dts
sudo cp vc4-kms-dsi-xxx.dtbo /boot/overlays/
sudo cp panel-xxx-xxx.ko /lib/modules/`uname -r`/kernel/drivers/gpu/drm/panel
sudo depmod
```
Finally edit /boot/config.txt and add parameters below to enable the driver.  
```
ignore_lcd=1
dtoverlay=vc4-kms-dsi-xxx # Same as dtbo file name
```
Reboot your raspberry pi and desktop will show up.
