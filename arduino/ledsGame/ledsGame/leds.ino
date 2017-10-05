/*
  LEDS
*/

void init_leds() {
  int status_eeprom;
  EEPROM.get(0, status_eeprom);
  if (status_eeprom != -1) {
    read_eeprom();
  } else {
    change_color_all(0, 0, 0);
    change_anim_all(0, 0, 0);
    brightness = 255;
    LEDS.setBrightness(255);
  }
}

void change_color_all(int h, int s, int v) {
  for (int i = 0; i < NUM_LEDS; i++) {
    change_color(i, h, s, v);
  }
}

void change_color(int led, int h, int s, int v) {
  leds[led] = CHSV(h, s, v);
  hue[led] = h;
  sat[led] = s;
  val[led] = v;
}

void change_anim_all(int animation, int parameter1, int parameter2) {
  for (int led = 0; led < NUM_LEDS; led++) {
    change_anim(led, animation, parameter1 , parameter2);
  }
}

void change_anim(int led, int animation, int parameter1, int parameter2) {
  anim[led] = animation;
  params1[led] = parameter1;
  params2[led] = parameter2;
  status[led] = 0;
  timing[led] = 0;
}

void animation_loop() {
  currentTime = millis();
  for (int led = 0; led < NUM_LEDS; led++) {

    // Pulse animation
    if (anim[led] == PULSE) {
      //Orientation (fading in/fading out)
      if (val[led] <= 0) {
        status[led] = 1;
      }
      if (val[led] >= 255) {
        status[led] = 0;
      }
      if (status[led] == 0) {
        val[led]--;
      } else {
        val[led]++;
      }
      change_color(led, hue[led], sat[led], val[led]);
      //Serial.println(val[led]);
    }

    //Blink animation
    if (anim[led] == BLINK) {
      //If interval is finish change state
      if (currentTime - timing[led] > params1[led]) {
        timing[led] = currentTime;
        if (val[led] == 255) {
          val[led] = 0;
        } else {
          val[led] = 255;
        }
        change_color(led, hue[led], sat[led], val[led]);
      }
    }

    if (anim[led] == RAINBOW) {
      if (hue[led] <= 0) {
        status[led] = 1;
      }
      if (hue[led] >= 255) {
        status[led] = 0;
      }
      if (status[led] == 0) {
        hue[led]--;
      } else {
        hue[led]++;
      }
      change_color(led, hue[led], sat[led], val[led]);
    }

    if (anim[led] == ALTERNATE_BLINK) {
      //If led is master (ignore if slave)
      if (params1[led] == 0) {
        int led2 = params2[led];
        int interval = params2[led2];
        if (currentTime - timing[led] > interval) {
          timing[led] = currentTime;
          if (val[led] == 255) {
            val[led] = 0;
            val[led2] = 255;
          } else {
            val[led] = 255;
            val[led2] = 0;
          }
        }
        change_color(led, hue[led], sat[led], val[led]);
        change_color(led2, hue[led2], sat[led2], val[led2]);
      }
    }
  }
}


