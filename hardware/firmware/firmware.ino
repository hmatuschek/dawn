#include <EEPROM.h>

#include <Time.h>
#include <TimeAlarms.h>
#include "siphash24.h"

#define ledPin  3
#define Port Serial1

#define N_ALARM (dtNBR_ALARMS-1)

#define CMD_GET_SALT      0x00
#define CMD_GET_VALUE     0x01
#define CMD_SET_VALUE     0x02
#define CMD_GET_TIME      0x03
#define CMD_SET_TIME      0x04
#define CMD_GET_N_ALARMS  0x05
#define CMD_GET_ALARM     0x06
#define CMD_SET_ALARM     0x07
#define CMD_GET_MAX_DIM   0x08
#define CMD_SET_MAX_DIM   0x09
#define CMD_GET_DIM_DUR   0x0A
#define CMD_SET_DIM_DUR   0x0B

#define RES_OK            0x00
#define RES_CMD_UNKNOWN   0x01
#define RES_SIG_INVALID   0x02
#define RES_CMD_ERROR     0x03


typedef struct {
  // 0 - wait, 1 - start dimming, 2 - dimming
  byte state;    
  // [0,255] Current value
  int value;     
  // Delay between dim-increments in ms
  unsigned long dimDelay;
  // Next tick for dim-increment
  unsigned long nextDim;
  // Max value for dimming
  byte maxDim;
  // Duration of dimming [0, maxDim] in sec.
  byte dimDur;  
  // The current salt value
  byte salt[8];
  // The shared secret
  byte secret[16];
} State;

// The state variable
static State current_state;

// An alarm configuration
typedef struct {
  // Like enabled, day-of-week
  byte flags;
  // The alarm hour
  char hour;
  // The alarm minute
  char minute;
} AlarmCfg;

// The vector of alarm configurations
static AlarmCfg alarm_cfg[N_ALARM];


void setup() {
  // BT serial
  Port.begin(9600);
  // Switch LED off
  analogWrite(ledPin, 0);
  
  // Read time & date from RTC
  readRTC();
  // Read state from EEPROM
  readState();
  // Read salt
  readSalt();
  // Read secret
  readSecret();
  // Read alarm settings from EEPROM
  readAlarm();
  // config timers
  updateAlarm();
}


void loop() 
{
  unsigned long current_millis = millis();
  // receive command (if there is any)
  processCommand();
  
  // If start dimming:
  if (1 == current_state.state) { 
    current_state.value = 0;
    current_state.state = 2;
    current_state.nextDim = millis() + current_state.dimDelay;
  } else if ((2 == current_state.state) && (current_millis >= current_state.nextDim)){
    current_state.value++;
    current_state.nextDim += current_state.dimDelay;
    if (current_state.value >= current_state.maxDim) { 
      current_state.state = 0;
    }
  }
  
  analogWrite(ledPin, current_state.value);
  
  // trigger alarm routines
  Alarm.delay(0);
}


// Gets called on alarm
void alarm_callback() {
  if (0 == current_state.state) {
    current_state.state = 1;
  }
}



void readState() {
  current_state.state = 0;
  current_state.value = 0;
  current_state.maxDim = EEPROM.read(0);
  current_state.dimDur = EEPROM.read(1);
  current_state.dimDelay = (unsigned long)(current_state.dimDur*60000UL)/
      ((unsigned long) current_state.maxDim);
  current_state.nextDim = 0;
}

void writeState() {
  // compute delay between dim increments (ms)
  current_state.dimDelay = (unsigned long)(current_state.dimDur*60000UL)/
      ((unsigned long) current_state.maxDim);
  // Store persistent values
  EEPROM.write(0, current_state.maxDim);
  EEPROM.write(1, current_state.dimDur);
}


void readSalt() {
  int idx = 2;
  for (int i=0; i<8; i++) { 
    current_state.salt[i]   = EEPROM.read(idx); idx++; 
  }
}

void writeSalt() {
  int idx = 2;
  for (int i=0; i<8; i++) { 
    EEPROM.write(idx, current_state.salt[i]);
  }
}


void readSecret() {
  int idx = 10;
  for (int i=0; i<16; i++) { 
    current_state.secret[i]   = EEPROM.read(idx); idx++; 
  }
}

void writeSecret() {
  int idx = 10;
  for (int i=0; i<16; i++) { 
    EEPROM.write(idx, current_state.secret[i]);
  }
}


void readAlarm() {
  int idx = 26;
  // Iterate over all possible alarms
  for (int i=0; i<N_ALARM; i++) {
    // LSB specifies if alarm is enabled
    // (flags>>1) specifies day of week 
    // (All=0, Sun=1, Mon=2, Tue=3, Wen=4, Thu=5, Fri=6, Sat=7)
    alarm_cfg[i].flags  = EEPROM.read(idx); idx++;
    alarm_cfg[i].hour   = EEPROM.read(idx); idx++;
    alarm_cfg[i].minute = EEPROM.read(idx); idx++;
  }
}

void writeAlarm() {
  int idx = 26;
  // Iterate over all possible alarms
  for (int i=0; i<N_ALARM; i++) {
    EEPROM.write(idx, alarm_cfg[i].flags); idx++;
    EEPROM.write(idx, alarm_cfg[i].hour); idx++;
    EEPROM.write(idx, alarm_cfg[i].minute); idx++;
  }
}

void updateAlarm() 
{
  // Reset all alarms
  for (int i=0; i<dtNBR_ALARMS; i++) { Alarm.free(i); }
  // Update user configurable alarms  
  for (int i=0; i<N_ALARM; i++) 
  {
    // Create one if enabled
    if (alarm_cfg[i].flags & 1) { 
      timeDayOfWeek_t dow = timeDayOfWeek_t(alarm_cfg[i].flags>>1);
      if (dowInvalid == dow) { // Invalid means every day
        Alarm.alarmRepeat(
          alarm_cfg[i].hour, alarm_cfg[i].minute, 0, alarm_callback);
      } else { // otherwise a specific day of week
        Alarm.alarmRepeat(
          dow, alarm_cfg[i].hour, alarm_cfg[i].minute, 0, alarm_callback);
      }
    }
  }
  // Register an internal alarm (timer) to update local clock from RTC every hour
  Alarm.timerRepeat(10, readRTC);
}


void readRTC() {
  // BUG READ RTC 
}

void setRTCTime(int year, int month, int day, int h, int m, int s) {
  // BUG SET RTC
  // Set date-time of local clock
  setTime(h, m, s, day, month, year);
}

int verify(byte *buffer, int len) {
  byte osig[8];
  memcpy(osig, current_state.salt, 8);
  siphash24_cbc_mac(osig, buffer, len, current_state.secret);
  int correct = 1;
  for (int i=0; i<8; i++) {
    correct &= (osig[i] == buffer[len+i]);
  }
  return correct;
} 


void sign(byte *buffer, int len) {
  memcpy(buffer+len, current_state.salt, 8);
  siphash24_cbc_mac(buffer+len, buffer, len, current_state.secret);
}

void advanceSalt(byte *sig) {
  memcpy(current_state.salt, sig, 8);
  writeSalt();
}


void processCommand() 
{
  if (! Port.available())  { return; }

  byte message[16];
  
  // Read command from BT serial:
  message[0] = Port.read();
  
  // Dispatch by command byte
  switch (message[0]) 
  {
  case CMD_GET_SALT:
    Port.write(current_state.salt, 8);
    break;
    
  case CMD_GET_VALUE:
    Port.readBytes((char *)message+1, 8);
    if (! verify(message, 1)) { goto SIG_ERROR; }
    advanceSalt(message+1);
    message[0] = RES_OK; message[1] = current_state.value;
    sign(message, 2); Port.write(message, 10);
    advanceSalt(message+2);
    break;
    
  case CMD_SET_VALUE:
    Port.readBytes((char *)message+1, 9);
    if (! verify(message, 2)) { goto SIG_ERROR; }
    current_state.value = message[1];
    advanceSalt(message+2);
    message[0] = RES_OK;
    sign(message, 1);  Port.write(message, 9);
    advanceSalt(message+1);
    break;
    
  case CMD_GET_TIME:
    Port.readBytes((char *)message+1, 8);
    if (! verify(message, 1)) { goto SIG_ERROR; }
    advanceSalt(message+1);
    message[0] = RES_OK;   message[1] = (year()>>8);
    message[2] = year();   message[3] = month();
    message[4] = day();    message[5] = hour();
    message[6] = minute(); message[7] = second();
    sign(message, 8); Port.write(message, 16);
    advanceSalt(message+8);
    break;
    
  case CMD_SET_TIME:
    Port.readBytes((char *)message+1, 15);
    if (! verify(message, 8)) { goto SIG_ERROR; }
    advanceSalt(message+8);
    setRTCTime((int(message[1])<<8)+message[2], message[3], message[4], 
               message[5], message[6], message[7]);
    message[0] = RES_OK;
    sign(message,1); Port.write(message, 9);
    advanceSalt(message+1);
    break;
    
  case CMD_GET_N_ALARMS:
    Port.readBytes((char *)message+1, 8);
    if (! verify(message, 1)) { goto SIG_ERROR; }
    advanceSalt(message+1);
    message[0] = RES_OK; message[1] = N_ALARM;
    sign(message, 2); Port.write(message, 10);
    advanceSalt(message+2);
    break;
    
  case CMD_GET_ALARM:
    Port.readBytes((char *)message+1, 9);
    if (! verify(message, 2)) { goto SIG_ERROR; }
    if (message[2] >= N_ALARM) { goto CMD_ERROR; }
    advanceSalt(message+2);
    { int idx = message[1];
      message[0] = RES_OK;              message[1] = alarm_cfg[idx].flags;
      message[2] = alarm_cfg[idx].hour; message[3] = alarm_cfg[idx].minute; }
    sign(message, 4); Port.write(message, 12);
    advanceSalt(message+4);
    break;
    
  case CMD_SET_ALARM:
    Port.readBytes((char *)message+1, 12);
    if (! verify(message, 5)) { goto SIG_ERROR; }
    if (message[1] >= N_ALARM) { goto CMD_ERROR; }
    advanceSalt(message+5);
    alarm_cfg[message[1]].flags  = message[2]; 
    alarm_cfg[message[1]].hour   = message[3]; 
    alarm_cfg[message[1]].minute = message[4]; 
    updateAlarm(); writeAlarm();
    message[0] = RES_OK;
    sign(message, 1); Port.write(message, 9);
    advanceSalt(message+1);
    break;
    
  case CMD_GET_MAX_DIM:
    Port.readBytes((char *)message+1, 8);
    if (! verify(message, 1)) { goto SIG_ERROR; }
    advanceSalt(message+1);
    message[0] = RES_OK; message[1] = current_state.maxDim;
    sign(message, 2); Port.write(message, 10);
    advanceSalt(message+2);
    break; 

  case CMD_SET_MAX_DIM:
    Port.readBytes((char *)message+1, 9);
    if (! verify(message, 2)) { goto SIG_ERROR; }
    advanceSalt(message+2);
    current_state.maxDim = message[1];
    writeState();
    message[0] = RES_OK; 
    sign(message, 1); Port.write(message, 9);
    advanceSalt(message+1);
    break;

  case CMD_GET_DIM_DUR:
    Port.readBytes((char *)message+1, 8);
    if (! verify(message, 1)) { goto SIG_ERROR; }
    advanceSalt(message+1);
    message[0] = RES_OK; message[1] = current_state.dimDur;
    sign(message, 2); Port.write(message, 10);
    advanceSalt(message+2);
    break;
    
  case CMD_SET_DIM_DUR:
    Port.readBytes((char *)message+1, 9);
    if (! verify(message, 2)) { goto SIG_ERROR; }
    advanceSalt(message+2);
    current_state.dimDur = message[1];
    writeState();
    message[0] = RES_OK; 
    sign(message, 1); Port.write(message, 9);
    advanceSalt(message+1);
    break;
    
  default:  
    goto CMD_UNKNOWN;
  }
 
  // OK
  return;
  
CMD_UNKNOWN:
  message[0] = RES_CMD_UNKNOWN;
  Port.write(message, 1);
  return;
  
SIG_ERROR:
  message[0] = RES_SIG_INVALID;
  Port.write(message, 1);
  return;
  
CMD_ERROR:
  message[0] = RES_CMD_ERROR;
  Port.write(message, 2);
}

