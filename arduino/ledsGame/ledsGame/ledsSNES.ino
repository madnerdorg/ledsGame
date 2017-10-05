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



