/*
 * Memory EEPROM
 * 
 */

//Get Leds State
void read_eeprom() {
  int address = 0;
  EEPROM.get(0, brightness);
  LEDS.setBrightness(brightness);
  address = 2;
  for (int led = 0; led < NUM_LEDS; led++) {
    EEPROM.get(address, hue[led]);
    address = address + 2;
    EEPROM.get(address, sat[led]);
    address = address + 2;
    EEPROM.get(address, val[led]);
    address = address + 2;
    EEPROM.get(address, anim[led]);
    address = address + 2;
    EEPROM.get(address, params1[led]);
    address = address + 2;
    EEPROM.get(address, params2[led]);
    address = address + 2;
    change_color(led, hue[led], sat[led], val[led]);
    change_anim(led, anim[led], params1[led], params2[led]);
  }
}

//Save Leds State
void save_eeprom() {
  int address = 0;
  EEPROM.put(0, brightness);
  address = 2;
  for (int led = 0; led < NUM_LEDS; led++) {
    EEPROM.put(address, hue[led]);
    address = address + 2;
    EEPROM.put(address, sat[led]);
    address = address + 2;
    EEPROM.put(address, val[led]);
    address = address + 2;
    EEPROM.put(address, anim[led]);
    address = address + 2;
    EEPROM.put(address, params1[led]);
    address = address + 2;
    EEPROM.put(address, params2[led]);
    address = address + 2;
  }
  //Serial.println("Saved eeprom");
}
