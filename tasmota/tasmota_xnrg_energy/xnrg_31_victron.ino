/*
  xnrg_12_solaxX1.ino - Solax X1 inverter RS485 support for Tasmota

  Copyright (C) 2021 by Pablo Zerón
  Copyright (C) 2022 by Stefan Wershoven

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
#ifdef USE_VICTRON_BLUE_SOLAR
/*********************************************************************************************\
 * Victron
 * https://www.atakale.com.tr/image/catalog/urunler/charger/victron/pdf/victron_energy_haberlesme_protokolu_VE.Direct-Protocol-3.29.pdf
\*********************************************************************************************/

#define XNRG_33            33

#ifndef VICTRON_SPEED
#define VICTRON_SPEED      19200    // default victron rs485 speed
#endif

#define D_VICTRON_BLUE_SOLAR         "Victron"

#include <TasmotaSerial.h>


struct {
  int32_t current[3] = { 0 };
  int32_t power[3] = { 0 };
} victronValues;

//std::optional
struct optional{
}

struct blueSolar {
  uint32_t  PID       = 0;            // Product ID for BlueSolar MPPT 
  uint16_t  FW        = 0;            // Firmware version of controller
  uint8_t   SER[16]  = {0};          // Serial number
  float     V         = 0;            // Battery Voltage in Volts
  float     I         = 0;            // Battery current in Amps
  float     VPV       = 0;            // Panel voltage in Volts
  float     PPV       = 0;            // Panel power in Watts
  uint8_t   CS        = 0;            // Charge state from 0-9 see ....
  uint8_t   MPPT      = 0;            // MPPT operation mode
  uint8_t   ERR       = 0;            // Error Code
  uint8_t   LOAD      = 0;            // Load output ON or OFF
  float     IL        = 0;            // Load current in A
  uint16_t  H19       = 0;            // Yield total in kWh
  uint16_t  H20       = 0;            // Yield totay in kWh
  uint16_t  H21       = 0;            // Maximum Power today in Watts
  uint16_t  H22       = 0;            // Yield yesterday in kWh
  uint16_t  H23       = 0;            // Day sequence number from 0 - 365
  uint8_t   CRC       = 0;            // Checksum
} blueSolarValues;

void parse_integral(const char * val, void* field){
  *((int*)field) = atoi(val);
}
void parse_pid(const char * val, void* field){
  strcpy((char*)field, val);
  strcpy(blueSolarValues)
}
struct {
  char label[16];
  void (*parse)(const char*, void*);
  void* field;
} fields[] = {
  {"PID", parse_integral, &blueSolarValues.PID},
  {"PID", parse_pid, &blueSolarValues.PID},
};

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

int victron_parse_live_data(uint8_t *ReadBuffer, int len) {
	// Read VE.Direct lines from the serial port
	// Search for the label specified by enum target
	// Extract and return the corresponding value
	// If value is "ON" return 1. If "OFF" return 0;
  uint8_t idx = 0;
  char line[30] = "\0";	
  char* label;
  char* value_str;
  int ret = 0;
  
  for(uint8_t i=0; i<len; ++i){
	  uint8_t b = ReadBuffer[i]; 
	  if (!((b == -1) || (b == '\r'))) { 	// Ignore '\r' and empty reads
			if (!(b == '\n')) { 				// EOL
				line[idx++] = b;		// Add it to the buffer 
			}
      else{
        line[idx] = '\0';						// Terminate the string
        label = strtok(line, "\t");
        //AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("Line: %s"), line);
	      if (!(label == NULL)) {
          for (size_t i = 0; i < (sizeof(fields) / sizeof(fields[0])); ++i) {
            if (strcmp(label, fields[i].label))
              continue;
            	
            value_str = strtok(NULL, "\t");
            fields[i].parse(value_str, fields[i].field);
          }
          line[0] = '\0';
	        idx = 0;
        }
	    }
    }
    else continue;
  }
	return ret;
}

uint8_t victron_Receive(uint8_t *ReadBuffer)
{
  uint8_t len = 0;
  while (victronSerial->available() > 0) {
    ReadBuffer[len++] = (uint8_t)victronSerial->read();
  }
  //AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("Message: %s\nCharacters Read: %d"), ReadBuffer, len);
  
  uint8_t crc = victron_calculateCRC(ReadBuffer, len);      // calculate out crc bytes
  AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("CRC: %d"), crc);
  
  //if (!crc){
  //  return len;
  //}
  //return false;
  //AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("Len: %d"), len);
  return len;
}


void victron_EverySecond(void) {
  uint8_t DataRead[256] = {0};
  uint8_t bytesReceived = victron_Receive(DataRead);
  //AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("Message: %s\nCharacters Read: %d"), DataRead, bytesReceived);
  if(bytesReceived){
    blueSolarValues.V = victron_parse_live_data(DataRead, bytesReceived, "V") / 1000.0;
    blueSolarValues.I = victron_parse_live_data(DataRead, bytesReceived, "I") / 1000.0;
    blueSolarValues.VPV = victron_parse_live_data(DataRead, bytesReceived, "VPV");
    blueSolarValues.PPV = victron_parse_live_data(DataRead, bytesReceived, "PPV");
  }

  AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("Voltage: %f"), blueSolarValues.V);
    
    //EnergyUpdateToday();
  
}


void victron_DrvInit(void) 
{
  //Energy.phase_count = (TasmotaGlobal.devices_present < ENERGY_MAX_PHASES) ? TasmotaGlobal.devices_present : ENERGY_MAX_PHASES;
  //Energy.voltage_common = NRG_DUMMY_U_COMMON;    // Phase voltage = false, Common voltage = true
  //Energy.frequency_common = NRG_DUMMY_F_COMMON;  // Phase frequency = false, Common frequency = true
  //Energy.type_dc = NRG_DUMMY_DC;                 // AC = false, DC = true;
  //Energy.use_overtemp = NRG_DUMMY_OVERTEMP;      // Use global temperature for overtemp detection
  TasmotaGlobal.energy_driver = XNRG_33;

  AddLog(LOG_LEVEL_DEBUG, PSTR("Victron DrvInit"));
}

void victron_SnsInit(void)
{
  AddLog(LOG_LEVEL_INFO, PSTR("SX1: Init - RX-pin: %d, TX-pin: %d"), Pin(GPIO_VICTRON_RX), Pin(GPIO_VICTRON_TX));
  victronSerial = new TasmotaSerial(Pin(GPIO_VICTRON_RX), Pin(GPIO_VICTRON_TX), 1);
  if (victronSerial->begin(VICTRON_SPEED)) {
    if (victronSerial->hardwareSerial()) { ClaimSerial(); }
  } else {
    TasmotaGlobal.energy_driver = ENERGY_NONE;
  }
  //TasmotaGlobal.energy_driver = XNRG_33;
  AddLog(LOG_LEVEL_INFO, "VICTRON SnsInit");
}

//#ifdef USE_WEBSERVER
  const char HTTP_SNS_victron_DATA1[] PROGMEM =
    "{s}" D_VICTRON_BLUE_SOLAR " " D_SOLAR_POWER "{m}%s " D_UNIT_WATT "{e}"
    "{s}" D_VICTRON_BLUE_SOLAR " " D_PV1_VOLTAGE "{m}%s " D_UNIT_VOLT "{e}"
    "{s}" D_VICTRON_BLUE_SOLAR " " D_PV1_CURRENT "{m}%s " D_UNIT_AMPERE "{e}"
    "{s}" D_VICTRON_BLUE_SOLAR " " D_PV1_POWER "{m}%s " D_UNIT_WATT "{e}";
//#endif

void victron_Show(bool json){
  char solar_power[33];
  dtostrfd(blueSolarValues.V, Settings->flag2.wattage_resolution, solar_power);
  char pv1_voltage[33];
  dtostrfd(blueSolarValues.V, Settings->flag2.energy_resolution, pv1_voltage);
  char pv1_current[33];
  dtostrfd((blueSolarValues.PPV / blueSolarValues.VPV), Settings->flag2.current_resolution, pv1_current);
  char pv1_power[33];
  dtostrfd(105, Settings->flag2.wattage_resolution, pv1_power);

  WSContentSend_PD(HTTP_SNS_victron_DATA1, solar_power, pv1_voltage, pv1_current, pv1_power);
  //AddLog(LOG_LEVEL_DEBUG, PSTR("Victron Show"));
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
  }
  return result;
}

#endif  // USE_VICTRON_BLUE_SOLAR_NRG
#endif  // USE_ENERGY_SENSOR
