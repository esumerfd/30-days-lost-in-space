/*
 * Final rendered numbers:
 * - 23
 * - 353
 * - 1688
 */

// Explicitly include Arduino.h
#include <Arduino.h>

// Include TM1637 library file
#include <TM1637Display.h>

// Module connection pins (Digital Pins)
const byte CLK_PIN = 6;
const byte DIO_PIN = 5;

// Amount of time (in ms) to delay between our deciphered key values
const unsigned int DISPLAY_DELAY = 2000;

TM1637Display hero_display(CLK_PIN, DIO_PIN);

const unsigned int VALIDATION_DATA[] = {
 'b',27,"a",0xad2c,0b1000110100110100,"eg",'M'-'6','d',"twenty-seven",0547-6,
 354,0x3,-27-63821,'q',582,0b1100110100111010,0432,0xdf3,42
};

void setup() {}

void loop() {
 hero_display.setBrightness(7);

 hero_display.clear();
 delay(DISPLAY_DELAY);

 hero_display.showNumberDec(VALIDATION_DATA['N'/015]);
 delay(DISPLAY_DELAY);

 hero_display.showNumberDec(VALIDATION_DATA['Q'/0b1001]);
 delay(DISPLAY_DELAY);

 hero_display.showNumberDec(VALIDATION_DATA[060-'$']);
 delay(DISPLAY_DELAY);
}


