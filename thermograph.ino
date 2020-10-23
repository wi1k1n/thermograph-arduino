#include <Adafruit_SSD1306.h>
#include <GyverButton.h>
#include <GyverTimer.h>
#include <EEPROM.h>

#include "constants.h"
#include "logo.h"
#include "util.h"

#define PGM_READ_CHARARR(val) (char*)pgm_read_word(&val)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

GButton btnL(BTNLEFT_PIN, LOW_PULL);
GButton btnR(BTNRIGHT_PIN, LOW_PULL);

GTimer timerGetTemp;
GTimer timerShowTemp;
GTimer timerStoreTemp;
GTimer timerDisplayDim;
GTimer timeoutSaveLastMenu;

char tempStrBuf[6];
bool isDimmed = false;

// temperature variables
float tempValPartAverage;  // partially averaged value of analogRead(thermistor)
byte tempValN = 0;  // number of currently got measurements
float temp;  // degrees of Celsium

// measurements and display graph variables
int8_t measData[MEASDATALENGTH];
byte curs = 0;
bool cycled = false;
bool measChanged = true;
float measMin = INT32_MAX;
float measMax = INT32_MIN;
float measMinG = INT32_MAX;
float measMaxG = INT32_MIN;
int8_t measMin8 = INT8_MAX;
byte measMinInd = 0;
byte measMaxInd = 0;
float scale = 2.f;

// Menu variables
byte menuScreen = 1;  // 0 - live temp, 1 - graph, 2 - settings
bool forceMenuRedraw = true;  // can be set to force display methods to redraw. (display methods unset this flag!)
byte settingSelected = 0;
byte settingIsChanging = false;
// https://www.arduino.cc/reference/en/language/variables/utilities/progmem/
const char* const PROGMEM settingsNames[] = {"       Dimming:", " Graph timeout:", "Clear data"};
#define SETTINGSNAMESSIZE 3
const byte settingsIsChangable = 0b11000000;  // older bit goes first
const char* const PROGMEM settingsOptsDimming[] = {"off ", " 5s ", "10s ", "15s ", "30s ", "60s "};
#define SETTINGSOPTSDIMMINGSIZE 6  // no more than 8!
byte settingsOptsDimmingCur = 3;
const char* const PROGMEM settingsOptsGraph[] = {".25s", ".5s ", " 1s ", " 2s ", " 5s ", "10s ", "15s ", "30s ", " 1m ", " 2m ", " 5m ", "10m ", "15m ", "30m ", " 1h "};
const uint16_t PROGMEM settingsOptsGraphVals[] = {250, 500, 1, 2, 5, 10, 15, 30, 60, 120, 300, 600, 900, 1800, 3600};
const uint16_t PROGMEM settingsOptsGraphValsMSMask = 0b1100000000000001;
#define SETTINGSOPTSGRAPHSIZE 15  // no more than 16!
byte settingsOptsGraphCur = 1;

// graphCurs > MEASDATALENGTH  ==>  graphCurs is not active
byte graphCurs = 255;
byte graphCursSteps = 0;


void setup() {
  // Serial.begin(57600);

  // Serial.println(pgm_read_word(&settingsOptsGraphValsMSMask));
  // for (byte i = 0; i < SETTINGSOPTSGRAPHSIZE; i++) {
  //   Serial.print(PGM_READ_CHARARR(settingsOptsGraph[i]));
  //   Serial.print(": ");
  //   Serial.println(getStoreTempTimerInterval(i));
  // }

  // Serial.print("displayDimTimeout: ");
  // Serial.println(displayDimTimeout);
  // Serial.print("GRAPHYOFFSET: ");
  // Serial.println(GRAPHYOFFSET);

  initDisplay();
  initParamsFromEEPROM();
  initButtons();
  initSensors();
  initTimers();
  
  delay(DISPLAYLOGODURATION);
  display.clearDisplay();
}

void loop() {
  btnL.tick();
  btnR.tick();
  
  // get temperature value and store it in partial average
  if (timerGetTemp.isReady()) {
    measurePartial();
  }
  // store averaged value in array
  if (timerStoreTemp.isReady()) {
    // Serial.println(F("store timer ready"));
    storeMeasurement();    
  }

  if (btnL.isPress()) {
    // if button has been pushed while active (not sleeping)
    if (!displayWakeUp()) {
      // if graph screen
      if (menuScreen == 1) {
        // graphCursor mode active
        if (graphCurs < MEASDATALENGTH) {
          if (!btnR.isHold()) {
            graphCursorMove(-1);
            forceMenuRedraw = true;
          }
        } else {}
      }
    }
  }
  if (btnR.isPress()) {
    // if button has been pushed while active (not sleeping)
    if (!displayWakeUp()) {
      if (menuScreen == 1) {
        // graphCursor mode active
        if (graphCurs < MEASDATALENGTH) {
          if (!btnL.isHold()) {
            graphCursorMove(1);
            forceMenuRedraw = true;
          }
        }
      }
    }
  }

  if (btnL.isSingle()) {
    // no need for displayWakeUp, since it is already done in isPress() check
    if (menuScreen == 1) {}
    // if settings screen
    else if (menuScreen == 2) {
        // if in process of changing settings
        if (settingIsChanging) {
          incrementSettingSelected(-1);
        }
        // if not changing settings atm
        else {
          // Serial.println("btnL -> not changing");
          settingSelected = (settingSelected + 1) % SETTINGSNAMESSIZE;
          // timeoutSaveLastMenu.start(); // probably not needed
        }
    }
    forceMenuRedraw = true;
  }
  if (btnR.isSingle()) {
    // if graph screen
    if (menuScreen == 1) {
      // graphCursor mode active
      if (graphCurs >= MEASDATALENGTH) {
        changeMenuScreen(1);
      }
    }
    // if settings screen
    else if (menuScreen == 2) {
      // if in process of changing settings
      if (settingIsChanging) {
        incrementSettingSelected(1);
      }
      // if not changing settings
      else {
        changeMenuScreen(1);
      }
    }
    else {
      changeMenuScreen(1);
    }
    forceMenuRedraw = true;
  }
  // triggers once after BTNHOLDTIMEOUT has passed
  if (btnL.isHolded()) {
    // if graph screen
    if (menuScreen == 1) {
      graphCursSteps = 0;
      // if graphCursor mode not active
      if (graphCurs >= MEASDATALENGTH) {
        graphCurs = curs;
        btnL.resetStates();
      }
    }
    // if settings screen
    else if (menuScreen == 2) {
      settingsHoldAction();
      timeoutSaveLastMenu.start();
    }
    forceMenuRedraw = true;
  }
  if (btnR.isHolded()) {
    if (menuScreen == 1) {
      graphCursSteps = 0;
    }
  }

  if (btnL.isRelease()) {
    if (!displayWakeUp()) {
      // if settings screen
      if (menuScreen == 2) {
        // if button selected (currently only at index 2)
        if (settingSelected == 2) {
          // if holded before (not single release)
          if (settingIsChanging) {
            // Serial.println(F("hbr"));
            settingIsChanging = false;
            forceMenuRedraw = true;
          }
        }
      }
    }
  }

  // if btnL is incremented by holding
  if (btnL.isStep()) {
    if (!displayWakeUp()) {
      // if on graph screen
      if (menuScreen == 1) {
        // if graphCursor mode is active
        if (graphCurs < MEASDATALENGTH) {
          // Serial.println(btnL.getHoldClicks());
          graphCursorMove(++graphCursSteps < GRAPHBTNSTEPS4SPEED ? -1 : -GRAPHBTNSPEED);
          forceMenuRedraw = true;
        }
      }
    }
  }
  // if btnR is incremented by holding
  if (btnR.isStep()) {
    if (!displayWakeUp()) {
      // if on graph screen
      if (menuScreen == 1) {
        // if graphCursor mode is active
        if (graphCurs < MEASDATALENGTH) {
          graphCursorMove(++graphCursSteps < GRAPHBTNSTEPS4SPEED ? 1 : GRAPHBTNSPEED);
          forceMenuRedraw = true;
        }
      }
    }
  }
  // both buttons are held
  if (btnL.isHold() && btnR.isHold()) {
    // if on graph screen
    if (menuScreen == 1) {
      // if graphCursor mode is active
      if (graphCurs < MEASDATALENGTH) {
        graphCurs = 255;
        forceMenuRedraw = true;
      }
    }
  }

  if (menuScreen == 0) {
    displayLive();
  } else if (menuScreen == 1) {
    displayGraph();
  } else {
    displaySettings();
  }

  // timer for dimming screen
  if (timerDisplayDim.isReady() && !isDimmed) {
    dim(true);
  }

  // timeout for saving last menu screen to EEPROM
  if (timeoutSaveLastMenu.isReady()){
    saveParams2EEPROM();
  }
}

void initDisplay() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // 0x3c - display address
    // Serial.println(F("SSD1306 allocation failed"));
    pinMode(LED_BUILTIN, OUTPUT);
    for(;;) {
      // Don't proceed, loop forever
      digitalWrite(LED_BUILTIN, HIGH);
      delay(1000);
      digitalWrite(LED_BUILTIN, LOW);
      delay(500); 
    }
  }

  // display.setRotation(2);
  display.cp437(true);  // Use full 256 char 'Code Page 437' font

  display.clearDisplay();
  display.drawBitmap(0, 0, logo_data, LOGO_WIDTH, LOGO_HEIGHT, SSD1306_WHITE);
  
  display.display();  // display is initialized with Adafruit splash screen
}
// takes 
void initParamsFromEEPROM() {
  uint16_t params;
  EEPROM.get(EEPROM.length() - 2, params);
  // 3 bits for dim, 4 bits for graph, 2 bits for last_menu, 3 bits for setting_entry
  // 15 14 13   12 11 10 9   8 7   6 5 4   3 2 1 0   <- bits
  //   dim         graph    menu   s_line            <- vars
  settingsOptsDimmingCur = params >> 13;
  settingsOptsGraphCur = (params >> 9) & 15;
  menuScreen = (params >> 7) & 3;
  settingSelected = (params >> 4) & 7;
}
void saveParams2EEPROM() {
  // 3 bits for dim, 4 bits for graph, 2 bits for last_menu, 3 bits for setting_entry
  uint16_t params = settingsOptsDimmingCur << 4;
  params = (params << 4) | settingsOptsGraphCur;
  params = (params << 2) | menuScreen;
  params = (params << 3) | settingSelected;
  params <<= 4;
  EEPROM.put(EEPROM.length() - 2, params);
}
void initButtons() {
  btnL.setDebounce(BTNDEBOUNCE);
  btnR.setDebounce(BTNDEBOUNCE);
  btnL.setClickTimeout(BTNCLICKTIMEOUT);
  btnR.setClickTimeout(BTNCLICKTIMEOUT);
  btnL.setTimeout(BTNHOLDTIMEOUT);
  btnR.setTimeout(BTNHOLDTIMEOUT);
  btnL.setStepTimeout(BTNSTEPTIMEOUT);
  btnR.setStepTimeout(BTNSTEPTIMEOUT);
}
void initSensors() {
  // here both are initialized and first measurement made
  measurePartial();
}
void initTimers() {
  // when live temp is updated (on both live and graph screens)
  timerShowTemp.setInterval(TEMPSHOWINTERVAL);

  // when graph is updated
  setStoreTempTimer();
  timerStoreTemp.setReadyOnStart(true);

  // intermediate (between each store iterations) measurements (regulated by TEMPAVERAGEN const)
  timerGetTemp.setInterval((min(TEMPSHOWINTERVAL, timerStoreTemp.getInterval()) - 1) / TEMPAVERAGEN);

  // when last visited screen should be stored in EEPROM
  timeoutSaveLastMenu.setTimeout(EEPROMSAVEMENUTIMEOUT);
  timeoutSaveLastMenu.stop();
}

void changeMenuScreen(int8_t dir) {
  menuScreen = (menuScreen + 1) % 3;
  timeoutSaveLastMenu.start();
}
// either wakes arduino up and resets dimTimeout, or just resets timeout.
// returns true, if woke up, false, if only timeout has been reset
bool displayWakeUp() {
  if (isDimmed) {
    dim(false);
    return true;
  } else {
    updateDimTimer();
    return false;
  }
}
// either turns dim on or off
void dim(const bool &v) {
  if (!timerDisplayDim.isEnabled()) return;
  if (!v) {
    // Serial.println(F("cl"));
    updateDimTimer();
    // clear display to not have any flashing screens
    display.clearDisplay();
    display.display();
  }
  display.dim(v);
  isDimmed = v;
}
// resets timeout 
void updateDimTimer() {
  if (!timerDisplayDim.isEnabled()) return;
  timerDisplayDim.start();
}

// changing value of changing setting
void incrementSettingSelected(const int8_t &dir) {
    if (settingSelected == 0)
      settingsOptsDimmingCur = (settingsOptsDimmingCur + dir) < 0 ? (SETTINGSOPTSDIMMINGSIZE-1) : ((settingsOptsDimmingCur + dir) % SETTINGSOPTSDIMMINGSIZE);
    else if (settingSelected == 1)
      settingsOptsGraphCur = (settingsOptsGraphCur + dir) < 0 ? (SETTINGSOPTSGRAPHSIZE-1) : ((settingsOptsGraphCur + dir) % SETTINGSOPTSGRAPHSIZE);
}

// is called when holded action button on settings menu
void settingsHoldAction() {
  // if button selected (currently only button at index 2)
  if (settingSelected == 2) {
    // Serial.println(F("hb"));
    curs = 0;
    cycled = false;
    measChanged = true;
    measMin = INT32_MAX;
    measMax = INT32_MIN;
    measMinG = INT32_MAX;
    measMaxG = INT32_MIN;
    measMin8 = INT8_MAX;
    measMinInd = 0;
    measMaxInd = 0;
    scale = 2.f;
    measurePartial();
    setStoreTempTimer();

    settingIsChanging = true;
    forceMenuRedraw = true;
  }
  // if changable setting is selected
  else {
    if (settingIsChanging) {
      // changing dimming interval
      if (settingSelected == 0) {
        setDisplayDimTimer();
        if (timerDisplayDim.isEnabled()) {
          forceMenuRedraw = true;
          updateDimTimer();
        }
      }
      // changing graph timeout setting
      else if (settingSelected == 1) {
        setStoreTempTimer();
        timerStoreTemp.start();
      }
      saveParams2EEPROM();
    }
    // toggle settingIsChanging flag for non-buttons
    settingIsChanging = !settingIsChanging;
  }
}

// takes index of dimming option and sets timer with according interval
void setDisplayDimTimer() {
  if (settingsOptsDimmingCur == 1) timerDisplayDim.setInterval(5000);
  else if (settingsOptsDimmingCur == 2) timerDisplayDim.setInterval(10000);
  else if (settingsOptsDimmingCur == 3) timerDisplayDim.setInterval(15000);
  else if (settingsOptsDimmingCur == 4) timerDisplayDim.setInterval(30000);
  else if (settingsOptsDimmingCur == 5) timerDisplayDim.setInterval(60000);
  else timerDisplayDim.stop();
}
// returns corresponding to ind value of store timer (in ms)
uint32_t getStoreTempTimerInterval(byte ind) {
  if (bitRead(pgm_read_word(&settingsOptsGraphValsMSMask), 15-ind))
    return pgm_read_word(&settingsOptsGraphVals[ind]);
  else return (uint32_t)pgm_read_word(&settingsOptsGraphVals[ind]) * 1000;
}
void setStoreTempTimer() {
  timerStoreTemp.setInterval(getStoreTempTimerInterval(settingsOptsGraphCur));
}



// show live temperature on display
void displayLive() {
  // do not waste cpu if display is isDimmed
  if (isDimmed) return;

  // skip execution until show timer is ready
  if (!forceMenuRedraw && !timerShowTemp.isReady())
    return;
  
  // calc averaged value of temperature
  dtostrf(computeTemp(tempValPartAverage), 6, 2, tempStrBuf);
  
  display.clearDisplay();
  
  // char size at scale 1 is 5x8. 6 digits and font_scale=3
  // display.setCursor((SCREEN_WIDTH - 6*5*3) / 2, DISPLAYPADDINGTOP + 6);
  display.setCursor(0, DISPLAYPADDINGTOP);
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setTextSize(3);
  display.print(tempStrBuf);

  display.setCursor(display.getCursorX(), display.getCursorY() - 4);
  display.setTextSize(2);
  display.print('o');

  forceMenuRedraw = false;
  display.display();
}


// build graph on the display
void displayGraph() {
  if (isDimmed) return;

  // Serial.println(forceMenuRedraw);

  /* === draw caption === */
  if (timerShowTemp.isReady() || forceMenuRedraw) {
    // only draw this caption if graphCursor mode is inactive
    if (graphCurs >= MEASDATALENGTH) {
      display.fillRect(0, 0, SCREEN_WIDTH, DISPLAYPADDINGTOP, SSD1306_BLACK);

      display.setCursor(0, 0);
      display.setTextColor(SSD1306_WHITE);  // Draw white text
      display.setTextSize(1);

      dtostrf(computeTemp(tempValPartAverage), 6, 2, tempStrBuf);
      display.print(tempStrBuf);
      // do not show min/max until first storage iteration updates its default values
      if (cycled || curs > 0) {
        display.print(" ");
        byte curX = display.getCursorX();
        display.print("l:");
        dtostrf(measMin, 5, 1, tempStrBuf);
        display.print(tempStrBuf);
        display.print(" ");
        dtostrf(measMax, 5, 1, tempStrBuf);
        display.print(tempStrBuf);
        display.setCursor(curX, 8);
        display.print("g:");
        dtostrf(measMinG, 5, 1, tempStrBuf);
        display.print(tempStrBuf);
        display.print(" ");
        dtostrf(measMaxG, 5, 1, tempStrBuf);
        display.print(tempStrBuf);
      }
    }
  }

  /* === draw graph === */
  if (measChanged || forceMenuRedraw) {
    // update graph if new data exists or forced by menyJustChanged
    measChanged = false;
    display.fillRect(0, DISPLAYPADDINGTOP, SCREEN_WIDTH, DISPLAYDATAHEIGHT, SSD1306_BLACK);

    // i iterates from 0 to cursor (or from cursor+1 up to cursor, wrapping around the MEASDATALENGTH)
    byte i = cycled ? (curs + 1) : 0;
    byte prevX = 0;
    // constrain is needed to correctly fit into screen
    byte prevY = SCREEN_HEIGHT - GRAPHPADDINGBOT - (measData[(i < MEASDATALENGTH ? i : 0)] - measMin8) * scale;
    for (byte x = 1;; i++, x++) {
      if (i == MEASDATALENGTH) i = 0;  // wrap around i
      if (i == curs) break;  // stop condition
      byte y = SCREEN_HEIGHT - GRAPHPADDINGBOT - (measData[i] - measMin8) * scale;
      display.drawLine(prevX, prevY, x, y, SSD1306_WHITE);
      prevX = x;
      prevY = y;
    }
  }

  /* === draw cursor === */
  if (graphCurs < MEASDATALENGTH) {
    display.drawLine(graphCurs, DISPLAYPADDINGTOP, graphCurs, SCREEN_HEIGHT, SSD1306_WHITE);
    
    
    display.fillRect(0, 0, SCREEN_WIDTH, DISPLAYPADDINGTOP, SSD1306_BLACK);
    // draw current X position as -time
    int16_t curBackTime = (cycled ? MEASDATALENGTH : (curs + 1)) - graphCurs;
    if (curBackTime > 0) {
      display.setCursor(0, 0);
      display.setTextColor(SSD1306_WHITE);  // Draw white text
      display.setTextSize(1);

      formatBackTime(curBackTime * timerStoreTemp.getInterval(), tempStrBuf, 1);
      if (curBackTime > 0) display.print('-');
      display.print(tempStrBuf[1]);
      display.print(tempStrBuf[2]);
      display.print(tempStrBuf[3]);
      display.print(tempStrBuf[4]);
      
      display.setCursor(display.getCursorX() + 8, 0);
      dtostrf(measData[cycled ? ((curs + graphCurs) % MEASDATALENGTH) : (graphCurs - 1)], 6, 2, tempStrBuf);
      display.print(tempStrBuf);
    }
  }

  forceMenuRedraw = false;
  display.display();
}
void graphCursorMove(const int8_t dir) {
  if ((int8_t)graphCurs + dir < 0)
    graphCurs = MEASDATALENGTH - 1;
  else if ((int8_t)graphCurs + dir >= MEASDATALENGTH)
    graphCurs = 0;
  else graphCurs += dir;
}


// show settings screen
void displaySettings() {
  if (isDimmed) return;

  if (forceMenuRedraw) {
    display.clearDisplay();
    display.setTextSize(1);

    // settings with options
    displaySettingsEntry(0, PGM_READ_CHARARR(settingsNames[0]), PGM_READ_CHARARR(settingsOptsDimming[settingsOptsDimmingCur]));
    displaySettingsEntry(1, PGM_READ_CHARARR(settingsNames[1]), PGM_READ_CHARARR(settingsOptsGraph[settingsOptsGraphCur]));

    // settings as buttons
    display.setCursor(2, (DISPLAYPADDINGTOP+2) + 12*2 + 2);  // default_padding + 12*row + extra_padding
    if (settingSelected == 2 && !settingIsChanging) {  // == row
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      display.drawRect(1, display.getCursorY()-1, 10*6+2, 10, SSD1306_WHITE);
    }
    display.print(settingsNames[2]);
    display.drawRect(0, display.getCursorY()-2, 10*6+4, 12, SSD1306_WHITE);

    // settings description
    // graph timeout setting
    if (settingSelected == 1) {
      display.setCursor(4, 4);
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);

      display.print(F("Max storage: "));
      formatBackTime(MEASDATALENGTH * getStoreTempTimerInterval(settingsOptsGraphCur), tempStrBuf, 0);
      // Serial.print(tempStrBuf);
      // Serial.print(" <- ");
      // Serial.println(MEASDATALENGTH * getStoreTempTimerInterval(settingsOptsGraphCur));
      display.print(tempStrBuf[0]);
      display.print(tempStrBuf[1]);
      display.print(tempStrBuf[2]);
      display.print(tempStrBuf[3]);
    }
    
    display.display();
  }
  forceMenuRedraw = false;
}
// draws each entry in settings
void displaySettingsEntry(byte row, char* name, char* val) {
    display.setCursor(0, (DISPLAYPADDINGTOP+2) + 12*row);
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.print(name);
    display.print(" ");
    if (settingIsChanging && settingSelected == row) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      display.drawRect(display.getCursorX()-1, display.getCursorY()-1, 4*6+2, 10, SSD1306_WHITE);
      display.drawRect(display.getCursorX()-2, display.getCursorY()-2, 4*6+4, 12, SSD1306_WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    }
    display.print(val);
    // draw box around whole entry
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    if (!settingIsChanging && settingSelected == row) {
      display.drawRect(0, display.getCursorY()-2, SCREEN_WIDTH, 12, SSD1306_WHITE);
    }
}

void measurePartial() {
  tempValPartAverage = (tempValPartAverage * tempValN + analogRead(THERMISTORPIN)) / (tempValN + 1);
  tempValN++;
}

void storeMeasurement() {
  // calculate temp out of partial average
  temp = computeTemp(tempValPartAverage);
  
  // reset partial average variables
  tempValPartAverage = 0;
  tempValN = 0;
  measurePartial();
  
  // check, if are going to overrid min or max value
  bool recalcMin = curs == measMinInd;
  bool recalcMax = curs == measMaxInd;
  bool recalcRange = false;

  // store
  measData[curs] = temp;
  
  // update min/max and scale variables
  if (temp < measMin) {
    measMin = temp;
    measMinInd = curs;
    recalcMin = false;
    recalcRange = true;
    // Serial.print("measMin: ");
    // Serial.println(measMin);
  }
  if (temp > measMax) {
    measMax = temp;
    measMaxInd = curs;
    recalcMax = false;
    recalcRange = true;
    // Serial.print("measMax: ");
    // Serial.println(measMax);
  }
  // update global min/max
  if (temp < measMinG) measMinG = temp;
  if (temp > measMaxG) measMaxG = temp;

  // increment curs
  measChanged = true;
  if (curs == MEASDATALENGTH - 1) {
    cycled = true;
    curs = 0;
  } else {
    curs++;
  }

  // recalc min/max if needed
  if (recalcMin) {
    recalcRange = true;
    measMin = INT32_MAX;
    byte i = cycled ? (curs + 1) : 0;
    for (;; i++) {
      if (i == MEASDATALENGTH) i = 0;
      if (i == curs) break;
      if (measData[i] < measMin) {
        measMin = measData[i];
        measMinInd = i;
      }
    }
  }
  if (recalcMax) {
    recalcRange = true;
    measMax = (float)INT32_MIN;
    byte i = cycled ? (curs + 1) : 0;
    for (;; i++) {
      if (i == MEASDATALENGTH) i = 0;
      if (i == curs) break;
      if (measData[i] > measMax) {
        measMax = measData[i];
        measMaxInd = i;
      }
    }
  }
  if (recalcMin || recalcMax || recalcRange) {
    measMin8 = measMin;
    int8_t range = (int8_t)measMax - measMin8;  // calculate in bytes for scale to correspond correctly to screen height
    if (range >= 1) scale = (float)(DISPLAYDATAHEIGHT - 1 - (GRAPHPADDINGTOP + GRAPHPADDINGBOT)) / range;  // use this for aligning graph vertically to the center
    else scale = 2.f;
    
    // Serial.print("; scale: ");
    // Serial.println(scale);
  }
}