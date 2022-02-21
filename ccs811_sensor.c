#include "ccs811_sensor.h"

static inline void _write_reg(struct _ccs811_device *ccs811_device, u8 mem_address, u8 *data, u8 write_len)
{
    int write_buf_idx = 1;

    u8 write_buf[100] = {0};

    write_buf[0] = mem_address;

    for (write_buf_idx = 1; write_buf_idx < write_len+1; write_buf_idx++) {
        write_buf[write_buf_idx] = data[write_buf_idx-1];
    }

    i2c_master_send(ccs811_device->client, write_buf, write_len+1);
}

static inline void _read_reg(struct _ccs811_device *ccs811_device, u8 mem_address, u8 *data, u8 read_len)
{
    i2c_master_send(ccs811_device->client, &mem_address, 1);

    i2c_master_recv(ccs811_device->client, data, read_len);
}

void ccs811_init(struct _ccs811_device *ccs811_device, struct i2c_client *client)
{
    ccs811_device->client = client;
}

/**
 * @brief 
 * 
 * @param ccs811_device 
 * @return u8 
 */
static inline void _get_hw_id(struct _ccs811_device *ccs811_device, u8 *buf)
{
    _read_reg(ccs811_device, HW_ID, buf, 1);
}

static inline void _get_hw_version(struct _ccs811_device *ccs811_device, u8 *buf)
{
    _read_reg(ccs811_device, HW_Version, buf, 1);
}

static inline void _get_status(struct _ccs811_device *ccs811_device, u8 *buf)
{
   _read_reg(ccs811_device, STATUS, buf, 1);
}

static inline void _transfer_boot_to_app(struct _ccs811_device *ccs811_device, u8 *read_buf)
{
    _write_reg(ccs811_device, APP_START, NULL, 0);

    _get_status(ccs811_device, read_buf);
}

static inline void _reset(struct _ccs811_device *ccs811_device)
{   
    u8 reset_cmd[4] = {0x11, 0xE5, 0x72, 0x8A};
    _write_reg(ccs811_device, SW_RESET, reset_cmd, sizeof(reset_cmd)/sizeof(reset_cmd[0]));
}

void ccs811_get_status(struct _ccs811_device *ccs811_device, u8 *buf)
{
   _read_reg(ccs811_device, STATUS, buf, 1);
}

void ccs811_get_error_id(struct _ccs811_device *ccs811_device, u8 *buf)
{
   _read_reg(ccs811_device, ERROR_ID, buf, 1);
}


void ccs881_set_humidity(struct _ccs811_device *ccs811_device, u8 humidity_high, u8 humidity_low)
{
    u8 eva_data[4] = {0};

    _read_reg(ccs811_device, ENV_DATA, eva_data, 4);

    eva_data[0] = humidity_high;
    eva_data[1] = humidity_low;

    _write_reg(ccs811_device, ENV_DATA, eva_data, 4);
}

void ccs881_set_temperature(struct _ccs811_device *ccs811_device, u8 temperature_high, u8 temperature_low)
{
    u8 eva_data[4] = {0};

    _read_reg(ccs811_device, ENV_DATA, eva_data, 4);

    eva_data[2] = temperature_high;
    eva_data[3] = temperature_low;

    _write_reg(ccs811_device, ENV_DATA, eva_data, 4);
}

void ccs881_set_measure_mode(struct _ccs811_device *ccs811_device, u8 mode)
{
    u8 cur_mode = 0;

    _read_reg(ccs811_device, MEAS_MODE, &cur_mode, 1);

    cur_mode = (cur_mode & ~(1 << 4 | 1 << 5 | 1 << 6)) | (mode << 4);

    _write_reg(ccs811_device, MEAS_MODE, &cur_mode, 1);
}

void ccs811_get_tovc(struct _ccs811_device *ccs811_device, u8 *buf)
{
    u16 tvoc = 0;

    u16 eco2 = 0;

    _read_reg(ccs811_device, ALG_RESULT_DATA, buf, 8);

    eco2 = buf[0] << 8 | buf[1];

    tvoc = buf[2] << 8 | buf[3];

    // Ripple
    if ((eco2 >= CCS811_ECO2_MAXIMUM || eco2 <= CCS811_ECO2_MINIMUM) || (tvoc >= CCS811_TVOC_MAXIMUM || tvoc <= CCS811_TVOC_MINIMUM)) {
        buf[4] = _CCS811_TVOC_ECO2_STATUS_INEFFECTIVE;
        memset(buf, 0, sizeof(buf[0])*8);
    } else {
        buf[4] = _CCS811_TVOC_ECO2_STATUS_EFFECTIVE;
    }
}

_CCS811_INITIAL_RESULT ccs811_startup(struct _ccs811_device *ccs811_device)
{
    u8 read_buf[5] = {0};

    // Reset CCS811
    _reset(ccs811_device);

    msleep(100);

    // Get HW id
    _get_hw_id(ccs811_device, read_buf);
    if (read_buf[0] != 0x81) {
        return _CCS811_HW_ID_FAILED;
    }

    // Get HW version
    _get_hw_version(ccs811_device, read_buf);
    if ((read_buf[0] & 0x10) == 0) {
        return _CCS811_HW_VERSION_FAILED;
    }

    // Transfer boot mode to app mode
    _transfer_boot_to_app(ccs811_device, read_buf);
    if (((read_buf[0] >> 4) & 0x01) == 0) {
        return _CCS811_TRANSFER_FROM_BOOT_TO_APP_FAILED;
    }

    // Set default temperature to 25C, humidity to 50%
    ccs881_set_humidity(ccs811_device, 0x64, 0x00);
    
    ccs881_set_temperature(ccs811_device, 0x64, 0x00);

    // Config Mode 1
    //ccs881_set_measure_mode(ccs811_device, 0);

    return _CCS811_INITIAL_SUCCESS;
}