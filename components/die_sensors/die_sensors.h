#ifndef DIE_SENSORS_H_
#define DIE_SENSORS_H_

enum dieSensors_t {
	TEMPERATURE = 0,
	HALL_EFFECT,
	BATTERY_VOLTAGE,
};

void publish_battery_stat();
void publish_rssi_stat();

void dieSensorsInit(void);
void dieSensorsResetNVS(void);

#endif