// Leds SNES

#include <FastLED.h>
#include <EEPROM.h>

/*
  Animation
  0: No animation
  1: Blinking (param1:speed in ms)
  2: Pulsing
  3: Rainbow mode
  4: Alternative Blink (param1:master/slave, param2:led/speed in ms)
*/

// Settings
const String usb_name = "leds:42001";
const int NUM_LEDS = 5;

//Buttons
const int DATA_CLOCK    = 7;
const int DATA_LATCH    = 8;
const int DATA_SERIAL  = 9;
long timingButtons = 0;
const int debounce = 10;
long timingButtonsSlow = 0;
const int debounceSlow = 100;
bool buttonsOnce = false;

//Game
//Sublevel Nb of leds
//Level 0=Color 1=ColorValue 2=ColorValueBlink

int sublevel = 1;
int level = 0;
long endGameTime = -1;
long startGameTime = 0;
long currentGameTime = 0;
bool gameActive = true;


bool buttons[12];
bool buttonsChange = false;
bool buttonsState[12] = {false, false, false, false, false, false, false, false, false, false, false, false};
String buttonsName[12] = {"B", "Y", "SELECT", "START", "UP", "DOWN", "LEFT", "RIGHT", "A", "X", "LB", "RB"};
int ledSelected = 1;

//End Anim
long timingAnimation = 0;
const int animationTime = 2000;

//Leds Setup
const int DATA_PIN = 6; //WS2812b led

//Constants animation
const int STOP = 0;
const int BLINK = 1;
const int PULSE = 2;
const int RAINBOW = 3;
const int ALTERNATE_BLINK = 4;

const int MASTER = 0;
const int SLAVE = 1;

//Leds objects
CRGB leds[NUM_LEDS];

//Leds Settings
int hue[NUM_LEDS];
int sat[NUM_LEDS];
int val[NUM_LEDS];
int brightness;

// Animation Settings
int anim[NUM_LEDS];
int params1[NUM_LEDS];
int params2[NUM_LEDS];

// Leds Status
int status[NUM_LEDS];
long timing[NUM_LEDS];


// Serial Settings
String command;
String readString; //String Buffer

//Timing
unsigned long currentTime = 0;
unsigned long previousTime = 0;

void setup() {
  // put your setup code here, to run once:
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  Serial.begin(115200);
  init_leds();
  setupController();
  resetGame();
}

void loop() {
  animation_loop();
  serial_loop();
  getControllerData();
  buttons_loop();
  if (gameActive) {
    game_loop();
  } else {

    endGame();
  }

  FastLED.show();
}

bool MatchLed(int led1, int led2) {
  /*
    if (level == 0) {
    if (hue[led2] >= (hue[led1] - 10) && hue[led2] <= (hue[led1] + 10)) {
      return true;
    }
    }
  */
  //if (level == 1) {
  if (hue[led2] >= (hue[led1] - 5) && hue[led2] <= (hue[led1] + 5)) {
    //Serial.println("HUE OK!");
    //if (sat[1] >= (sat[0] - 10) && sat[1] <= (sat[0] + 10)) {
    //Serial.println("SAT OK!");
    if (val[led2] >= (val[led1] - 3) && val[led2] <= (val[led1] + 3)) {
      //Serial.println("VAL OK!");
      return true;
    }
    // }
  }
  //}
  return false;
}

void endGame() {
  //Serial.println(currentTime);
  //Serial.println(timingAnimation);
  currentTime = millis();
  if (currentTime - timingAnimation > animationTime) {
    timingAnimation = millis();
    gameActive = true;
    hardReset();
  }
}

void gameOver() {
  Serial.println("ANIM RED BLINK");
  change_color_all(0, 255, 255);
  change_anim_all(1, 100, 0);
  timingAnimation = millis();
  gameActive = false;
}

void newHighScore() {
  Serial.println("ANIM RAINBOW");
  change_color_all(0, 255, 255);
  change_anim_all(3, 0, 0);
  timingAnimation = millis();
  gameActive = false;
}

void hardReset() {
  level = 0;
  sublevel = 1;
  currentGameTime = 0;
  startGameTime = millis();
  Serial.println("HARD RESET");
  Serial.println(currentGameTime);
  resetGame();
}

void resetGame() {
  ledSelected = 1;
  if (level == 0) {
    change_color_all(0, 255, 0);
    val[0] = 255;
  }
  if (level == 1) {
    change_color_all(0, 255, 0);
    val[0] = random(80, 255);
  }

  change_anim_all(0, 0, 0);
  hue[0] = random(0, 255);

  Serial.print("level:");
  Serial.print(level);
  Serial.print(":");
  Serial.println(sublevel);
  Serial.print("Score:");
  currentGameTime = millis() - startGameTime;
  Serial.println(currentGameTime);
  Serial.print("HighScore:");
  Serial.println(endGameTime);

  if (endGameTime != -1) {
    if (currentGameTime > endGameTime) {
      gameOver();
    }
  }

  //        Serial.println(hue[0]);
  //        Serial.println(sat[0]);
  //        Serial.println(val[0]);
  leds[0] = CHSV(hue[0], sat[0], val[0]);
  leds[0] = CHSV(hue[0], sat[1], val[0]);
}

void game_loop() {

 
 


  if (ledSelected == sublevel) {
    if (MatchLed(ledSelected, (ledSelected - 1))) {
      sublevel = sublevel + 1;
      resetGame();
    }
  }

  if (ledSelected < sublevel) {
    if (MatchLed(ledSelected, (ledSelected - 1))) {
      //ledSelected = ledSelected + 1;
      sublevel = sublevel + 1;
      resetGame();
    }
  }

  if (sublevel == 5) {
    level = level + 1;
    sublevel = 1;
    resetGame();
  }

  if (level == 2) {
    Serial.println("Game Completed");
    level = 0;
    sublevel = 1;
    resetGame();
    long currentGameTime = millis() - startGameTime;
    //Serial.println(endGameTime);
    if (endGameTime == -1) {
      Serial.println("FIRST WIN!");
      endGameTime = currentGameTime;
      //Serial.println(endGameTime);
      newHighScore();
    } else {
      if (currentGameTime < endGameTime) {
        Serial.println("NEW HIGHSCORE!");
        Serial.println(currentGameTime);
        endGameTime = currentGameTime;
        Serial.println(endGameTime);
        newHighScore();
      } else {
        Serial.println("GAME OVER");
        Serial.println(currentGameTime);
        Serial.println(endGameTime);
        gameOver();
      }
    }
    

  }
}

void buttonsActionsOnce() {
  //SELECT
  if (buttonsState[2]) {
    level = 0;
    sublevel = 1;
    startGameTime = millis();
    resetGame();

  }

  /*
    if (buttonsOnce == false) {
    //LEFT
    if (buttonsState[6]) {
      if (ledSelected > 0) {
        Serial.print("LED- :");
        ledSelected = ledSelected - 1;
        Serial.println(ledSelected);
        buttonsOnce = true;
      }
    }

    //RIGHT
    if (buttonsState[7]) {
      if (ledSelected < 4) {
        Serial.print("LED+ :");
        ledSelected = ledSelected + 1;
        Serial.println(ledSelected);
        buttonsOnce = true;
      }
    }
    }
  */
}

void buttonsActions() {
  //SLOW DEBOUNCE
  //

  if (currentTime - timingButtonsSlow > debounceSlow) {
    timingButtonsSlow = millis();
  }
  //


  //FAST DEBOUNCE
  if (currentTime - timingButtons > debounce) {
    //UP
    if (buttonsState[5]) {
      if (hue[ledSelected] > 0) {
        //Serial.print(ledSelected);
        //Serial.print(" : HUE- :");
        hue[ledSelected] = hue[ledSelected] - 1;
        //Serial.println(hue[ledSelected]);
      }
    }

    //DOWN
    if (buttonsState[4]) {
      if (hue[ledSelected] < 255) {
        //Serial.print(ledSelected);
        //Serial.print(": HUE+: ");
        hue[ledSelected] = hue[ledSelected] + 1;
        //Serial.println(hue[ledSelected]);
      }
    }



    //Y
    if (buttonsState[1]) {
      if (val[ledSelected] > 0) {
        //Serial.print(ledSelected);
        //Serial.print(" : VAL- :");
        val[ledSelected] = val[ledSelected] - 1;
        //Serial.println(val[ledSelected]);
      }
    }

    //X
    if (buttonsState[9]) {
      if (val[ledSelected] < 255) {
        //Serial.print(ledSelected);
        //Serial.print(": VAL+: ");
        val[ledSelected] = val[ledSelected] + 1;
        //Serial.println(val[ledSelected]);
      }
    }
    /*
        //B
        if (buttonsState[0]) {
          if (sat[ledSelected] > 0) {
            Serial.print(ledSelected);
            Serial.print(" : SAT- :");
            sat[ledSelected] = sat[ledSelected] - 1;
            Serial.println(sat[ledSelected]);
          }
        }

        //A
        if (buttonsState[8]) {
          if (sat[ledSelected] < 255) {
            Serial.print(ledSelected);
            Serial.print(": SAT+: ");
            sat[ledSelected] = sat[ledSelected] + 1;
            Serial.println(sat[ledSelected]);
          }
        }
    */
    leds[ledSelected] = CHSV(hue[ledSelected], sat[ledSelected], val[ledSelected]);

    timingButtons = millis();

  }

}

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


