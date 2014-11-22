#include <EEPROM.h>

#include <Time.h>
#include <TimeAlarms.h>

#define ledPin  3
#define Port Serial1


typedef struct {
  byte state;    // 0 - wait, 1 - start dimming, 2 - dimming
  int value;
  unsigned long dimDelay;
  unsigned long nextDim;
  byte maxDim;
  byte dimDur;  
} State;

static State current_state;

typedef struct {
  byte flags;
  char hour;
  char minute;
} AlarmCfg;

static AlarmCfg alarm_cfg[dtNBR_ALARMS];


void setup() {
  // BT serial
  Port.begin(9600);
  // Switch LED off
  analogWrite(ledPin, 0);
  
  // BUG: Read time & date from RTC
  readRTC();
  // Read state and alarms from eeprom
  readState();
  // Read alarm settings & config timers
  readAlarm();
  updateAlarm();
}


void loop() 
{
  unsigned long current_millis = millis();
  // receive commend (if there are any)
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
}


// Gets called on alarm
void alarm_callback() {
  current_state.state = 1;
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


void readAlarm() {
  int idx = 2;
  // Iterate over all possible alarms
  for (int i=0; i<dtNBR_ALARMS; i++) {
    // LSB specifies if alarm is enabled
    // (flags>>1) specifies day of week 
    // (All=0, Sun=1, Mon=2, Tue=3, Wen=4, Thu=5, Fri=6, Sat=7)
    alarm_cfg[i].flags  = EEPROM.read(idx); idx++;
    alarm_cfg[i].hour   = EEPROM.read(idx); idx++;
    alarm_cfg[i].minute = EEPROM.read(idx); idx++;
  }
}

void writeAlarm() {
  int idx = 2;
  // Iterate over all possible alarms
  for (int i=0; i<dtNBR_ALARMS; i++) {
    EEPROM.write(idx, alarm_cfg[i].flags); idx++;
    EEPROM.write(idx, alarm_cfg[i].hour); idx++;
    EEPROM.write(idx, alarm_cfg[i].minute); idx++;
  }
}

void updateAlarm() {
  for (int i=0; i<dtNBR_ALARMS; i++) 
  {
    // Reset alarm
    Alarm.free(i);
    // Create one if enabled
    if (alarm_cfg[i].flags & 1) { 
      timeDayOfWeek_t dow = timeDayOfWeek_t(alarm_cfg[i].flags>>1);
      if (dowInvalid) { // Invalid means every day
        Alarm.alarmRepeat(
          alarm_cfg[i].hour, alarm_cfg[i].minute, 0, alarm_callback);
      } else { // otherwise a specific day of week
        Alarm.alarmRepeat(
          dow, alarm_cfg[i].hour, alarm_cfg[i].minute, 0, alarm_callback);
      }
    }
  }
}


void readRTC() {
  // BUG READ RTC 
  setTime(8,29,0,1,1,14);
}

void setRTCTime(int year, int month, int day, int h, int m, int s) {
  // BUG set RTC
  setTime(year, month, day, h, m, s);
}




void processCommand() 
{
  if (! Port.available())  { return; }

  // Read command from BT serial:
  String line = String(Port.readStringUntil('\n'));
    
  // Dispatch command
  if (line == "ON") {
    current_state.value = 255;
    current_state.state = 0;
    Port.println("OK");
    return;
  } 
    
  if (line == "OFF") {
    current_state.value = 0;
    current_state.state = 0;
    Port.println("OK");
    return;
  } 
    
  if (line == "DIM") {
    current_state.value = 0;
    current_state.state = 1;
    return;
  } 
    
  if (line.startsWith("SETLED")) {
    long value = line.substring(7).toInt();
    if ((value < 0) || (value > 255)) {
      Port.println("ERR"); return;
    }
    current_state.value = value;
    Port.println("OK");
    return;
  }
   
  if (line.startsWith("SETMAX")) {
    long value = line.substring(7).toInt();
    if ((value < 1) || (value > 255)) {
      Port.println("ERR"); return;
    }   
    // Store max dim-value into eeprom
    current_state.maxDim = value;
    writeState();
    Port.println("OK");
    return;
  } 
    
  if (line.startsWith("SETDUR")) {
    long value = line.substring(7).toInt();
    if ((value < 1) || (value > 120)) {
      Port.println("ERR"); return;
    }
    // Store dim duration into eeprom
    current_state.dimDur = value;
    writeState();        
    Port.println("OK");
    return;
  }
  
  if (line == "NALARM") {
    // Returns the number of possible alarms
    Port.println(dtNBR_ALARMS);
    return;
  } 
  
  if (line.startsWith("ALARM")) {
    int idx = line.substring(6).toInt();
    if ((idx < 0) || (idx>=dtNBR_ALARMS)) {
      Port.println("ERR"); return;
    } 
    Port.print(int(alarm_cfg[idx].flags&1)); Port.print(" ");
    Port.print(int(alarm_cfg[idx].flags>>1)); Port.print(" ");
    Port.print(int(alarm_cfg[idx].hour)); Port.print(" ");
    Port.println(int(alarm_cfg[idx].minute)); 
    return;
  } 

  if (line.startsWith("SETALARM")) {
    line = line.substring(9);
      
    // Read alarm index
    int idx = line.toInt();
    if ((idx < 0) || (idx>=dtNBR_ALARMS) || (0==line.length())) {
      Port.println("ERR idx"); return;
    }
    line = line.substring(line.indexOf(' ')+1);
    
    // Read enabled flag
    int enabled = line.toInt();
    line = line.substring(line.indexOf(' ')+1);
      
    // Read day of week
    int dow = line.toInt();
    if ((dow<0) || (dow>7) || (0==line.length())) {
      Port.println("ERR"); return;
    }
    line = line.substring(line.indexOf(' ')+1);
      
    // Read hour
    int hour = line.toInt();
    if ((hour<0) || (hour>23) || (0==line.length())) {
      Port.println("ERR"); return;
    }
    line = line.substring(line.indexOf(' ')+1);
      
    // Read minute
    int minute = line.toInt();
    if ((minute<0) || (minute>59) || (0==line.length())) {
      Port.println("ERR"); return;
    }
      
    // Assemble and store alarm config
    alarm_cfg[idx].flags = (dow<<1) | (0 != enabled);
    alarm_cfg[idx].hour  = hour;
    alarm_cfg[idx].minute = minute;
    writeAlarm();
    updateAlarm();
    return;
  } 

  if (line == "TIME") {
    Port.print(year()); Port.print("-");
    Port.print(month()); Port.print("-");
    Port.print(day()); Port.print(" ");
    Port.print(hour()); Port.print(":");
    Port.print(minute()); Port.print(":");
    Port.println(second());
    return;
  }
  
  if (line == "STAT") {
    Port.print("State "); Port.println(current_state.state);
    Port.print("Value "); Port.println(current_state.value);
    Port.print("Dim max "); Port.println(current_state.maxDim); 
    Port.print("Dim dur "); Port.println(current_state.dimDur); 
    Port.print("Dim delay "); Port.println(current_state.dimDelay);
    return;
  } 
  
  Port.println("ERR");
}
