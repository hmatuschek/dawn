#ifndef __DAWN_COMMUNICACTION_H__
#define __DAWN_COMMUNICACTION_H__

#include "ds1307.h"
#include "clock.h"

/** Specifies the possible commands. */
typedef enum {
  GET_VALUE  = 0x01,
  SET_VALUE  = 0x02,
  GET_TIME   = 0x03,
  SET_TIME   = 0x04,
  GET_ALARM  = 0x05,
  SET_ALARM  = 0x06,
  GET_TEMP   = 0x07,
  GET_NALARM = 0x08,
  GET_NONCE  = 0x09,
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
    /// For GET_ALARM the alarm index.
    uint8_t alarmIdx;
    /// For SET_ALARM, the index and alarm settings.
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
uint8_t comm_wait(Command *cmd);

// Sends (blocking) the OK response (for SET_VALUE, SET_TIME, SET_ALARM)
void comm_send_ok();
// Sends (blocking) the ERR response
void comm_send_err();
// Sends (blocking) the value (for GET_VALUE)
void comm_send_value(uint16_t value);
// Sends (blocking) the datetime (for GET_TIME)
void comm_send_time(DateTime *datetime);
// Sends (blocking) the alarm settings (for GET_ALARM)
void comm_send_alarm(Alarm *alarm);
// Sends (blocking) the core and ambient temperatures.
void comm_send_temp(uint16_t core, uint16_t ambient);
// Sends (blocking) the number of alarm settings
void comm_send_nalarm(uint8_t nalarm);
// Sends (blocking) the current nonce.
void comm_send_nonce();

#endif // __DAWN_COMMUNICACTION_H__
