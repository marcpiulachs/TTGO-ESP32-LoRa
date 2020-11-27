#ifndef VALVE_H_
#define VALVE_H_

#define VALVE_OPENED 1
#define VALVE_CLOSED 0

void open_valve1(void);
void close_valve1(void);
void open_valve2(void);
void close_valve2(void);
void publish_valve1_stat(void);
void publish_valve2_stat(void);

void valve_init(void);

#endif