#include <avr/io.h>
#include <avr/interrupt.h>
#include "communication.h"
#include "clock.h"
#include "temp.h"
#include "pwm.h"
#include "uart.h"

#include "i2c.h"
#include "ds1307.h"

int main(void)
{
  // Initialize UART interface
  comm_init();
  // Init all
  clock_init();
  // Init ADC
  temp_init();

  // Enable interrupts
  sei();

  // Wait for commands
  Command cmd;
  while (1) {
    if(comm_wait(&cmd)) {
      // Dispatch by command
      switch (cmd.command)
      {
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
        if (clock_set_datetime(&cmd.payload.datetime) ) {
          comm_send_ok();
        } else {
          comm_send_err();
        }
        break;

      case GET_ALARM:
        clock_get_alarm(cmd.payload.alarmIdx, &cmd.payload.alarm.alarm);
        comm_send_alarm(&cmd.payload.alarm.alarm);
        break;

      case SET_ALARM:
        if(clock_set_alarm(cmd.payload.alarm.alarmIdx, &cmd.payload.alarm.alarm)) {
          comm_send_ok();
        } else {
          comm_send_err();
        }
        break;

      case GET_TEMP:
        comm_send_temp(temp_get_core(), temp_get_external());
        break;

      default:
        comm_send_err();
        break;
      }
    } else {
      comm_send_err();
    }
  }

  return 0;
}
