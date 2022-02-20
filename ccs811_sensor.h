#ifndef __CCS811_H__
#define __CCS811_H__
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>

#define CCS811_SLAVE_ADDRESS 0x5A

#define CCS811_TVOC_MAXIMUM 1187
#define CCS811_TVOC_MINIMUM 0
#define CCS811_ECO2_MAXIMUM 8192
#define CCS811_ECO2_MINIMUM 400

typedef enum {
    STATUS = 0x00,
    MEAS_MODE = 0x01,
    ALG_RESULT_DATA = 0x02,
    RAW_DATA = 0x03,
    ENV_DATA = 0x05,
    NTC = 0x06,
    THRESHOLDS = 0x10,
    BASELINE = 0x11,
    HW_ID = 0x20,
    HW_Version = 0x21,
    FW_Boot_Version = 0x23,
    FW_App_Version = 0x24,
    ERROR_ID = 0xE0,
    SW_RESET = 0xFF
} _CCS811_REGISTER_ADDRESS;

typedef enum {
    APP_START = 0xF4
} _CCS811_BOOTLOADER_REGISTER_ADDRESS;

typedef enum {
    _CCS811_HW_ID_FAILED,
    _CCS811_HW_VERSION_FAILED,
    _CCS811_TRANSFER_FROM_BOOT_TO_APP_FAILED,
    _CCS811_INITIAL_SUCCESS
} _CCS811_INITIAL_RESULT;

struct _ccs811_device {
    struct i2c_client *client;
    uint8_t hw_version;
};

void ccs811_init(struct _ccs811_device*, struct i2c_client*);
_CCS811_INITIAL_RESULT ccs811_startup(struct _ccs811_device*);
void ccs811_get_tovc(struct _ccs811_device*, u8*);
void ccs881_set_measure_mode(struct _ccs811_device*, u8);
void ccs881_set_humidity(struct _ccs811_device*, u8, u8);
void ccs881_set_temperature(struct _ccs811_device*, u8, u8);

#endif