#include <Adafruit_SSD1306.h>
#include <GyverButton.h>
#include <GyverTimer.h>
#include <EEPROM.h>
#include <GyverUART.h>

#include "constants.h"
#include "logo.h"
#include "util.h"

#define PGM_READ_CHARARR(val) (char*)pgm_read_word(&val)

Adafruit_SSD1306* display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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
byte menuScreen = MENUGRAPH;  // 0 - live temp, 1 - graph, 2 - settings
byte menuScreenLast = menuScreen;  // restore correct screen after settings
bool forceMenuRedraw = true;  // can be set to force display methods to redraw. (display methods unset this flag!)
byte settingSelected = SETTINGSDIMMING;
byte settingIsChanging = false;
// https://www.arduino.cc/reference/en/language/variables/utilities/progmem/
const char* const PROGMEM settingsNames[] = {"       Dimming:", " Graph timeout:", "Reset", "Save", "Load", "USB"};
#define SETTINGSNAMESSIZE 6
const byte settingsIsChangable = 0b11000000;  // older bit goes first
const char* const PROGMEM settingsOptsDimming[] = {"off ", " 5s ", "10s ", "15s ", "30s ", "60s "};
#define SETTINGSOPTSDIMMINGSIZE 6  // no more than 8!
byte settingsOptsDimmingCur = 3;
const char* const PROGMEM settingsOptsGraph[] = {".25s", ".5s ", " 1s ", " 2s ", " 5s ", "10s ", "15s ", "30s ", " 1m ", " 2m ", " 5m ", "10m ", "15m ", "30m ", " 1h "};
const uint16_t PROGMEM settingsOptsGraphVals[] = {250, 500, 1, 2, 5, 10, 15, 30, 60, 120, 300, 600, 900, 1800, 3600};
const uint16_t PROGMEM settingsOptsGraphValsMSMask = 0b1100000000000001;  // 1 if value in ms, 0 - if in s
#define SETTINGSOPTSGRAPHSIZE 15  // no more than 16!
byte settingsOptsGraphCur = 1;

// graphCurs > MEASDATALENGTH  ==>  graphCurs is not active
byte graphCurs = 255;
byte graphCursSteps = 0;

// usb-mode
boolean displayEnabled = true;
boolean updateMeasurementUSB = false;


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

  initDisplay(true);
  initParamsFromEEPROM();
  initButtons();
  initSensors();
  initTimers();
  
  delay(DISPLAYLOGODURATION);
  display->clearDisplay();
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
    if (updateMeasurementUSB) {
      uart.println(computeTemp(tempValPartAverage));
    }
  }

  if (btnL.isPress()) {
    // if button has been pushed while active (not sleeping)
    if (!displayWakeUp()) {
      // if graph screen
      if (menuScreen == MENUGRAPH) {
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
      if (menuScreen == MENUGRAPH) {
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
    if (menuScreen == MENULIVE) {
      changeMenuScreen(MENUGRAPH);
    }
    else if (menuScreen == MENUGRAPH) {
      changeMenuScreen(MENULIVE);
    }
    // if settings screen
    else if (menuScreen == MENUSETTINGS) {
      if (displayEnabled) {
        // if in process of changing settings
        if (settingIsChanging) {
          incrementSettingSelected(-1);
        }
        // if not changing settings atm
        else {
          // focus previous setting
          settingSelected = settingSelected == 0 ? (SETTINGSNAMESSIZE - 1) : (settingSelected - 1);
          // timeoutSaveLastMenu.start(); // probably not needed
        }
      }
    }
    forceMenuRedraw = true;
  }
  if (btnR.isSingle()) {
    // if graph screen
    if (menuScreen == MENULIVE) {
      changeMenuScreen(MENUGRAPH);
    } else if (menuScreen == MENUGRAPH) {
      // graphCursor mode not active
      if (graphCurs >= MEASDATALENGTH) {
        changeMenuScreen(MENULIVE);
      }
    }
    // if settings screen
    else if (menuScreen == MENUSETTINGS) {
      if (displayEnabled) {
        // if in process of changing settings
        if (settingIsChanging) {
          incrementSettingSelected(1);
        }
        // if not changing settings
        else {
          settingSelected = (settingSelected + 1) % SETTINGSNAMESSIZE;
        }
      }
    }
    forceMenuRedraw = true;
  }
  // triggers once after BTNHOLDTIMEOUT has passed
  if (btnL.isHolded()) {
    // if graph screen
    if (menuScreen == MENUGRAPH) {
      graphCursSteps = 0;
      // if graphCursor mode not active
      if (graphCurs >= MEASDATALENGTH) {
        graphCurs = curs;
        btnL.resetStates();
      }
    }
    // if settings screen
    else if (menuScreen == MENUSETTINGS) {
      if (displayEnabled) {
        settingsHoldAction();
        timeoutSaveLastMenu.start();
      } else {
        endSerialUSB();
      }
    }
    forceMenuRedraw = true;
  }
  if (btnR.isHolded()) {
    if (menuScreen == MENULIVE) {
        menuScreenLast = menuScreen;
        changeMenuScreen(MENUSETTINGS);
    } else if (menuScreen == MENUGRAPH) {
      graphCursSteps = 0;  // set g-cursor steps to 0 (after them the speed is bigger when holding)
      // if not in graph cursor mode
      if (graphCurs >= MEASDATALENGTH) {
        // save current menu screen to restore exactly to it later
        menuScreenLast = menuScreen;
        changeMenuScreen(MENUSETTINGS);
      }
    } else if (menuScreen == MENUSETTINGS) {
      if (displayEnabled) {
        if (!settingIsChanging) {
          changeMenuScreen(menuScreenLast);
        }
      }
    }
    forceMenuRedraw = true;
  }

  if (btnL.isRelease()) {
    if (!displayWakeUp()) {
      // if settings screen
      if (menuScreen == MENUSETTINGS) {
        if (displayEnabled) {
          // TODO: refactor this code!
          // if reset button selected
          if (settingSelected == SETTINGSRESET) {
            // if holded before (not single release)
            if (settingIsChanging) {
              // Serial.println(F("hbr"));
              settingIsChanging = false;
              forceMenuRedraw = true;
            }
          } else if (settingSelected == SETTINGSSAVE) {
            if (settingIsChanging) {
              settingIsChanging = false;
              forceMenuRedraw = true;
            }
          } else if (settingSelected == SETTINGSLOAD) {
            if (settingIsChanging) {
              settingIsChanging = false;
              forceMenuRedraw = true;
            }
          }
        }
      }
    }
  }

  // if btnL is incremented by holding
  if (btnL.isStep()) {
    if (!displayWakeUp()) {
      // if on graph screen
      if (menuScreen == MENUGRAPH) {
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
      if (menuScreen == MENUGRAPH) {
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
    if (menuScreen == MENUGRAPH) {
      // if graphCursor mode is active
      if (graphCurs < MEASDATALENGTH) {
        graphCurs = 255;
        forceMenuRedraw = true;
      }
    }
  }

  if (menuScreen == MENULIVE) {
    displayLive();
  } else if (menuScreen == MENUGRAPH) {
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

  // receive commands via USB
  if (!displayEnabled) {
    if (uart.available()) {
      byte cmd = uart.read();
      // send data from meas[]
      if (cmd == USBCMD_SENDDATA) {
        uart.print(timerStoreTemp.getInterval());
        uart.print(' ');
        uint8_t i = cycled ? curs : 0;  // uint16_t for MEGA
        for (;;) {
          uart.print(measData[i++]);
          if (i == MEASDATALENGTH) i = 0;  // wrap around i
          if (i == curs) break;  // stop condition
          uart.print(',');
        }
        uart.print('\r');
        uart.print('\n');
      }
      // send content of eeprom
      else if (cmd == USBCMD_SENDEEPROM) {
        uint16_t indOfStart;
        uint16_t dataLength;
        eepromGetIndLen(indOfStart, dataLength);
        if (indOfStart < EEPROMDATASTARTINDEX || indOfStart >= EEPROMDATAENDINDEX)
          indOfStart = EEPROMDATASTARTINDEX;
        if (dataLength > EEPROMDATAENDINDEX - EEPROMDATASTARTINDEX)
          dataLength = EEPROMDATAENDINDEX - EEPROMDATASTARTINDEX;
        
        uart.print(timerStoreTemp.getInterval());
        uart.print(' ');
        uint16_t indLast = indOfStart + dataLength - 1;
        if (indLast >= EEPROMDATAENDINDEX)
          indLast -= EEPROMDATAENDINDEX - EEPROMDATASTARTINDEX;
        for (;; ++indOfStart) {
          uint8_t d;
          EEPROM.get(indOfStart, d);
          uart.print(d);
          if (indOfStart == EEPROMDATAENDINDEX) indOfStart = EEPROMDATASTARTINDEX;  // wrap around i
          if (indOfStart == indLast) break;  // stop condition
          uart.print(',');
        }
        uart.print('\r');
        uart.print('\n');
      }
      // start sending updates
      else if (cmd == USBCMD_SENDLIVESTART) {
        updateMeasurementUSB = true;
      }
      // stop sending updates
      else if (cmd == USBCMD_SENDLIVESTOP) {
        updateMeasurementUSB = false;
      }
      // ping request
      else if (cmd == USBCMD_PING) {
        uart.println(USBSTATUS_PONG);
      }
    }
  }
}

void destroyDisplay() {
  delete display;
  display = NULL;
}
void initDisplay(const boolean logo) {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display->begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // 0x3c - display address
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

  // display->setRotation(2);
  display->cp437(true);  // Use full 256 char 'Code Page 437' font

  display->clearDisplay();

  if (logo) {
    display->drawBitmap(0, 0, logo_data, LOGO_WIDTH, LOGO_HEIGHT, SSD1306_WHITE);
  }
  
  display->display();  // display is initialized with Adafruit splash screen
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
  // here both sensors are initialized and the first measurement made
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

void changeMenuScreen(const uint8_t menu) {
  menuScreen = menu % 3;
  timeoutSaveLastMenu.start();
}
// either wakes arduino up and resets dimTimeout, or just resets timeout.
// returns true, if woke up, false, if only timeout has been reset
bool displayWakeUp() {
  if (!displayEnabled) return;
  if (isDimmed) {
    dim(false);
    return true;
  } else {
    updateDimTimer();
    return false;
  }
}
// either turns dim on or off
void dim(const bool v) {
  if (!displayEnabled || !timerDisplayDim.isEnabled()) return;
  if (!v) {
    // Serial.println(F("cl"));
    updateDimTimer();
    // clear display to not have any flashing screens
    display->clearDisplay();
    display->display();
  }
  display->dim(v);
  isDimmed = v;
}
// resets timeout 
void updateDimTimer() {
  if (!displayEnabled || !timerDisplayDim.isEnabled()) return;
  timerDisplayDim.start();
}

// changing value of changing setting
void incrementSettingSelected(const int8_t &dir) {
    if (settingSelected == SETTINGSDIMMING)
      settingsOptsDimmingCur = (settingsOptsDimmingCur + dir) < 0 ? (SETTINGSOPTSDIMMINGSIZE-1) : ((settingsOptsDimmingCur + dir) % SETTINGSOPTSDIMMINGSIZE);
    else if (settingSelected == SETTINGSGRAPHTIMEOUT)
      settingsOptsGraphCur = (settingsOptsGraphCur + dir) < 0 ? (SETTINGSOPTSGRAPHSIZE-1) : ((settingsOptsGraphCur + dir) % SETTINGSOPTSGRAPHSIZE);
}


// remove stored data
void onButtonReset() {
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
}
// save stored data to eeprom
void onButtonSave() {
  // first 5 bytes of eeprom contain service info:
  //           b0                         b1                        b2
  // 39 38 37 36 35 34 33 32 | 31 30 29 28 27 26 25 24 | 23 22 21 20 19 18 17 16 |
  // <------ indexOfStart (10bits) ------> <------ lengthOfData (10bits) ------>   
  //            b3                     b4
  // 15 14 13 12 11 10 09 08 |  7 6 5     4 3 2 1 0
  // <------ minTemp ------>  resolution

  // get cursor & length of last saving iteration
  // uint32_t indLenMin;
  // EEPROM.get(0, indLenMin);
  // uint16_t indOfStart = indLenMin >> 20;
  // uint16_t dataLength = (indLenMin >> 8) & 0xFFF;
  uint16_t indOfStart;
  uint16_t dataLength;
  eepromGetIndLen(indOfStart, dataLength);
  // validate and "constrain"
  // first 5 bytes and last 2 bytes are reserved as service storage
  if (indOfStart < EEPROMDATASTARTINDEX || indOfStart >= EEPROMDATAENDINDEX)
    indOfStart = EEPROMDATASTARTINDEX;
  if (dataLength > EEPROMDATAENDINDEX - EEPROMDATASTARTINDEX)
    dataLength = EEPROMDATAENDINDEX - EEPROMDATASTARTINDEX;
  
  // calculate start index for current saving (for reducing eeprom wear)
  uint32_t startShift = indOfStart + dataLength;
  if (startShift >= EEPROMDATAENDINDEX)
    startShift = EEPROMDATASTARTINDEX;

  // put measured data to eeprom byte-by-byte
  uint8_t i = cycled ? curs : 0;  // uint16_t for MEGA
  uint8_t shift = 0;  // uint16_t for MEGA
  for (;; i++, shift++) {
    if (i == MEASDATALENGTH) i = 0;  // wrap around i
    if (i == curs) break;  // stop condition
    EEPROM.put(startShift + shift, measData[i]);
  }

  // update cursor & length
  uint32_t indLenMin = (startShift << 12 | (shift & 0x0FFF)) << 8;
  EEPROM.put(0, indLenMin);
  EEPROM.put(4, 0);  // this is a placeholder for now
}
// load data from eeprom
void onButtonLoad() {
  // check the scheme in onButtonSave method
  // uint32_t indLenMin;
  // EEPROM.get(0, indLenMin);
  // uint16_t indOfStart = indLenMin >> 20;
  // uint16_t dataLength = (indLenMin >> 8) & 0xFFF;
  uint16_t indOfStart;
  uint16_t dataLength;
  eepromGetIndLen(indOfStart, dataLength);

  if (indOfStart < EEPROMDATASTARTINDEX || indOfStart >= EEPROMDATAENDINDEX)
    indOfStart = EEPROMDATASTARTINDEX;
  uint16_t dataLengthMax = min(EEPROMDATAENDINDEX - EEPROMDATASTARTINDEX, MEASDATALENGTH);
  if (dataLength > dataLengthMax)
    dataLength = dataLengthMax;

  // load measurements byte-by-byte
  for (uint8_t i = 0; i < dataLength; i++, indOfStart++) {  // uint16_t for MEGA
    if (indOfStart >= EEPROMDATAENDINDEX) indOfStart = EEPROMDATAENDINDEX;
    EEPROM.get(indOfStart, measData[i]);
  }
  cycled = dataLength >= MEASDATALENGTH;
  curs = cycled ? 0 : dataLength;

  // display->setCursor(0, 0);
  // display->setTextColor(SSD1306_WHITE);
  // display->setTextSize(1);

  // display->print(measMin);
  // display->print(' ');

  recalculateMin();
  recalculateMax();
  recalculateRange();
  if (measMin < measMinG) measMinG = measMin;
  if (measMax > measMaxG) measMaxG = measMax;

  // display->print(measMin);
  // display->display();

  // delay(1000);
  
  tempValPartAverage = 0;
  tempValN = 0;
  measurePartial();
}
// reads eepromCursor and dataLength from eeprom
void eepromGetIndLen(uint16_t &ind, uint16_t &len) {
  uint32_t indLenMin;
  EEPROM.get(0, indLenMin);
  ind = indLenMin >> 20;
  len = (indLenMin >> 8) & 0xFFF;
}
// turn USB synchronization mode on
void onButtonUSB() {
  display->clearDisplay();
  display->setTextSize(2);
  display->setCursor(13, DISPLAYPADDINGTOP);
  display->print(F("Hold LEFT to exit"));
  display->display();

  destroyDisplay();
  displayEnabled = false;

  startSerialUSB();
}
void startSerialUSB() {
  uart.begin(9600);
  uart.println(USBSTATUS_BEGIN);
}
void endSerialUSB() {
  updateMeasurementUSB = false;
  uart.println(USBSTATUS_END);
  delay(5);
  uart.end();

  display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  initDisplay(false);
  displayEnabled = true;
}
// is called when holded action button on settings menu
void settingsHoldAction() {
  if (settingSelected == SETTINGSRESET) {
    onButtonReset();
    settingIsChanging = true;
  }
  else if (settingSelected == SETTINGSSAVE) {
    onButtonSave();
    settingIsChanging = true;
  }
  else if (settingSelected == SETTINGSLOAD) {
    onButtonLoad();
    settingIsChanging = true;
  }
  else if (settingSelected == SETTINGSUSB) onButtonUSB();
  // if changable setting is selected
  else {
    if (settingIsChanging) {
      // changing dimming interval
      if (settingSelected == SETTINGSDIMMING) {
        setDisplayDimTimer();
        if (timerDisplayDim.isEnabled()) {
          updateDimTimer();
        }
      }
      // changing graph timeout setting
      else if (settingSelected == SETTINGSGRAPHTIMEOUT) {
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
  if (isDimmed || !displayEnabled) return;

  // skip execution until show timer is ready
  if (!forceMenuRedraw && !timerShowTemp.isReady())
    return;
  
  // calc averaged value of temperature
  dtostrf(computeTemp(tempValPartAverage), 6, 2, tempStrBuf);
  
  display->clearDisplay();
  
  // char size at scale 1 is 5x8. 6 digits and font_scale=3
  // display->setCursor((SCREEN_WIDTH - 6*5*3) / 2, DISPLAYPADDINGTOP + 6);
  display->setCursor(0, DISPLAYPADDINGTOP);
  display->setTextColor(SSD1306_WHITE);  // Draw white text
  display->setTextSize(3);
  display->print(tempStrBuf);

  display->setCursor(display->getCursorX(), display->getCursorY() - 4);
  display->setTextSize(2);
  display->print('o');

  forceMenuRedraw = false;
  display->display();
}


// build graph on the display
void displayGraph() {
  if (isDimmed || !displayEnabled) return;

  // Serial.println(forceMenuRedraw);

  /* === draw caption === */
  if (timerShowTemp.isReady() || forceMenuRedraw) {
    // only draw this caption if graphCursor mode is inactive
    if (graphCurs >= MEASDATALENGTH) {
      display->fillRect(0, 0, SCREEN_WIDTH, DISPLAYPADDINGTOP, SSD1306_BLACK);

      display->setCursor(0, 0);
      display->setTextColor(SSD1306_WHITE);  // Draw white text
      display->setTextSize(1);

      dtostrf(computeTemp(tempValPartAverage), 6, 2, tempStrBuf);
      display->print(tempStrBuf);
      // do not show min/max until first storage iteration updates its default values
      if (cycled || curs > 0) {
        display->print(" ");
        byte curX = display->getCursorX();
        display->print("l:");
        dtostrf(measMin, 5, 1, tempStrBuf);
        display->print(tempStrBuf);
        display->print(" ");
        dtostrf(measMax, 5, 1, tempStrBuf);
        display->print(tempStrBuf);
        display->setCursor(curX, 8);
        display->print("g:");
        dtostrf(measMinG, 5, 1, tempStrBuf);
        display->print(tempStrBuf);
        display->print(" ");
        dtostrf(measMaxG, 5, 1, tempStrBuf);
        display->print(tempStrBuf);
      }
    }
  }

  /* === draw graph === */
  if (measChanged || forceMenuRedraw) {
    // update graph if new data exists or forced by menyJustChanged
    measChanged = false;
    display->fillRect(0, DISPLAYPADDINGTOP, SCREEN_WIDTH, DISPLAYDATAHEIGHT, SSD1306_BLACK);

    // i iterates from 0 to cursor (or from cursor+1 up to cursor, wrapping around the MEASDATALENGTH)
    byte i = cycled ? (curs + 1) : 0;
    byte prevX = 0;
    // constrain is needed to correctly fit into screen
    byte prevY = SCREEN_HEIGHT - GRAPHPADDINGBOT - (measData[(i < MEASDATALENGTH ? i : 0)] - measMin8) * scale;
    for (byte x = 1;; i++, x++) {
      if (i == MEASDATALENGTH) i = 0;  // wrap around i
      if (i == curs) break;  // stop condition
      byte y = SCREEN_HEIGHT - GRAPHPADDINGBOT - (measData[i] - measMin8) * scale;
      display->drawLine(prevX, prevY, x, y, SSD1306_WHITE);
      prevX = x;
      prevY = y;
    }
  }

  /* === draw cursor === */
  if (graphCurs < MEASDATALENGTH) {
    display->drawLine(graphCurs, DISPLAYPADDINGTOP, graphCurs, SCREEN_HEIGHT, SSD1306_WHITE);
    
    
    display->fillRect(0, 0, SCREEN_WIDTH, DISPLAYPADDINGTOP, SSD1306_BLACK);
    // draw current X position as -time
    int16_t curBackTime = (cycled ? MEASDATALENGTH : (curs + 1)) - graphCurs;
    if (curBackTime > 0) {
      display->setCursor(0, 0);
      display->setTextColor(SSD1306_WHITE);  // Draw white text
      display->setTextSize(1);

      formatBackTime(curBackTime * timerStoreTemp.getInterval(), tempStrBuf, 1);
      if (curBackTime > 0) display->print('-');
      display->print(tempStrBuf[1]);
      display->print(tempStrBuf[2]);
      display->print(tempStrBuf[3]);
      display->print(tempStrBuf[4]);
      
      display->setCursor(display->getCursorX() + 8, 0);
      dtostrf(measData[cycled ? ((curs + graphCurs) % MEASDATALENGTH) : (graphCurs - 1)], 6, 2, tempStrBuf);
      display->print(tempStrBuf);
    }
  }

  forceMenuRedraw = false;
  display->display();
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
  if (isDimmed || !displayEnabled) return;

  if (forceMenuRedraw) {
    display->clearDisplay();
    display->setTextSize(1);

    // dimming & graph timeout
    displaySettingsEntry(SETTINGSDIMMING, PGM_READ_CHARARR(settingsNames[0]), PGM_READ_CHARARR(settingsOptsDimming[settingsOptsDimmingCur]));
    displaySettingsEntry(SETTINGSGRAPHTIMEOUT, PGM_READ_CHARARR(settingsNames[1]), PGM_READ_CHARARR(settingsOptsGraph[settingsOptsGraphCur]));

    // save, load, usb button
    const byte PROGMEM SLULEFTPADDING = 6;  // SaveLoadUsbLeftPadding
    const byte PROGMEM SLUY = (DISPLAYPADDINGTOP+2) + 12*2;

    // clear data
    displaySettingsButton(SETTINGSRESET, SLULEFTPADDING, SLUY, 5, PGM_READ_CHARARR(settingsNames[2]));
    displaySettingsButton(SETTINGSSAVE, display->getCursorX() + 3, SLUY, 4, PGM_READ_CHARARR(settingsNames[3]));
    displaySettingsButton(SETTINGSLOAD, display->getCursorX() + 3, SLUY, 4, PGM_READ_CHARARR(settingsNames[4]));
    displaySettingsButton(SETTINGSUSB, display->getCursorX() + 3, SLUY, 3, PGM_READ_CHARARR(settingsNames[5]));


    // settings description
    // graph timeout setting
    if (settingSelected == SETTINGSGRAPHTIMEOUT) {
      display->setCursor(4, 4);
      display->setTextColor(SSD1306_WHITE, SSD1306_BLACK);

      display->print(F("Max storage: "));
      formatBackTime(MEASDATALENGTH * getStoreTempTimerInterval(settingsOptsGraphCur), tempStrBuf, 0);
      // Serial.print(tempStrBuf);
      // Serial.print(" <- ");
      // Serial.println(MEASDATALENGTH * getStoreTempTimerInterval(settingsOptsGraphCur));
      display->print(tempStrBuf[0]);
      display->print(tempStrBuf[1]);
      display->print(tempStrBuf[2]);
      display->print(tempStrBuf[3]);
    }
    
    display->display();
  }
  forceMenuRedraw = false;
}
// draws each entry in settings
void displaySettingsEntry(const byte row, const char* name, const char* val) {
    display->setCursor(0, (DISPLAYPADDINGTOP+2) + 12*row);
    display->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display->print(name);
    display->print(" ");
    if (settingIsChanging && settingSelected == row) {
      display->setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      display->drawRect(display->getCursorX()-1, display->getCursorY()-1, 4*6+2, 10, SSD1306_WHITE);
      display->drawRect(display->getCursorX()-2, display->getCursorY()-2, 4*6+4, 12, SSD1306_WHITE);
    } else {
      display->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    }
    display->print(val);
    // draw box around whole entry
    display->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    if (!settingIsChanging && settingSelected == row) {
      display->drawRect(0, display->getCursorY()-2, SCREEN_WIDTH, 12, SSD1306_WHITE);
    }
}
// draws button in settings
void displaySettingsButton(const byte ind, const byte x, const byte y, const byte l, const char* val) {
    display->setCursor(x + 2, y + 2);
    if (settingSelected == ind && !settingIsChanging) {  // == row
      display->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    } else {
      display->setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      display->drawRect(display->getCursorX()-1, display->getCursorY()-1, l*6+2, 10, SSD1306_WHITE);
    }
    display->drawRect(display->getCursorX()-2, display->getCursorY()-2, l*6+4, 12, SSD1306_WHITE);
    display->print(val);
}

void measurePartial() {
  tempValPartAverage = (tempValPartAverage * tempValN + analogRead(THERMISTORPIN)) / (tempValN + 1);
  tempValN++;
}

void storeMeasurement() {
  // calculate temp out of partial average
  float temp = computeTemp(tempValPartAverage);
  
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
  }
  if (temp > measMax) {
    measMax = temp;
    measMaxInd = curs;
    recalcMax = false;
    recalcRange = true;
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
    recalculateMin();
  }
  if (recalcMax) {
    recalcRange = true;
    recalculateMax();
  }
  if (recalcMin || recalcMax || recalcRange) {
    recalculateRange();
  }
}

void recalculateMin() {
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
void recalculateMax() {
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
void recalculateRange() {
  measMin8 = measMin;
  int8_t range = (int8_t)measMax - measMin8;  // calculate in bytes for scale to correspond correctly to screen height
  if (range >= 1) scale = (float)(DISPLAYDATAHEIGHT - 1 - (GRAPHPADDINGTOP + GRAPHPADDINGBOT)) / range;  // use this for aligning graph vertically to the center
  else scale = 2.f;
}