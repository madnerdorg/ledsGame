// SNES Controller

// Prints which buttons are pressed
void buttons_loop(void) {
  //B Button

  buttonsChange = false;
  for (int i = 0; i < 12; i++) {
    if (buttons[i] == false) {
      
      if (buttonsState[i] == false) {

        buttonsState[i] = true;
        for (int i = 0; i < 12; i++) {
          if (buttonsState[i] == true) {
            //Serial.println(buttonsName[i]);
            buttonsActionsOnce();
          }
        }
      }
    } else {
      if (buttonsState[i] == true) {
        buttonsState[i] = false;
        //Serial.println("B RELEASE");
        buttonsOnce = false;
      }
    }
  }
  buttonsActions();

}


void getControllerData(void) {
  // Latch for 12us
  digitalWrite(DATA_LATCH, HIGH);
  delayMicroseconds(12);
  digitalWrite(DATA_LATCH, LOW);
  delayMicroseconds(6);

  // Retrieve button presses from shift register by pulling the clock high for 6us
  for (int i = 0; i < 16; i++) {
    digitalWrite(DATA_CLOCK, LOW);
    delayMicroseconds(6);
    if (i <= 11) {
      buttons[i] = digitalRead(DATA_SERIAL);
    }
    digitalWrite(DATA_CLOCK, HIGH);
    delayMicroseconds(6);

  }

}

void setupController(void) {
  // Set DATA_CLOCK normally HIGH
  pinMode(DATA_CLOCK, OUTPUT);
  digitalWrite(DATA_CLOCK, HIGH);

  // Set DATA_LATCH normally LOW
  pinMode(DATA_LATCH, OUTPUT);
  digitalWrite(DATA_LATCH, LOW);

  // Set DATA_SERIAL normally HIGH
  pinMode(DATA_SERIAL, OUTPUT);
  digitalWrite(DATA_SERIAL, HIGH);
  pinMode(DATA_SERIAL, INPUT);
}
