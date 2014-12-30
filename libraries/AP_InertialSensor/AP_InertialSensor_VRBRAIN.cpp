/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include <AP_HAL.h>
#if CONFIG_HAL_BOARD == HAL_BOARD_VRBRAIN

#include "AP_InertialSensor_VRBRAIN.h"

const extern AP_HAL::HAL& hal;

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <drivers/drv_accel.h>
#include <drivers/drv_gyro.h>
#include <drivers/drv_hrt.h>

#include <stdio.h>

AP_InertialSensor_VRBRAIN::AP_InertialSensor_VRBRAIN(AP_InertialSensor &imu) :
    AP_InertialSensor_Backend(imu),
    _last_get_sample_timestamp(0)
{
}

/*
  detect the sensor
 */
AP_InertialSensor_Backend *AP_InertialSensor_VRBRAIN::detect(AP_InertialSensor &_imu)
{
    AP_InertialSensor_VRBRAIN *sensor = new AP_InertialSensor_VRBRAIN(_imu);
    if (sensor == NULL) {
        return NULL;
    }
    if (!sensor->_init_sensor()) {
        delete sensor;
        return NULL;
    }
    return sensor;
}

bool AP_InertialSensor_VRBRAIN::_init_sensor(void) 
{
    // assumes max 3 instances
    _accel_fd[0] = open(ACCEL_DEVICE_PATH, O_RDONLY);
    _accel_fd[1] = open(ACCEL_DEVICE_PATH "1", O_RDONLY);
    _accel_fd[2] = open(ACCEL_DEVICE_PATH "2", O_RDONLY);
    _gyro_fd[0] = open(GYRO_DEVICE_PATH, O_RDONLY);
    _gyro_fd[1] = open(GYRO_DEVICE_PATH "1", O_RDONLY);
    _gyro_fd[2] = open(GYRO_DEVICE_PATH "2", O_RDONLY);

    _num_accel_instances = 0;
    _num_gyro_instances = 0;
    for (uint8_t i=0; i<INS_MAX_INSTANCES; i++) {
        if (_accel_fd[i] >= 0) {
            _num_accel_instances = i+1;
            _accel_instance[i] = _imu.register_accel();
        }
        if (_gyro_fd[i] >= 0) {
            _num_gyro_instances = i+1;
            _gyro_instance[i] = _imu.register_gyro();
        }
    }    
	if (_num_accel_instances == 0) {
        return false;
    }
	if (_num_gyro_instances == 0) {
        return false;
    }

    _default_filter_hz = _default_filter();
    _set_filter_frequency(_imu.get_filter());




    return AP_PRODUCT_ID_VRBRAIN;


    return true;
}

/*
  set the filter frequency
 */
void AP_InertialSensor_VRBRAIN::_set_filter_frequency(uint8_t filter_hz)
{
    if (filter_hz == 0) {
        filter_hz = _default_filter_hz;
    }
    for (uint8_t i=0; i<_num_gyro_instances; i++) {
        ioctl(_gyro_fd[i],  GYROIOCSLOWPASS,  filter_hz);
    }
    for (uint8_t i=0; i<_num_accel_instances; i++) {
        ioctl(_accel_fd[i], ACCELIOCSLOWPASS, filter_hz);
    }
}

bool AP_InertialSensor_VRBRAIN::update(void) 
{
    // get the latest sample from the sensor drivers
    _get_sample();

    for (uint8_t k=0; k<_num_accel_instances; k++) {
        Vector3f accel = _accel_in[k];
        _rotate_and_offset_accel(_accel_instance[k], accel);
        _last_accel_update_timestamp[k] = _last_accel_timestamp[k];
    }

    for (uint8_t k=0; k<_num_gyro_instances; k++) {
        Vector3f gyro = _gyro_in[k];
        _rotate_and_offset_gyro(_gyro_instance[k], gyro);
        _last_gyro_update_timestamp[k] = _last_gyro_timestamp[k];
    }

    if (_last_filter_hz != _imu.get_filter()) {
        _set_filter_frequency(_imu.get_filter());
        _last_filter_hz = _imu.get_filter();
    }

    return true;
}

void AP_InertialSensor_VRBRAIN::_get_sample(void)
{
    for (uint8_t i=0; i<_num_accel_instances; i++) {
        struct accel_report	accel_report;
        while (_accel_fd[i] != -1 && 
               ::read(_accel_fd[i], &accel_report, sizeof(accel_report)) == sizeof(accel_report) &&
               accel_report.timestamp != _last_accel_timestamp[i]) {        
            _accel_in[i] = Vector3f(accel_report.x, accel_report.y, accel_report.z);
            _last_accel_timestamp[i] = accel_report.timestamp;
        }
    }
    for (uint8_t i=0; i<_num_gyro_instances; i++) {
        struct gyro_report	gyro_report;
        while (_gyro_fd[i] != -1 && 
               ::read(_gyro_fd[i], &gyro_report, sizeof(gyro_report)) == sizeof(gyro_report) &&
               gyro_report.timestamp != _last_gyro_timestamp[i]) {        
            _gyro_in[i] = Vector3f(gyro_report.x, gyro_report.y, gyro_report.z);
            _last_gyro_timestamp[i] = gyro_report.timestamp;
        }
    }
    _last_get_sample_timestamp = hal.scheduler->micros64();
}

bool AP_InertialSensor_VRBRAIN::gyro_sample_available(void)
{
    _get_sample();
    for (uint8_t i=0; i<_num_gyro_instances; i++) {
        if (_last_gyro_timestamp[i] != _last_gyro_update_timestamp[i]) {
            return true;
        }
    }    
    return false;
}

bool AP_InertialSensor_VRBRAIN::accel_sample_available(void)
{
    _get_sample();
    for (uint8_t i=0; i<_num_accel_instances; i++) {
        if (_last_accel_timestamp[i] != _last_accel_update_timestamp[i]) {
            return true;
        }
    }    
    return false;
}

#endif // CONFIG_HAL_BOARD
