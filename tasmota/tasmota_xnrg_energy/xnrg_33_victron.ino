/*
  xnrg_31_victron.ino - Victron Products Ve.direct support for Tasmota

  Copyright (C) 2022 by Michael Erhart

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef USE_ENERGY_SENSOR
#ifdef USE_VICTRON
/*********************************************************************************************\
 * Victron
 * https://www.atakale.com.tr/image/catalog/urunler/charger/victron/pdf/victron_energy_haberlesme_protokolu_VE.Direct-Protocol-3.29.pdf
\*********************************************************************************************/

#define XNRG_33            33

#ifndef VICTRON_SPEED
#define VICTRON_SPEED      19200    // default victron serial speed
#endif

#define D_VICTRON                    "Victron"

#define D_CMND_VICTRON_READ_HISTORY  "ReadHistory"
#define D_CMND_VICTRON_PING          "Ping"

const char kVictronCommands[] PROGMEM = "Victron|"      //prefix
  D_CMND_VICTRON_READ_HISTORY "|" D_CMND_VICTRON_PING;

void(* const VictronCommands[])(void) PROGMEM = {
  &CmndReadHistory, &CmndVictronPing
};

#include <TasmotaSerial.h>

struct PID2DeviceName {
  const char* name;
  int pid;
};

PID2DeviceName DeviceLookUp[] = {
  {"BMV-700" ,                    0x203},
  {"BMV-702" ,                    0x204},
  {"BMV-700H",                    0x205},
  {"BlueSolar MPPT 70|15",       0x0300},
  {"BlueSolar MPPT 75|50",       0xA040},
  {"BlueSolar MPPT 150|35",      0xA041},
  {"BlueSolar MPPT 75|15",       0xA042},
  {"BlueSolar MPPT 100|15",      0xA043},
  {"BlueSolar MPPT 100|30",      0xA044},
  {"BlueSolar MPPT 100|50",      0xA045},
  {"BlueSolar MPPT 150|70",      0xA046},
  {"BlueSolar MPPT 150|100",     0xA047},
  {"BlueSolar MPPT 100|50 rev2", 0xA049},
  {"BlueSolar MPPT 100|30 rev2", 0xA04A},
  {"BlueSolar MPPT 150|35 rev2", 0xA04B},
  {"BlueSolar MPPT 75|10",       0xA04C},
};
/*
  "BlueSolar MPPT 150|45 0xA04D
  "BlueSolar MPPT 150|60 0xA04E
  "BlueSolar MPPT 150|85 0xA04F
  "SmartSolar MPPT 250|100 0xA050
  "SmartSolar MPPT 150|100 0xA051
  "SmartSolar MPPT 150|85 0xA052
  "SmartSolar MPPT 75|15 0xA053
  "SmartSolar MPPT 75|10 0xA054
  "SmartSolar MPPT 100|15 0xA055
  "SmartSolar MPPT 100|30 0xA056
  "SmartSolar MPPT 100|50 0xA057
  "SmartSolar MPPT 150|35 0xA058
  "SmartSolar MPPT 150|100 rev2 0xA059
  "SmartSolar MPPT 150|85 rev2 0xA05A
  "SmartSolar MPPT 250|70 0xA05B
  "SmartSolar MPPT 250|85 0xA05C
  "SmartSolar MPPT 250|60 0xA05D
  "SmartSolar MPPT 250|45 0xA05E
  "SmartSolar MPPT 100|20 0xA05F
  "SmartSolar MPPT 100|20 48V 0xA060
  "SmartSolar MPPT 150|45 0xA061
  "SmartSolar MPPT 150|60 0xA062
  "SmartSolar MPPT 150|70 0xA063
  "SmartSolar MPPT 250|85 rev2 0xA064
  "SmartSolar MPPT 250|100 rev2 0xA065
  "BlueSolar MPPT 100|20 0xA06
  "BlueSolar MPPT 100|20 48V 0xA06
  "SmartSolar MPPT 250|60 rev2 0xA06
  "SmartSolar MPPT 250|70 rev2 0xA069
  "SmartSolar MPPT 150|45 rev2 0xA06
  "SmartSolar MPPT 150|60 rev2 0xA06
  "SmartSolar MPPT 150|70 rev2 0xA06
  "SmartSolar MPPT 150|85 rev3 0xA06
  "SmartSolar MPPT 150|100 rev3 0xA06
  "BlueSolar MPPT 150|45 rev2 0xA06
  "BlueSolar MPPT 150|60 rev2 0xA070
  "BlueSolar MPPT 150|70 rev2 0xA071
  "SmartSolar MPPT VE.Can 150/70 0xA102
  "SmartSolar MPPT VE.Can 150/45 0xA103
  "SmartSolar MPPT VE.Can 150/60 0xA104
  "SmartSolar MPPT VE.Can 150/85 0xA105
  "SmartSolar MPPT VE.Can 150/100 0xA106
  "SmartSolar MPPT VE.Can 250/45 0xA107
  "SmartSolar MPPT VE.Can 250/60 0xA108
  "SmartSolar MPPT VE.Can 250/70 0xA109
  "SmartSolar MPPT VE.Can 250/85 0xA10A
  "SmartSolar MPPT VE.Can 250/100 0xA10B
  "SmartSolar MPPT VE.Can 150/70 rev2 0xA10C
  "SmartSolar MPPT VE.Can 150/85 rev2 0xA10D
  "SmartSolar MPPT VE.Can 150/100 rev2 0xA10E
  "BlueSolar MPPT VE.Can 150/100 0xA10F
  "BlueSolar MPPT VE.Can 250/70 0xA112
  "BlueSolar MPPT VE.Can 250/100 0xA113
  "SmartSolar MPPT VE.Can 250/70 rev2 0xA114
  "SmartSolar MPPT VE.Can 250/100 rev2 0xA115
  "SmartSolar MPPT VE.Can 250/85 rev2 0xA116
  "Phoenix Inverter 12V 250VA 230V 0xA201
  "Phoenix Inverter 24V 250VA 230V 0xA202
  "Phoenix Inverter 48V 250VA 230V 0xA204
  "Phoenix Inverter 12V 375VA 230V 0xA211
  "Phoenix Inverter 24V 375VA 230V 0xA212
  "Phoenix Inverter 48V 375VA 230V 0xA214
  "Phoenix Inverter 12V 500VA 230V 0xA221
  "Phoenix Inverter 24V 500VA 230V 0xA222
  "Phoenix Inverter 48V 500VA 230V 0xA224
  "Phoenix Inverter 12V 250VA 230V 0xA231
  "Phoenix Inverter 24V 250VA 230V 0xA232
  "Phoenix Inverter 48V 250VA 230V 0xA234
  "Phoenix Inverter 12V 250VA 120V 0xA239
  "Phoenix Inverter 24V 250VA 120V 0xA23A
  "Phoenix Inverter 48V 250VA 120V 0xA23C
  "Phoenix Inverter 12V 375VA 230V 0xA241
  "Phoenix Inverter 24V 375VA 230V 0xA242
  "Phoenix Inverter 48V 375VA 230V 0xA244
  "Phoenix Inverter 12V 375VA 120V 0xA249
  "Phoenix Inverter 24V 375VA 120V 0xA24A
  "Phoenix Inverter 48V 375VA 120V 0xA24C
  "Phoenix Inverter 12V 500VA 230V 0xA251
  "Phoenix Inverter 24V 500VA 230V 0xA252
  "Phoenix Inverter 48V 500VA 230V 0xA254
  "Phoenix Inverter 12V 500VA 120V 0xA259
  "Phoenix Inverter 24V 500VA 120V 0xA25A
  "Phoenix Inverter 48V 500VA 120V 0xA25C
  "Phoenix Inverter 12V 800VA 230V 0xA261
  "Phoenix Inverter 24V 800VA 230V 0xA262
  "Phoenix Inverter 48V 800VA 230V 0xA264
  "Phoenix Inverter 12V 800VA 120V 0xA269
  "Phoenix Inverter 24V 800VA 120V 0xA26A
  "Phoenix Inverter 48V 800VA 120V 0xA26C
  "Phoenix Inverter 12V 1200VA 230V 0xA271
  "Phoenix Inverter 24V 1200VA 230V 0xA272
  "Phoenix Inverter 48V 1200VA 230V 0xA274
  "Phoenix Inverter 12V 1200VA 120V 0xA279
  "Phoenix Inverter 24V 1200VA 120V 0xA27A
  "Phoenix Inverter 48V 1200VA 120V 0xA27C
  "Phoenix Inverter 12V 1600VA 230V 0xA281
  "Phoenix Inverter 24V 1600VA 230V 0xA282
  "Phoenix Inverter 48V 1600VA 230V 0xA284
  "Phoenix Inverter 12V 2000VA 230V 0xA291
  "Phoenix Inverter 24V 2000VA 230V 0xA292
  "Phoenix Inverter 48V 2000VA 230V 0xA294
  "Phoenix Inverter 12V 3000VA 230V 0xA2A1
  "Phoenix Inverter 24V 3000VA 230V 0xA2A2
  "Phoenix Inverter 48V 3000VA 230V 0xA2A4
  "Phoenix Smart IP43 Charger 12|50 (1+1) 0xA340
  "Phoenix Smart IP43 Charger 12|50 (3) 0xA341
  "Phoenix Smart IP43 Charger 24|25 (1+1) 0xA342
  "Phoenix Smart IP43 Charger 24|25 (3) 0xA343
  "Phoenix Smart IP43 Charger 12|30 (1+1) 0xA344
  "Phoenix Smart IP43 Charger 12|30 (3) 0xA345
  "Phoenix Smart IP43 Charger 24|16 (1+1) 0xA346
  "Phoenix Smart IP43 Charger 24|16 (3) 0xA347
}
*/
const char* EnergyFieldNames[] = {
  "V",
  "I",
  "H20",
  "H22",
  "H18"
};

float* EnergyFields[] = {
  &Energy.voltage[0],
  &Energy.current[0],
  &Energy.daily_sum,
  &Energy.yesterday_sum,
  &Energy.total_sum
};

struct VictronField {
  bool valid;
  union {
    int int_value;
    float float_value;
    bool bool_value;
  };
};

enum VictronFieldType {
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_BOOL
};
struct VictronFieldDefinition {
  const char* label;
  const char* description;
  VictronFieldType type;
  int divider;
  const char* unit;

  bool try_parse(VictronField* field, const char* lbl, const char* value) {
    float *energyTmp = NULL;

    if (strcasecmp(label, lbl))
      return false;

    for(size_t i=0;i<(sizeof(EnergyFieldNames)/sizeof(EnergyFieldNames[0])); ++i){
      if(!strcasecmp(lbl, EnergyFieldNames[i])){
        energyTmp = EnergyFields[i];
      }
    }
    switch (type) {
      case TYPE_INT:
        field->int_value = strtol(value, NULL, 0);
        break;
      case TYPE_FLOAT:
        field->float_value = atof(value);
        field->float_value /= divider;
        if(energyTmp){
          *energyTmp = field->float_value;
          //AddLog(LOG_LEVEL_DEBUG, PSTR("Energy field: %s = %f"),lbl, *energyTmp);
        }
        break;
      case TYPE_BOOL:
        field->bool_value = !strcmp(value, "ON");
        break;
    }
    field->valid = true;
    return true;
  }

  bool output(VictronField* field, bool json, bool isFirstField) {
    const char* jsonFirst = "";
    const char* jsonAppend = ",";
    const char* jsonString = jsonAppend;
    if (!field->valid)
      return false;
    
    if(json){
      if (!isFirstField){
        ResponseAppend_P(PSTR(",\"Victron\":{"));
        jsonString = jsonFirst;
      }
      switch(type){
        case TYPE_INT:
          ResponseAppend_P(PSTR("%s\"%s\":%d"),jsonString, label, field->int_value);
          break;
        case TYPE_FLOAT:
          ResponseAppend_P(PSTR("%s\"%s\":%.2f"),jsonString,label, field->float_value);
          break;
        case TYPE_BOOL:
          ResponseAppend_P(PSTR("%s\"%s\":%d"),jsonString,label, field->bool_value ? 1 : 0);
          break;
      }
      return 1;
    }
    
    switch (type) {
      case TYPE_INT:
        WSContentSend_PD(PSTR("{s} %s {m}%d  %s {e}"), description, field->int_value, unit);
        break;
      case TYPE_FLOAT:
        WSContentSend_PD(PSTR("{s} %s {m}%.2f  %s {e}"), description, field->float_value, unit);
        break;
      case TYPE_BOOL:
        WSContentSend_PD(PSTR("{s} %s {m}%s  %s {e}"), description, field->bool_value ? "ON" : "OFF", unit);
        break;
    }
    return 0;
  }
};
#define D_BATTERY "Battery"
#define D_AUXILIARY "Auxiliary"
#define D_PANEL "Panel"       // As in photovoltaic panel

/*
#define D_BATTERY "Battery"       
#define D_BATTERY2 "Battery 2 Voltage"    
#define D_BATTERY2"Battery 3 Voltage"  
#define D_AUXILIARY_VOLTAGE "Auxiliary Voltage"                    
#define D_PANEL_VOLTAGE "Panel Voltage"       
#define D_PANEL_POWER "Panel Power"         
#define D_BATTERY_CURRENT "Battery Current"      
#define D_BATTERY_CURRENT" Battery 2 Current"     
#define D_BATTERY_CURRENT" Battery 3 Current"    
#define D_LOAD_CURRENT "Load Current"        
#define D_LOAD_STATE "Load State"            
#define D_BATTERY_TEMPERATURE "Battery Temperature"  
#define D_INSTANT_POWER "Instantaneous Power"   
#define D_CONSUMED_AMP_H "Consumed Amp Hours"    
#define D_SOC "State-Of-Charge"       
#define D_TIME_TO_GO "Time-To-Go"            
#define D_ALARM_CONDITION "Alarm Condition"        
#define D_RELAY_STATE "Relay State"           
#define D_ALARM_REASON "Alarm Reason"         
#define D_OFF_REASON "Off Reason"        
#define D_DEEPEST_DISCHARGE "Deepest Discharge"   
#define D_LAST_DISCHARGE_DEPTH "Last Discharge Depth" 
#define D_AVG_DEPTH "Average Discharge Depth"
#define D_CHARGE_CYCLES "Charge Cycles"        
#define D_FULL_DISCHARGES "Full Discharges"       
#define D_TOTAL_AH_DRAWN "Comulative Ah drawn"
#define D_MIN_BAT_VOLT "Min Battery Voltage"  
#define D_MAX_BAT_VOLT "Max Battery Voltage"
#define D_SECS_SINCE_FULL_CHARGE "Secs Since Full Charge",
#define D_AUTOMATIC_SYNCS "Automatic Syncs"     
#define D_LOW_VOLT_ALARMS "Low Voltage Alarms"   
#define D_HIGH_VOLT_ALARMS "High Voltage Alarms" 
#define D_LOW_AUX_VOLT_ALARM "Low Aux Voltage Alarms"
#define D_HIGH_AUX_VOLT_ALARM"High Aux Voltage Alarms"
#define D_MIN_AUX_VOLT"Min Aux Voltage"      
#define D_MAX_AUX_VOLT"Max Aux Voltage"      
#define D_DISCHARGED_ENERGY "Discharged Energy"   
#define D_CHARGED_ENERGY "Charged Energy"       
#define D_YIELD_TOTAL "Yield Total"           
#define D_YIELD_TODAY "Yield Today"           
#define D_MAX_POWER_TODAY "Max Power Today"     
#define D_YIELD_YESTERDAY "Yield Yesterday"      
#define D_MAX_POWER_YESTERDAY "Max Power Yesterday" 
#define D_ERROR_CODE "Error Code"           
#define D_STATE_OF_OPERATION "State Of Operation"    
#define D_FIRMWARE_VERSION "Firmware Version"      
#define D_PRODUCT_ID "Product ID"            
#define D_DAY_SEQUENCE "Day Sequence"          
#define D_DEVICE_MODE "Device Mode"      
#define D_AC_OUTPUT_VOLTAGE "AC Output Voltage"    
#define D_AC_OUTPUT_CURRENT "AC Output Current"   
#define D_AC_OUT_APPARENT_CURRENT "AC output app. Current"
#define D_WARNING_REASON "Warning Reason"       
#define D_TRACKER_OPERATION_MODE "Tracker Operation Mode"
*/

//https://www.atakale.com.tr/image/catalog/urunler/charger/victron/pdf/victron_energy_haberlesme_protokolu_VE.Direct-Protocol-3.29.pdf
VictronFieldDefinition victronFieldDefs[] = {
  {"V",         "Battery Voltage",            TYPE_FLOAT, 1000, D_UNIT_VOLT},
  {"V2",        "Battery 2 Voltage",          TYPE_FLOAT, 1000, D_UNIT_VOLT},                              //  mV Channel 2 (battery) voltage
  {"V3",        "Battery 3 Voltage",          TYPE_FLOAT, 1000, D_UNIT_VOLT},                              //  mV Channel 3 (battery) voltage
  {"VS",        "Auxiliary voltage",          TYPE_FLOAT, 1000, D_UNIT_VOLT},                              //  mV Auxiliary (starter) voltage • •
  {"VM",        "",                           TYPE_FLOAT, 1000, D_UNIT_VOLT},                              //  mV Mid-point voltage of the battery bank •
  {"DM",        "",                           TYPE_FLOAT,    1, "%"},                                      //  ‰ Mid-point deviation of the battery bank •
  {"VPV",       "Panel Voltage",              TYPE_FLOAT, 1000, D_UNIT_VOLT},                              //  mV Panel voltage •
  {"PPV",       "Panel Power",                TYPE_FLOAT,    1, D_UNIT_WATT},                              //  W Panel power
  {"I",         "Battery Current",            TYPE_FLOAT, 1000, D_UNIT_AMPERE},                            //  mA Main or channel 1 battery current • • •
  {"I2",        "Battery 2 Current",          TYPE_FLOAT, 1000, D_UNIT_AMPERE},                            // mA Channel 2 battery current •
  {"I3",        "Battery 3 Current",          TYPE_FLOAT, 1000, D_UNIT_AMPERE},                            // mA Channel 3 battery current •
  {"IL",        "Load Current",               TYPE_FLOAT, 1000, D_UNIT_AMPERE},                            //  mA Load current •
  {"LOAD",      "Load State",                 TYPE_BOOL,     1, ""},                                       //  Load output state (ON/OFF) •
  {"T",         "Battery Temperature",        TYPE_FLOAT,    1, D_UNIT_DEGREE D_UNIT_CELSIUS},             // °C Battery temperature •
  {"P",         "Instantaneous Power",        TYPE_FLOAT,    1, D_UNIT_WATT},                              // W Instantaneous power •
  {"CE",        "Consumed Amp Hours",         TYPE_FLOAT,    1, D_UNIT_MILLIAMPERE D_UNIT_HOUR},           // mAh7 Consumed Amp Hours • •
  {"SOC",       "State-Of-Charge",            TYPE_FLOAT,    1, "%"},                                      // ‰ State-of-charge • •
  {"TTG",       "Time-To-Go",                 TYPE_FLOAT,    1, D_UNIT_MINUTE},                            // Minutes Time-to-go • •
  {"Alarm",     "Alam Condition",             TYPE_BOOL,     1, ""},                                       // Alarm condition active • •
  {"Relay",     "Relay State",                TYPE_BOOL,     1, ""},                                       // Relay state • • •
  {"AR",        "Alarm Reason",               TYPE_INT,      1, ""},                              // Alarm reason • • •
  {"OR",        "Off Reason",                 TYPE_INT,      1, ""},                              // Off reason •
  {"H1",        "Deepest Discharge",          TYPE_FLOAT,    1, D_UNIT_MILLIAMPERE D_UNIT_HOUR},           // mAh Depth of the deepest discharge • •
  {"H2",        "Last Discharge Depth",       TYPE_FLOAT,    1, D_UNIT_MILLIAMPERE D_UNIT_HOUR},           // mAh Depth of the last discharge • •
  {"H3",        "Average Discharge Depth",    TYPE_FLOAT,    1, D_UNIT_MILLIAMPERE D_UNIT_HOUR},           // mAh Depth of the average discharge • •
  {"H4",        "Charge Cycles",              TYPE_INT,      1, ""},                                       // Number of charge cycles • •
  {"H5",        "Full Discharges",            TYPE_INT,      1, ""},                                       // Number of full discharges • •
  {"H6",        "Comulative Ah drawn",        TYPE_FLOAT, 1000, D_UNIT_MILLIAMPERE D_UNIT_HOUR},           // mAh Cumulative Amp Hours drawn • •
  {"H7",        "Min Battery Voltage",        TYPE_FLOAT, 1000, D_UNIT_VOLT},                              // mV Minimum main (battery) voltage • •
  {"H8",        "Max Battery Voltage",        TYPE_FLOAT, 1000, D_UNIT_VOLT},                              // mV Maximum main (battery) voltage • •
  {"H9",        "Secs Since Full Charge",     TYPE_INT,      1, ""},                                       // Seconds Number of seconds since last full charge • •
  {"H10",       "Automatic Syncs",            TYPE_INT,      1, ""},                                       // Number of automatic synchronizations • •
  {"H11",       "Low Voltage Alarms",         TYPE_INT,      1, ""},                                       // Number of low main voltage alarms • •
  {"H12",       "High Voltage Alarms",        TYPE_INT,      1, ""},                                       // Number of high main voltage alarms • •
  {"H13",       "Low Aux Voltage Alarms",     TYPE_INT,      1, ""},                                       // Number of low auxiliary voltage alarms •
  {"H14",       "High Aux Voltage Alarms",    TYPE_INT,      1, ""},                                       // Number of high auxiliary voltage alarms •
  {"H15",       "Min Aux Voltage",            TYPE_FLOAT, 1000, D_UNIT_VOLT},                              // mV Minimum auxiliary (battery) voltage • •
  {"H16",       "Max Aux Voltage",            TYPE_FLOAT, 1000, D_UNIT_VOLT},                              // mV Maximum auxiliary (battery) voltage • •
  {"H17",       "Discharged Energy",          TYPE_FLOAT,  100, D_UNIT_WATT D_UNIT_HOUR},                  // 0.01 kWh Amount of discharged energy •
  {"H18",       "Charged Energy",             TYPE_FLOAT,  100, D_UNIT_WATT D_UNIT_HOUR},                  // 0.01 kWh Amount of charged energy •
  {"H19",       "Yield Total",                TYPE_FLOAT,  100, D_UNIT_WATT D_UNIT_HOUR},                  // 0.01 kWh Yield total (user resettable counter) •
  {"H20",       "Yield Today",                TYPE_FLOAT,  100, D_UNIT_WATT D_UNIT_HOUR},                  // 0.01 kWh Yield today •
  {"H21",       "Max Power Today",            TYPE_INT,      1, D_UNIT_WATT},                              // W Maximum power today •
  {"H22",       "Yield Yesterday",            TYPE_FLOAT,  100, D_UNIT_WATT D_UNIT_HOUR},                  // 0.01 kWh Yield yesterday •
  {"H23",       "Max Power Yesterday",        TYPE_INT,      1, D_UNIT_WATT},                              // W Maximum power yesterday •
  {"ERR",       "Error Code",                 TYPE_INT,      1, ""},                                       // Error code • •
  {"CS",        "State Of Operation",         TYPE_INT,      1, ""},                                       // State of operation • • •
  {"FW",        "Firmware Version",           TYPE_INT,      1, ""},                                       // Firmware version (16 bit)
  {"FWE",       "Firmware Version",           TYPE_INT,      1, ""},                                       // Firmware version (24 bit) •
  {"PID",       "Product ID",                 TYPE_INT,      1, ""},                                       // Product ID • • • •
  {"HSDS",      "Day Sequence",               TYPE_INT,      1, ""},                                       // Day sequence number (0..364) 
  {"MODE",      "Device Mode",                TYPE_INT,      1, ""},                                       // Device mode • •
  {"AC_OUT_V",  "AC Output Voltage",          TYPE_FLOAT,  100, D_UNIT_VOLT},                              // 0.01 V AC output voltage •
  {"AC_OUT_I",  "AC Output Current",          TYPE_FLOAT,   10, D_UNIT_AMPERE},                            // 0.1 A AC output current •
  {"AC_OUT_S",  "AC output app. Current",     TYPE_INT,      1, D_UNIT_VOLT D_UNIT_AMPERE},                  // VA AC output apparent power •
  {"WARN",      "Warning Reason",             TYPE_INT,      1, ""},                                       // Warning reason •
  {"MPPT",      "Tracker Operation Mode",     TYPE_INT,      1, ""},                                       // racker operation mode
};            

VictronField victronFields[sizeof(victronFieldDefs) / sizeof(victronFieldDefs[0])];

TasmotaSerial *victronSerial;

/*********************************************************************************************/

uint8_t victron_calculateCRC(uint8_t *message, uint8_t len)
{
  uint8_t checksum = 0;
  for (uint8_t i = 0; i < len; i++) {
    checksum -= message[i];
  }
  return checksum;
}

void victron_parse_live_data(uint8_t *ReadBuffer, int len) {
	// Read VE.Direct lines from the serial port and parse it.
  uint8_t idx = 0;
  char line[30] = "\0";	
  char* label;
  char* value_str;
  
  for(size_t i=0; i<len; ++i){
	  uint8_t b = ReadBuffer[i]; 
	  //if (!((b == -1) || (b == '\r'))) { 	// Ignore '\r' and empty reads
		if (!(b == '\r')) { 	// Ignore '\r' and empty reads
    	if (!(b == '\n')) { 				      // EOL
				line[idx++] = b;		            // Add it to the buffer 
			}
      else{
        line[idx] = '\0';						    // Terminate the string
        label = strtok(line, "\t");
	      if (!(label == NULL)) {
          value_str = strtok(NULL, "\t");
          AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("Label: %s -> Value: %s"), label, value_str);
          for (size_t i = 0; i < (sizeof(victronFieldDefs) / sizeof(victronFieldDefs[0])); ++i) {
            if (!strcasecmp(label, victronFieldDefs[i].label)){
              victronFieldDefs[i].try_parse(&victronFields[i], label, value_str);
            }
          }
          line[0] = '\0';
	        idx = 0;
        }
	    }
    }
    else continue;
  }
	return;
}

void readHistory(void){
  uint8_t buffer[256] = {0};
  uint8_t cmd[16] = {':','7', '5', '0', '1', '0', 'E', 'E', '\n'};
  uint8_t response[128] = {0};
  int len = 0;
  bool flag = false;
  int idx = 0;
  Serial.println("Reading history:");
  Serial2.flush();
  Serial2.write(cmd, sizeof(cmd));
  delay(1);
  Serial2.readBytes(buffer, sizeof(buffer));
  Serial.print((char*)buffer);
  //Serial2.readBytesUntil(':', buffer, sizeof(buffer));
  //memset(&buffer, 0, sizeof(buffer));
  //Serial2.readBytesUntil('\n', buffer, sizeof(buffer));
  for(size_t i=0; i<sizeof(buffer); ++i){
    //if((buffer[i] == ':') && buffer[i+1] == '7'){
    if(buffer[i] == ':'){
      flag = true;
      Serial.println("Found response");
    }
    if(flag){
      Serial.write(buffer[i]);
      response[idx++] = buffer[i];
    }
    if(buffer[i] == '\n'){
      flag = false;
    }
  }
  Serial.println((char*)response);
}

uint8_t victron_Receive(uint8_t *ReadBuffer){
  uint8_t len = 0;
  while (victronSerial->available() > 0) {
    ReadBuffer[len++] = (uint8_t)victronSerial->read();
  }
  
  uint8_t crc = victron_calculateCRC(ReadBuffer, len);      // calculate checksum byte
  
  if (!crc){                                                // return number of bytes received on correct checksum
    return len;
  }
  return false;
}

void victron_EverySecond(void) {
  uint8_t DataRead[256] = {0};
  VictronFieldDefinition *field;
  uint8_t bytesReceived = victron_Receive(DataRead);
  if(bytesReceived){
    victron_parse_live_data(DataRead, bytesReceived);
    Energy.phase_count = 1;
    Energy.voltage_common = true;
    Energy.data_valid[0] = 0;
    Energy.active_power[0] = Energy.voltage[0] * abs(Energy.current[0]);
    //Energy.current[0] =      1.78; // AC Current
    //Energy.voltage[0] =      14.2; // AC Voltage
    //Energy.frequency[0] =    50; // AC Frequency
    //Energy.active_power[0] = 20; // AC Power
    Energy.type_dc = true;
    //AddLog(LOG_LEVEL_DEBUG, PSTR("Energy Volts: %f | Energy current: %f"),Energy.voltage[0], Energy.current[0]);
  }

  //EnergyUpdateToday();
  
}


void victron_DrvInit(void) 
{
  //Energy.phase_count = (TasmotaGlobal.devices_present < ENERGY_MAX_PHASES) ? TasmotaGlobal.devices_present : ENERGY_MAX_PHASES;
  //Energy.voltage_common = NRG_DUMMY_U_COMMON;    // Phase voltage = false, Common voltage = true
  //Energy.frequency_common = NRG_DUMMY_F_COMMON;  // Phase frequency = false, Common frequency = true
  //Energy.type_dc = NRG_DUMMY_DC;                 // AC = false, DC = true;
  //Energy.use_overtemp = NRG_DUMMY_OVERTEMP;      // Use global temperature for overtemp detection
  if (PinUsed(GPIO_VICTRON_RX) && PinUsed(GPIO_VICTRON_TX)) {
    TasmotaGlobal.energy_driver = XNRG_33;
  }
  //TasmotaGlobal.energy_driver = XNRG_33;
  AddLog(LOG_LEVEL_DEBUG, PSTR("Victron DrvInit"));
}

void victron_SnsInit(void)
{
  AddLog(LOG_LEVEL_INFO, PSTR("SX1: Init - RX-pin: %d, TX-pin: %d"), Pin(GPIO_VICTRON_RX), Pin(GPIO_VICTRON_TX));
  victronSerial = new TasmotaSerial(Pin(GPIO_VICTRON_RX), Pin(GPIO_VICTRON_TX), 1);
  if (victronSerial->begin(VICTRON_SPEED)) {
    if (victronSerial->hardwareSerial()) { ClaimSerial(); }
  } 
  else {
    TasmotaGlobal.energy_driver = ENERGY_NONE;
    AddLog(LOG_LEVEL_INFO, "VICTRON SnsInit failed");
  }
  //TasmotaGlobal.energy_driver = XNRG_33;
  AddLog(LOG_LEVEL_INFO, "VICTRON SnsInit");
}

void victron_Show(bool json){
  int jsonElements = 0;
  for (size_t i = 0; i < (sizeof(victronFieldDefs) / sizeof(victronFieldDefs[0])); ++i) {
    jsonElements += victronFieldDefs[i].output(&victronFields[i], json, jsonElements);
  }
  //AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("Json Elements appended: %d"), jsonElements);
  if(json){
    ResponseAppend_P(PSTR("}"));
  }
}
/*********************************************************************************************\
 * Commands
\*********************************************************************************************/

void CmndReadHistory(void){
  if (!strcasecmp(XdrvMailbox.data, "ReadHistory")) {
    AddLog(LOG_LEVEL_DEBUG, PSTR("Victron Command Received"));
    //return true;
  }
  AddLog(LOG_LEVEL_DEBUG, PSTR("In Victron CMD"));
  return;
}

void CmndVictronPing(void){
  AddLog(LOG_LEVEL_DEBUG, PSTR("In Victron CMD"));
  return;
}
/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xnrg33(uint8_t function)
{
  bool result = false;

  switch (function) {
    case FUNC_EVERY_SECOND:
      victron_EverySecond();
      break;
    case FUNC_JSON_APPEND:
      victron_Show(1);
      break;
#ifdef USE_WEBSERVER
    case FUNC_WEB_SENSOR:
      victron_Show(0);
      break;
#endif  // USE_WEBSERVER
    case FUNC_INIT:
      victron_SnsInit();
      break;
    case FUNC_PRE_INIT:
      victron_DrvInit();
      break;
    case FUNC_COMMAND:
      AddLog(LOG_LEVEL_DEBUG, PSTR("Victron FUNC CMD"));
      result = DecodeCommand(kVictronCommands, VictronCommands);
      break;
  }
  return result;
}

#endif  // USE_VICTRON_NRG
#endif  // USE_ENERGY_SENSOR
