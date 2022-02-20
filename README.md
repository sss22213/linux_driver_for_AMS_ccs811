# Linux-driver-for-ccs811

## Test Environment:
 - Board: Raspberry pi 4b

## Install guide:
  1. Compile device tree
  ```bash=
    dtc -@ -I dts -O dtb -o ccs811-i2c.dtbo ccs811-i2c.dti
  ```
  2. Move device tree binary code to /boot/firmware/overlays/
  3. Add line on /boot/firmware/config.txt
  ```text=
  + dtoverlay=ccs811-i2c  
  ```
  4. reboot
  5. Compile driver
  ```bash=
  make
  ```
  6. Load module
  ```bash=
  sudo insmod ccs811-i2c.ko
  ```
  7. Compile test program
  ```bash=
  make test
  ```
  8. Give permission
  ```bash=
  sudo chmod 666 /dev/ccs811-i2c
  ```
  9. Excute test
  ```bash=
  ./test
  ```

## Spec
[ccs811 spec](https://cdn.sparkfun.com/assets/learn_tutorials/1/4/3/CCS811_Datasheet-DS000459.pdf)

## Data format
```text=
0x01 0x02 0x03 0x04 0x98 0x06
```

### Raw data from sensor

ECO2 => 0x01 << 8 | 0x02;

TVOC => 0x03 << 8 | 0x04;

STATUS => 0x98;

When STATUS equal 0x98, Value of ECO2 and TVOC is effective.