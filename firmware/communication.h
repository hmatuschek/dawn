#ifndef __DAWN_COMMUNICACTION_H__
#define __DAWN_COMMUNICACTION_H__

#include "ds1307.h"
#include "clock.h"

/** Specifies the possible commands. */
typedef enum {
  CMD_MIN = 0x00,
  GET_VALUE,
  SET_VALUE,
  GET_TIME,
  SET_TIME,
  GET_ALARM,
  SET_ALARM,
  GET_TEMP,
  CMD_MAX,
} CommandFlag;


/** Union of all possible commands. */
typedef struct {
  /// The command flag.
  uint8_t command;
  /// Payload union
  union {
    /// For SET_VALUE, the value.
    uint16_t value;
    /// For SET_TIME the date & time.
    DateTime datetime;
    /// For GET_ALARM, the alarm index.
    uint8_t alarmIdx;
    /// For set alarm, the index and alarm settings.
    struct {
      uint8_t alarmIdx;
      Alarm   alarm;
    } alarm;
  } payload;
  /// MAC
  uint8_t mac[8];
} Command;


/// Initializes the uart.
void comm_init();

// Waits for a command to be received
void comm_wait(Command *cmd);

// Sends (blocking) the OK response (for SET_VALUE, SET_TIME, SET_ALARM)
void comm_send_ok();
// Sends (blocking) the value (for GET_VALUE)
void comm_send_value(uint16_t value);
// Sends (blocking) the datetime (for GET_TIME)
void comm_send_time(DateTime *datetime);
// Sends (blocking) the alarm settings (for GET_ALARM)
void comm_send_alarm(Alarm *alarm);
// Sends (blocking) the core and ambient temperatures.
void comm_send_temp(uint16_t core, uint16_t ambient);

#endif // __DAWN_COMMUNICACTION_H__
