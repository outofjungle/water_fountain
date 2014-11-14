#include "Wire.h"
#include "LiquidCrystal.h"
#include <EEPROM.h>

#define BUTTON 7
#define PUMP 5

int button_state = LOW;
int last_button_state = LOW;

long debounce_time = 0;
long debounce_delay = 50;
char msg[32];

long start = 0;
long volume = 0;
int x, y;
int oldx, oldy;
LiquidCrystal lcd(0);
long duration;
int bottlex, bottley;

void setup() {
  Serial1.begin( 9600 );
  Serial.begin( 9600 );
  pinMode( BUTTON, INPUT );
  pinMode( PUMP, OUTPUT );
  digitalWrite( PUMP, LOW );
  lcd.begin(16, 2);
  
  read_eeprom( oldx, oldy );
  bottle_saved( oldx, oldy, bottlex, bottley );
  lcd.setCursor(0, 0);
  lcd.print(bottlex);
  lcd.print(".");
  lcd.print(bottley);
  lcd.print(" bottles");
}

void loop() {
  int current_button_state = digitalRead( BUTTON );
  if ( current_button_state != last_button_state ) {
    debounce_time = millis();
  } 
  
  if ( (millis() - debounce_time) > debounce_delay ) {
    if ( current_button_state != button_state ) {
      button_state = current_button_state;
      if ( button_state == HIGH ) {
        volume = 0;
        read_eeprom(oldx, oldy);
        start = millis();
        digitalWrite( PUMP, HIGH );
        sprintf( msg, "$START,0.0,%d.%d;", oldx, oldy );
        Serial1.print( msg );
        Serial.println( msg );
        lcd.setCursor(0, 1);
        lcd.print("                ");
      } else {
        int oldx, oldy;
        digitalWrite( PUMP, LOW );
        read_eeprom(oldx, oldy);
        duration = debounce_time - start;
        get_ml(duration, x, y);
        add_ml(oldx, oldy, x, y);
        sprintf( msg, "$STOP,%d.%d,%d.%d;", x, y, oldx, oldy );
        Serial1.print( msg );
        Serial.println( msg );
        bottle_saved( oldx, oldy, bottlex, bottley );
        lcd.setCursor(0, 0);
        lcd.print(bottlex);
        lcd.print(".");
        lcd.print(bottley);
        lcd.print(" bottles");
        write_eeprom( oldx, oldy );
      }
    }
  }

  if ( button_state == HIGH ) {
    duration = millis() - start;
    
    get_ml( duration, x, y );
    lcd.setCursor(0, 1);
    lcd.print(x);
    lcd.print(".");
    lcd.print(y);
    lcd.print(" ml poured");
  }

  last_button_state = current_button_state;
}

void get_ml( long duration, int &x, int &y ){
  x = duration / 1000;
  y = duration % 1000;
  y = y / 100;
}

void write_eeprom( int x, int y ) {
  byte xlsb = (byte)(x & 0xFFu);
  byte xmsb = (byte)((x >> 8) & 0xFFu);
  byte ylsb = (byte)(y & 0xFFu);
  byte ymsb = (byte)((y >> 8) & 0xFFu);
  
  EEPROM.write(0, xlsb);
  EEPROM.write(1, xmsb);
  EEPROM.write(2, ylsb);
  EEPROM.write(3, ymsb);
}

void read_eeprom( int &x, int &y ) {
  byte xlsb = EEPROM.read(0);
  byte xmsb = EEPROM.read(1);
  byte ylsb = EEPROM.read(2);
  byte ymsb = EEPROM.read(3);
  
  x = xmsb << 8 | xlsb;
  y = ymsb << 8 | ylsb;
}

void add_ml( int &oldx, int &oldy, int x, int y ) {
  long value = ( oldx * 10 ) + ( x * 10 ) + oldy + y;
  oldx = value / 10;
  oldy = value % 10;
}

void bottle_saved( int x, int y, int &bottlex, int &bottley ) {
  long value = ( x * 10 ) + y;
  bottlex = value / 100;
  bottley = value % 100;
  bottley = bottley / 10;
}
