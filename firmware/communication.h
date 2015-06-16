/**
 * Serial communication protocol.
 *
 * The general packet format is
 * +-- ... --+-- ... --+
 * | Payload |   MAC   |
 * +-- ... --+-- ... --+
 *
 * where the payload is of variable size (detailed description below) and the
 * MAC is a 64 bit value computed over the payload with the shared secret.
 *
 * GET_VALUE: Returns the current luminicence.
 * Command:
 *  +------+
 *  | 0x01 |
 *  +------+
 * Response:
 *  +------+------+
 *  |    VALUE    |
 *  +------+------+
 *
 * SET_VALUE: Sets the current luminicence value.
 * Command:
 *  +------+------+------+
 *  | 0x02 |    VALUE    |
 *  +------+------+------+
 * Response:
 *  +------+
 *  | 0x00 |
 *  +------+
 *
 * GET_TIME: Returns the current time
 * Command:
 *  +------+
 *  | 0x03 |
 *  +------+
 * Response:
 *  +------+------+------+------+------+------+------+------+
 *  |     YEAR    |  MON |  DAY |  DOW | HOUR |  MIN |  SEC |
 *  +------+------+------+------+------+------+------+------+
 *
 * SET_TIME
 * Command:
 *  +------+------+------+------+------+------+------+------+------+
 *  | 0x04 |     YEAR    |  MON |  DAY |  DOW | HOUR |  MIN |  SEC |
 *  +------+------+------+------+------+------+------+------+------+
 * Response:
 *  +------+
 *  | 0x00 |
 *  +------+
 *
 * GET_ALARM:
 * Command:
 *  +------+------+
 *  | 0x05 |  IDX |
 *  +------+------+
 * Response:
 *  +------+------+------+
 *  | DOWF | HOUR |  MIN |
 *  +------+------+------+
 *
 * SET_ALARM:
 * Command:
 *  +------+------+------+------+------+
 *  | 0x06 |  IDX | DOWF | HOUR |  MIN |
 *  +------+------+------+------+------+
 * Response:
 *  +------+
 *  | 0x00 |
 *  +------+
 */

#ifndef __LAMPE_COMMUNICACTION_H__
#define __LAMPE_COMMUNICACTION_H__

#include "ds1307.h"
#include "clock.h"


typedef enum {
  CMD_MIN = 0x00,
  GET_VALUE,
  SET_VALUE,
  GET_TIME,
  SET_TIME,
  GET_ALARM,
  SET_ALARM,
  CMD_MAX,
} CommandFlag;

typedef struct {
  uint8_t command;
  union {
    uint16_t value;
    DateTime datetime;
    uint8_t alarmIdx;
    struct {
      uint8_t alarmIdx;
      Alarm   alarm;
    } alarm;
  } payload;
  uint8_t mac[8];
} Command;

void comm_init();

// Waits for a command to be received returns the command type
void comm_wait(Command *cmd);

// Receives a complete command
uint8_t comm_get(Command *cmd);

void comm_send_ok();
void comm_send_value(uint16_t value);
void comm_send_time(DateTime *datetime);
void comm_send_alarm(Alarm *alarm);

#endif // __LAMPE_COMMUNICACTION_H__
