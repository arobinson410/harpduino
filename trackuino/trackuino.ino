/* trackuino copyright (C) 2010  EA5HAV Javi
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

// Mpide 22 fails to compile Arduino code because it stupidly defines ARDUINO 
// as an empty macro (hence the +0 hack). UNO32 builds are fine. Just use the
// real Arduino IDE for Arduino builds. Optionally complain to the Mpide
// authors to fix the broken macro.
#if (ARDUINO + 0) == 0
#error "Oops! We need the real Arduino IDE (version 22 or 23) for Arduino builds."
#error "See trackuino.pde for details on this"

// Refuse to compile on arduino version 21 or lower. 22 includes an 
// optimization of the USART code that is critical for real-time operation
// of the AVR code.
#elif (ARDUINO + 0) < 22
#error "Oops! We need Arduino 22 or 23"
#error "See trackuino.pde for details on this"

#endif


// Trackuino custom libs
#include "config.h"
#include "afsk_avr.h"
#include "afsk_pic32.h"
#include "aprs.h"
//#include "buzzer.h"
#include "gps.h"
#include "pin.h"
#include "power.h"
//#include "sensors_avr.h"
//#include "sensors_pic32.h"
//#include <SPI.h>
#include <SdFat.h>

// Arduino/AVR libs
#if (ARDUINO + 1) >= 100
#  include <Arduino.h>
#else
#  include <WProgram.h>
#endif

// Module constants
static const uint32_t VALID_POS_TIMEOUT = 2000;  // ms

// Module variables
static int32_t next_aprs = 0;

void data_log();

File data_log_file;
SdFat SD;

void setup()
{
  SD.begin(SD_CARD_CS);
  data_log_file = SD.open("data.txt", FILE_WRITE);
  
  //pinMode(LED_PIN, OUTPUT);
  //pin_write(LED_PIN, LOW);

  pinMode(GPS_FIX_R_PIN, OUTPUT);
  pinMode(GPS_FIX_G_PIN, OUTPUT);
  
  pin_write(GPS_FIX_R_PIN, LOW);
  pin_write(GPS_FIX_G_PIN, HIGH);
  
  Serial.begin(9600);
  Serial1.begin(GPS_BAUDRATE);
#ifdef DEBUG_RESET
  Serial.println(F("RESET"));
#endif

 // buzzer_setup();
  afsk_setup();
  gps_setup();
 // sensors_setup();

#ifdef DEBUG_SENS
  Serial.print("Ti=");
  Serial.print(sensors_int_lm60());
  Serial.print(", Te=");
  Serial.print(sensors_ext_lm60());
  Serial.print(", Vin=");
  Serial.println(sensors_vin());
#endif

  // Do not start until we get a valid time reference
  // for slotted transmissions.
  if (APRS_SLOT >= 0) {
    do {
      while (! Serial1.available())
        power_save();
    } while (! gps_decode(Serial1.read()));

      next_aprs = millis() + 1000 *
      (APRS_PERIOD - (gps_seconds + APRS_PERIOD - APRS_SLOT) % APRS_PERIOD);
  }
  else {
    next_aprs = millis();
  }  
  // TODO: beep while we get a fix, maybe indicating the number of
  // visible satellites by a series of short beeps?
  pin_write(GPS_FIX_R_PIN, HIGH);
  
  for(int i = 0; i < 3; i++){
    pin_write(GPS_FIX_G_PIN, LOW);
    delay(100);
    pin_write(GPS_FIX_G_PIN, HIGH);
    delay(100);
  }
  pin_write(GPS_FIX_G_PIN, LOW);  
}

void get_pos()
{
  // Get a valid position from the GPS
  int valid_pos = 0;
  uint32_t timeout = millis();

#ifdef DEBUG_GPS
  Serial1.println("\nget_pos()");
#endif

  gps_reset_parser();

  do {
    if (Serial1.available())
      valid_pos = gps_decode(Serial1.read());
  } while ( (millis() - timeout < VALID_POS_TIMEOUT) && ! valid_pos) ;

  if (valid_pos) {
    if (gps_altitude > BUZZER_ALTITUDE) {
      //buzzer_off();   // In space, no one can hear you buzz
    } else {
      //buzzer_on();
    }
  }
}

void loop()
{  
  // Time for another APRS frame
  pin_write(GPS_FIX_G_PIN, HIGH);
  if ((int32_t) (millis() - next_aprs) >= 0) {
    pin_write(GPS_FIX_G_PIN, LOW);
    get_pos();
    aprs_send();
    next_aprs += APRS_PERIOD * 1000L;
    while (afsk_flush()) {
      power_save();
    }
    pin_write(GPS_FIX_G_PIN, HIGH);
    delay(2000);
    data_log();

#ifdef DEBUG_MODEM
    // Show modem ISR stats from the previous transmission
    afsk_debug();
#endif

  } else {
    // Discard GPS data received during sleep window
    while (Serial1.available()) {
      Serial1.read();
    }
  }

  power_save(); // Incoming GPS data or interrupts will wake us up
}

void data_log(){
    
    float e;
    data_log_file.print(gps_time[0]);
    data_log_file.print(gps_time[1]);
    data_log_file.print(':');
    data_log_file.print(gps_time[2]);
    data_log_file.print(gps_time[3]);
    data_log_file.print(':');
    data_log_file.print(gps_time[4]);
    data_log_file.print(gps_time[5]);
    data_log_file.print(" UTC, ");
    
    e = (float)((String(gps_aprs_lat)).substring(0,2).toFloat()) + ((float)((String(gps_aprs_lat)).substring(2,4).toFloat()) / 60.0) + ((float)((String(gps_aprs_lat)).substring(5).toFloat()) * 60.0 / 100.0 /3600.0);
    if(gps_aprs_lat[7] == 'S')
      e *= -1.0;
    data_log_file.print(e,6); 
     
    data_log_file.print(F(", "));
    e = (float)((String(gps_aprs_lon)).substring(0,3).toFloat()) + ((float)((String(gps_aprs_lon)).substring(3,5).toFloat()) / 60.0) + ((float)((String(gps_aprs_lon)).substring(6).toFloat()) * 60.0 / 100.0 /3600.0);
    if(gps_aprs_lon[8] == 'W')
      e *= -1.0;
    data_log_file.print(e,6);
    
    data_log_file.print(F(", "));
    data_log_file.print((float)gps_altitude);
    data_log_file.println(F(" m;"));
    data_log_file.flush();
}
