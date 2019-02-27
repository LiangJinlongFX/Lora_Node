#include "bme280_app.h"
#include "bme280.h"
#include "stdio.h"
#include "my_iic2.h"
#include "delay.h"

struct bme280_dev Global_BME280;

/**
 * BME280初始化(user)
 * @param   
 * @return 0-OK 1-ERROR 
 * @brief 
 **/
uint8_t BME280_Init(struct bme280_dev *dev)
{
	// 初始化soft IIC2
	IIC2_Init();
	// 初始化设备底层函数
	dev->dev_id = 0x76;
	dev->intf = BME280_I2C_INTF;
	dev->read = user_i2c_read;
	dev->write = user_i2c_write;
	dev->delay_ms = (void*)delay_ms;
	if(bme280_init(dev))
	{
		printf("BME280 Init ERROR!\r\n");
		return 1;
	}
	else
	{
		printf("BME280 Init OK!\r\n");
		return 0;
	}
}

int8_t set_sensor_data_normal_mode(struct bme280_dev *dev)
{
	int8_t rslt;
	uint8_t settings_sel;

	/* Recommended mode of operation: Indoor navigation */
	dev->settings.osr_h = BME280_OVERSAMPLING_1X;
	dev->settings.osr_p = BME280_OVERSAMPLING_16X;
	dev->settings.osr_t = BME280_OVERSAMPLING_2X;
	dev->settings.filter = BME280_FILTER_COEFF_16;
	dev->settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

	settings_sel = BME280_OSR_PRESS_SEL;
	settings_sel |= BME280_OSR_TEMP_SEL;
	settings_sel |= BME280_OSR_HUM_SEL;
	settings_sel |= BME280_STANDBY_SEL;
	settings_sel |= BME280_FILTER_SEL;
	rslt = bme280_set_sensor_settings(settings_sel, dev);
	rslt = bme280_set_sensor_mode(BME280_NORMAL_MODE, dev);

	return rslt;
}

void print_sensor_data(struct bme280_data *comp_data)
{
#ifdef BME280_FLOAT_ENABLE
        printf("%0.2f, %0.2f, %0.2f\r\n",comp_data->temperature, comp_data->pressure, comp_data->humidity);
#else
        printf("%ld, %ld, %ld\r\n",comp_data->temperature, comp_data->pressure, comp_data->humidity);
#endif
}

