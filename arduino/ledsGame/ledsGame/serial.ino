/*
  Serial 
*/

//Serials Commands
void serial_loop() {
  while (Serial.available()) {
    delay(5);
    if (Serial.available() > 0) {
      char c = Serial.read();
      readString += c;
    }
  }

  if (readString.length() > 0) {
    if (readString[0] == '/') {
      if (readString == "/info") {
        Serial.println(usb_name);
      }
      //Get status
      if (readString == "/status") {
        Serial.print(brightness);
        Serial.print(";");
        for (int led = 0; led < NUM_LEDS; led++) {
          Serial.print(hue[led]);
          Serial.print(";");
          Serial.print(sat[led]);
          Serial.print(";");
          Serial.print(val[led]);
          Serial.print(";");
          Serial.print(anim[led]);
          Serial.print(";");
          Serial.print(params1[led]);
          Serial.print(";");
          Serial.print(params2[led]);
          Serial.print(";");
        }
        Serial.println();
      }


      if (readString == "/save") {
        save_eeprom();
        Serial.println(readString);
      }

      //Turn off all leds
      if (readString == "/off") {
        change_color_all(0, 0, 0);
        change_anim_all(0, 0, 0);
        Serial.println(readString);
      }

      if (readString == "/on") {
        change_color_all(0, 0, 255);
        change_anim_all(0, 0, 255);
        Serial.println(readString);
      }

      if (readString == "/sync") {
        for (int led = 0; led < NUM_LEDS; led++) {
          val[led] = 255;
        }
        Serial.println(readString);
      }

      if (readString == "/reset") {
        read_eeprom();
      }

      if (readString == "/clear") {
        change_color_all(0, 0, 0);
        change_anim_all(0, 0, 0);
        LEDS.setBrightness(255);
        save_eeprom();
        Serial.println(readString);
      }
    } else {
      String command = splitString(readString, ';', 0);

      //Change brightness
      if (command == "brightness") {
        brightness = splitString(readString, ';', 1).toInt();
        LEDS.setBrightness(brightness);
        Serial.println(readString);
      }

      //Change color
      if (command == "color") {
        int led = splitString(readString, ';', 1).toInt();
        int h = splitString(readString, ';', 2).toInt();
        int s = splitString(readString, ';', 3).toInt();
        int v = splitString(readString, ';', 4).toInt();
        change_color(led, h, s, v);
        //If we turn off led we assume we want to stop the animation too.
        if ( (h == 0) && (s == 0) && (v == 0) ) {
          change_anim(led, 0, 0, 0);
        }
        Serial.println(readString);
      }

      //Change animation
      if (command == "animation") {
        int led = splitString(readString, ';', 1).toInt();
        int animation = splitString(readString, ';', 2).toInt();
        int parameter1 = splitString(readString, ';', 3).toInt();
        int parameter2 = splitString(readString, ';', 4).toInt();
        change_anim(led, animation, parameter1, parameter2);
        //Start Rainbow animation with red
        if (animation == 3) {
          change_color(led, 0, 255, 255);
        }
        Serial.println(readString);
      }

      //Load previous state
      if (command == "load") {
        int cursor = 1;
        brightness = splitString(readString, ';', cursor++).toInt();

        for (int led = 0; led < NUM_LEDS; led++) {
          int h = splitString(readString, ';', cursor).toInt();
          cursor++;
          int s = splitString(readString, ';', cursor).toInt();
          cursor++;
          int v = splitString(readString, ';', cursor).toInt();
          cursor++;
          int animation = splitString(readString, ';', cursor).toInt();
          cursor++;
          int parameter1 = splitString(readString, ';', cursor).toInt();
          cursor++;
          int parameter2 = splitString(readString, ';', cursor).toInt();
          cursor++;
          change_color(led, h, s, v);
          change_anim(led, animation, parameter1, parameter2);
        }
        Serial.println(readString);
      }

    }
    readString = "";
  }
}


//Equivalent of explode in PHP (use for serial commands parsing)
String splitString(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


