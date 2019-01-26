#ifndef _BME280_APP_H
#define _BME280_APP_H

#include "sys.h"
#include "bme280.h"


extern struct bme280_dev Global_BME280;


void BME280_Init(struct bme280_dev *dev);
int8_t set_sensor_data_normal_mode(struct bme280_dev *dev);
void print_sensor_data(struct bme280_data *comp_data);

#endif