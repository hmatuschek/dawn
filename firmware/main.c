#ifndef F_CPU
#define F_CPU 16000000L
#endif

#include "clock.h"
#include "communication.h"
#include <avr/interrupt.h>

int main(void)
{
  // Init all
  clock_init();
  // Initialize UART interface
  comm_init();

  // Enable interrupts
  sei();

  // The received command
  Command cmd;
  while (1) {
    // Wait for a valid command
    comm_wait(&cmd);

    // Dispatch by command
    switch (cmd.command) {
    case GET_VALUE:
      comm_send_value(clock_get_value());
      break;
    case SET_VALUE:
      clock_set_value(cmd.payload.value);
      comm_send_ok();
      break;
    case GET_TIME:
      clock_get_datetime(&cmd.payload.datetime);
      comm_send_time(&cmd.payload.datetime);
      break;
    case SET_TIME:
      clock_set_datetime(&cmd.payload.datetime);
      comm_send_ok();
      break;
    case GET_ALARM:
      clock_get_alarm(cmd.payload.alarmIdx, &cmd.payload.alarm.alarm);
      comm_send_alarm(&cmd.payload.alarm.alarm);
      break;
    case SET_ALARM:
      clock_set_alarm(cmd.payload.alarm.alarmIdx, &cmd.payload.alarm.alarm);
      comm_send_ok();
      break;
    default:
      break;
    }
  }

  return 0;
}
