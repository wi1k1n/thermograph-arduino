#include <Adafruit_SSD1306.h>
#include <GyverButton.h>
#include <GyverTimer.h>
#include <EEPROM.h>
#include <GyverUART.h>
#include <BCArray.h>

#include "constants.h"
#include "logo.h"
#include "util.h"
#include "RBCArray.h"

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
// BCArray* measArr = new BCArray(MP_CAP, MEASDATALENGTH);
RBCArray measData(MEASDATABYTELENGTH, MP_CAP);

byte curs = 0;
bool cycled = false;
float measMin = INT32_MAX;
float measMax = INT32_MIN;
float measMinG = INT32_MAX;
float measMaxG = INT32_MIN;
// int8_t measMin8 = INT8_MAX;
// int16_t measMinInd = 0;
// int16_t measMaxInd = 0;

bool measChanged = true;
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

boolean graphCursMode = false;
uint16_t graphCursI = 0;  // shift of measData from which graph starts rendering
uint16_t graphCurs = 0;  // current pos of cursor in measData coordinates
byte graphCursSteps = 0;  // steps (for increasing graph cursor speed)

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

  // connect and quickly communicate with app
  // (to update the app in case it has been restarted)
  uart.begin(9600);
  uart.println(USBSTATUS_END);
  delay(15);
  uart.end();

  initDisplay(true);
  initParamsFromEEPROM();
  initButtons();
  initSensors();
  initTimers();
  
  delay(DISPLAYLOGODURATION);
  display->clearDisplay();

  // DEBUG!!!
  // float decay = 1;
  // for (uint8_t i = 0; i < 150; i++) {
  //   const uint16_t amin = 438, amax = 583;
  //   tempValPartAverage = (amax - amin) * (0.5 * cos(i * 0.17f) + 0.5) * decay + amin;
  //   storeMeasurement();
  //   decay *= 0.995;
  // }

  // tempValPartAverage = 0;
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
    // send temp over Serial if USB mode is ON
    if (updateMeasurementUSB) {
      uart.println(computeTemp(tempValPartAverage));
    }
  }

  if (btnL.isPress()) {
    // if graph screen
    if (menuScreen == MENUGRAPH) {
      // graphCursor mode active
      if (graphCursMode) {
        if (!btnR.isHold()) {
          graphCursorMove(-1);
          forceMenuRedraw = true;
        }
      }
    }
  }
  if (btnR.isPress()) {
    // if button has been pushed while active (not sleeping)
    if (menuScreen == MENUGRAPH) {
      // graphCursor mode active
      if (graphCursMode) {
        if (!btnL.isHold()) {
          graphCursorMove(1);
          forceMenuRedraw = true;
        }
      }
    }
  }

  if (btnL.isSingle()) {
    if (!displayWakeUp()) {
      if (menuScreen == MENULIVE) {
        changeMenuScreen(MENUGRAPH);
      }
      else if (menuScreen == MENUGRAPH) {
        if (!graphCursMode) {
          changeMenuScreen(MENULIVE);
        }
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
  }
  if (btnR.isSingle()) {
    if (!displayWakeUp()) {
      // if graph screen
      if (menuScreen == MENULIVE) {
        changeMenuScreen(MENUGRAPH);
      } else if (menuScreen == MENUGRAPH) {
        // graphCursor mode not active
        if (!graphCursMode) {
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
  }
  // triggers once after BTNHOLDTIMEOUT has passed
  if (btnL.isHolded()) {
    // if graph screen
    if (menuScreen == MENUGRAPH) {
      if (graphCursMode) {
        graphCursSteps = 0;
      }
      // if graphCursor mode NOT active
      else {
        graphCurs = measData.count() - 1;
        graphCursMode = true;
        recalcMinMaxRange();
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
      if (graphCursMode) {
        graphCursSteps = 0;  // set g-cursor steps to 0 (after them the speed is bigger when holding)
      }
      // if NOT in graph cursor mode
      else {
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
        if (graphCursMode) {
          // if RIGHT is not holded
          if (!btnR.isHold()) {
            graphCursorMove(++graphCursSteps < GRAPHBTNSTEPS4SPEED ? -1 : -GRAPHBTNSPEED);
            forceMenuRedraw = true;
          }
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
        if (graphCursMode) {
          // if LEFT is not holded
          if (!btnL.isHold()) {  // TODO: this condition (as well as to other button) does not work!
            graphCursorMove(++graphCursSteps < GRAPHBTNSTEPS4SPEED ? 1 : GRAPHBTNSPEED);
            forceMenuRedraw = true;
          }
        }
      }
    }
  }
  // both buttons are held
  if (btnL.isHold() && btnR.isHold()) {
    // if on graph screen
    if (menuScreen == MENUGRAPH) {
      // if graphCursor mode is active
      if (graphCursMode) {
        // graphCurs = 255;
        graphCursMode = false;
        recalcMinMaxRange();
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
  if (!displayEnabled) { //CURS
    if (uart.available()) {
      byte cmd = uart.read();
      // send data from measData
      if (cmd == USBCMD_SENDDATA) {
        uart.print(timerStoreTemp.getInterval());
        uart.print(' ');
        for (uint16_t i = 0; i < measData.count(); i++) {
          uart.print(getTemp(i));
          uart.print(',');
        }
        uart.println();
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
  // dim (3 bit) - dimming timeout
  // graph (4 bit) - graph update timeout
  // menu (2 bit) - last visited menu
  // s_line (3 bit) - last selected entry in settings
  //
  //          b1022                      b1023
  // 15 14 13  12 11 10 09  08 | 07 06 05  04 03  02 01 00
  // <- dim->  <- graph ->                 <menu> <s_line>
  uint8_t b1022, b1023;
  EEPROM.get(EEPROM.length() - 2, b1022);
  EEPROM.get(EEPROM.length() - 1, b1023);
  settingsOptsDimmingCur = b1022 >> 5;
  settingsOptsGraphCur = (b1022 >> 1) & 0xF;
  menuScreen = (b1023 >> 3) & 0x3;
  settingSelected = b1023 & 0x7;
}
void saveParams2EEPROM() {
  // 3 bits for dim, 4 bits for graph, 2 bits for last_menu, 3 bits for setting_entry
  EEPROM.put(EEPROM.length() - 2, (uint8_t)(settingsOptsDimmingCur << 4 | settingsOptsGraphCur) << 1);
  EEPROM.put(EEPROM.length() - 1, (uint8_t)(menuScreen << 3 | settingSelected));
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

  // when display starts dimming
  setDisplayDimTimer();
}

void changeMenuScreen(const uint8_t menu) {
  menuScreen = menu % 3;
  timeoutSaveLastMenu.start();
}
// either wakes arduino up and resets dimTimeout, or just resets timeout.
// returns true, if woke up, false, if only timeout has been reset
bool displayWakeUp() {
  if (!displayEnabled)
    return false;
  if (isDimmed) {
    dim(false);
    return true;
  }
  updateDimTimer();
  return false;
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
    // curs = 0;
    // cycled = false;
    measData.clear();
    measChanged = true;
    measMin = DMAX;
    measMax = DMIN;
    measMinG = DMAX;
    measMaxG = DMIN;
    // measMin8 = DMAX;
    // measMinInd = 0;
    // measMaxInd = 0;
    scale = 2.f;
    measurePartial();
    setStoreTempTimer();
}
// save stored data to eeprom
void onButtonSave() {
  // pinMode(LED_BUILTIN, OUTPUT);
  // digitalWrite(LED_BUILTIN, HIGH);
  // delay(100);
  // digitalWrite(LED_BUILTIN, LOW);
  // return;

  // first 7 bytes of eeprom contain service info
  uint16_t indOfStart, dataCurs;
  int8_t mpmin, mpmax;
  uint8_t mpcap;
  boolean cycl;
  eepromGetIndLen(indOfStart, mpmin, mpmax, mpcap, dataCurs, cycl);

  // display->setCursor(0, 0);
  // display->print(indOfStart);
  // display->print(' ');
  // display->print(dataLength);
  // display->print(' ');
  // display->print(dataCurs);
  // display->setCursor(0, 8);
  // display->print(mpmin);
  // display->print(' ');
  // display->print(mpmax);
  // display->print(' ');
  // display->print(mpcap);
  // display->print(' ');
  // display->print(cycl);
  // display->display();

  // return;

  // validate and "constrain"
  // first 6 bytes and last 2 bytes are reserved as service storage
  if (indOfStart < EEPROMDATASTARTINDEX || indOfStart >= EEPROMDATAENDINDEX) indOfStart = EEPROMDATASTARTINDEX;
  
  // calculate start index for current saving (for reducing eeprom wear)
  uint32_t eepromStart = indOfStart + measData.byteLength();
  if (eepromStart >= EEPROMDATAENDINDEX)
    eepromStart -= EEPROMDATAENDINDEX - EEPROMDATASTARTINDEX;  // wrap cursor around


  // dbg_begin();
  // uart.println();
  // TODO!!!! eepromCurs = 7;
  // put measured data to eeprom byte-by-byte
  uint16_t eepromCurs = eepromStart;
  // uint16_t iStopInd = (uint16_t)ceil((cycled ? measArr->byteLength() : curs) * MP_CAP / 8.f);
  // uart.println(eepromCurs);
  // uart.println(iStopInd);
  // uart.println(curs);
  // uart.println();
  for (uint8_t i = 0; i < measData.byteLength(); i++, eepromCurs++) {  // uint16_t for MEGA
    if (eepromCurs >= EEPROMDATAENDINDEX) eepromCurs = EEPROMDATASTARTINDEX;
    EEPROM.put(eepromCurs, measData.byteArray()[i]);
    // uart.println(measArr->byteArray()[i]);
  }

  // uart.println();
  // uart.println(eepromCurs);
  // delay(100);
  // dbg_end();

  // update service bytes
  eepromPutIndLen(eepromStart, MP_MIN, MP_MAX, MP_CAP, measData.cursor(), measData.cycled());
}
// load data from eeprom
void onButtonLoad() {
  // pinMode(LED_BUILTIN, OUTPUT);
  // digitalWrite(LED_BUILTIN, HIGH);
  // delay(100);
  // digitalWrite(LED_BUILTIN, LOW);
  // return;

  // load service information from eeprom
  uint16_t indOfStart, dataCurs;
  int8_t mpmin, mpmax;
  uint8_t mpcap;
  boolean cycl;
  eepromGetIndLen(indOfStart, mpmin, mpmax, mpcap, dataCurs, cycl);

  // validate and "constrain"
  if (indOfStart < EEPROMDATASTARTINDEX || indOfStart >= EEPROMDATAENDINDEX) indOfStart = EEPROMDATASTARTINDEX;
  mpmin = constrain(mpmin, DMIN, DMAX);
  mpmax = constrain(mpmax, DMIN, DMAX);
  if (mpmax - mpmin < 1) {
    mpmax = mpmin;
    if (mpmax >= 127) mpmin = 126;
    else mpmax = mpmin + 1;
  }
  mpcap = constrain(mpcap, 2, 8);

  // dbg_begin();
  // uart.println();
  // uart.println(indOfStart);
  // uart.println();

  // indOfStart = 7;
  // load measurements byte-by-byte
  for (uint8_t i = 0; i < measData.byteLength(); i++, indOfStart++) {  // uint16_t for MEGA
    if (indOfStart >= EEPROMDATAENDINDEX) indOfStart = EEPROMDATAENDINDEX;
    EEPROM.get(indOfStart, measData.byteArray()[i]);
    // uart.println(measArr->byteArray()[i]);
  }
  measData.setCycled(cycl);
  measData.setCursor(dataCurs);
  // cycled = cycl;
  // curs = dataCurs;
  
  // uart.println();
  // uart.println((int)indOfStart);
  // delay(100);
  // dbg_end();

  //TODO: mpmin/mpmax should be considered here

  // display->setCursor(0, 0);
  // display->setTextColor(SSD1306_WHITE);
  // display->setTextSize(1);

  // display->print(measMin);
  // display->print(' ');

  // recalculateMin();
  // recalculateMax();
  // recalculateRange();

  recalcMinMaxRange();

  if (measMin < measMinG) measMinG = measMin;
  if (measMax > measMaxG) measMaxG = measMax;

  // display->print(measMin);
  // display->display();

  // delay(1000);
  
  tempValPartAverage = 0;
  tempValN = 0;
  measurePartial();
}
// reads data service variables from eeprom
void eepromGetIndLen(uint16_t &ind,
                     int8_t &mpmin,
                     int8_t &mpmax,
                     uint8_t &mpcap,
                     uint16_t &curs,
                     boolean &cycl) {
  // ind - index of byte in eeprom where data starts
  // mpmin/mpmax - min/max values of temperature (in degrees)
  // mpcap - number of bits used for each number
  // curs - index of number (NOT byte!) in data where measurements start
  // cycl - if loaded data is cycled or not (measurements start either from 0 or from curs)
  //
  // first 6 bytes of eeprom contain service info:
  //           b0                          b1            
  // 31 30 29 28 27 26 25 24 | 23 22 21 20 19 18 17   16 
  // <------ indexOfStart (12bits) ------> <-MPCAP-> cycl
  //           b2                        b3
  // 15 14 13 12 11 10 09 08 | 07 06 05 04 03 02 01 00
  //    <----------- dataCursor (15bits) ------------>
  //           b4                        b5           
  // 23 22 21 20 19 18 17 16 | 15 14 13 12 11 10 09 08
  // <--- MPMIN (8bits) --->   <--- MPMAX (8bits) --->
  uint8_t b0, b1, b2, b3;
  EEPROM.get(0, b0);
  EEPROM.get(1, b1);
  EEPROM.get(2, b2);
  EEPROM.get(3, b3);
  EEPROM.get(4, mpmin);
  EEPROM.get(5, mpmax);
  ind = b0 << 4 | b1 >> 4;
  mpcap = (uint8_t)(b1 >> 1 & 0x7) + 1;
  cycl = b1 & 0x1;
  curs = (b2 & 0x7F) << 8 | b3;
}
void eepromPutIndLen(const uint16_t &ind,
                     const int8_t &mpmin,
                     const int8_t &mpmax,
                     const uint8_t &mpcap,
                     const uint16_t &dcurs,
                     const boolean &cycl) {
  // args same as in eepromGetIndLen
  EEPROM.put(0, (uint8_t)(ind >> 4));
  EEPROM.put(1, (uint8_t)(((ind & 0xF) << 3 | (mpcap - 1)) << 1 | cycl));
  EEPROM.put(2, (uint8_t)(dcurs >> 8));
  EEPROM.put(3, (uint8_t)(dcurs & 0xFF));
  EEPROM.put(4, mpmin);
  EEPROM.put(5, mpmax);
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
  delay(15);
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




uint8_t dbgFlag = 0;
uint8_t dbgFlag2 = 0;
// build graph on the display
void displayGraph() {
  if (isDimmed || !displayEnabled) return;

  // Serial.println(forceMenuRedraw);

  /* === draw caption === */
  if (timerShowTemp.isReady() || forceMenuRedraw) {
    // only draw this caption if graphCursor mode is NOT active
    if (!graphCursMode) {
      display->fillRect(0, 0, SCREEN_WIDTH, DISPLAYPADDINGTOP, SSD1306_BLACK);

      display->setCursor(0, 0);
      display->setTextColor(SSD1306_WHITE);  // Draw white text
      display->setTextSize(1);

      dtostrf(computeTemp(tempValPartAverage), 6, 2, tempStrBuf);
      display->print(tempStrBuf);
      // do not show min/max until first storage iteration updates its default values
      if (measData.count() > 0) {
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

    // i might start from >0, since measData.count() can be bigger, than SCREEN_WIDTH
    uint16_t i = max(0, (int16_t)measData.count() - SCREEN_WIDTH);

    // if graph cursor mode is ON, we can move cursor beyond usually visible parts
    // so simply override starting index
    // dbgFlag = 0;
    // dbgFlag2 = 0;
    if (graphCursMode) {
      // if graphCurs if in left side padding zone
      if ((int16_t)graphCurs - graphCursI <= GRAPHCURSORSIDEPADDING) {
        graphCursI = max(0, (int16_t)graphCurs - GRAPHCURSORSIDEPADDING);
        // dbgFlag = 1;
      }
      // graphCurs in right side padding zone OR outside of screen to the left (wrapped around to 0)
      else if (graphCursI + SCREEN_WIDTH - (int16_t)graphCurs <= GRAPHCURSORSIDEPADDING) {
        graphCursI = min(measData.length() - SCREEN_WIDTH, (int16_t)graphCurs + GRAPHCURSORSIDEPADDING - SCREEN_WIDTH);
        // dbgFlag = 2;
      }
      // graphCurs in the middle zone
      else {
        // dbgFlag = 3;
      }
      // check if cursor is outside of screen area (after it was wrapped around)
      if ((int16_t)graphCurs - graphCursI >= SCREEN_WIDTH) {
        // dbgFlag2 += 1;
        graphCursI = max((int16_t)graphCurs - SCREEN_WIDTH + 1, 0);
      }
      i = graphCursI;
    }

    byte prevX = 0;
    byte prevY = SCREEN_HEIGHT - GRAPHPADDINGBOT - (getTemp(i) - measMin) * scale;
    for (uint8_t x = 0; i < measData.count(); i++, x++) {
      uint8_t y = SCREEN_HEIGHT - GRAPHPADDINGBOT - (getTemp(i) - measMin) * scale;
      display->drawLine(prevX, prevY, x, y, SSD1306_WHITE);
      prevX = x;
      prevY = y;
    }
  }

  /* === draw cursor === */
  if (graphCursMode) {
    uint8_t graphCursOnScreen = min(max((int16_t)graphCurs - graphCursI, 0), SCREEN_WIDTH);
    display->drawLine(graphCursOnScreen, DISPLAYPADDINGTOP, graphCursOnScreen, SCREEN_HEIGHT, SSD1306_WHITE);
    
    display->fillRect(0, 0, SCREEN_WIDTH, DISPLAYPADDINGTOP, SSD1306_BLACK);

    display->setCursor(0, 0);
    display->setTextColor(SSD1306_WHITE);  // Draw white text
    display->setTextSize(1);

    // // draw current X position as -time
    // int16_t curBackTime = (cycled ? MEASDATALENGTH : (curs + 1)) - graphCurs;
    int16_t curBackTime = measData.count() - (int16_t)graphCurs;
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
      // dtostrf(getTemp(cycled ? ((curs + graphCurs) % MEASDATALENGTH) : (graphCurs - 1)), 6, 2, tempStrBuf);
      dtostrf(getTemp(graphCurs), 6, 2, tempStrBuf);
      display->print(tempStrBuf);
    }
  }

  forceMenuRedraw = false;
  display->display();
}
void graphCursorMove(const int8_t dir) {
  if ((int16_t)graphCurs + dir < 0) {
    graphCurs = measData.length() - 1;
  } else if ((int16_t)graphCurs + dir >= measData.length()) {
    graphCurs = 0;
  } else graphCurs += dir;
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
      formatBackTime(measData.length() * getStoreTempTimerInterval(settingsOptsGraphCur), tempStrBuf, 0);
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
  
  // update global min/max
  if (temp < measMinG) measMinG = temp;
  if (temp > measMaxG) measMaxG = temp;

  // store
  pushTemp(temp);
  measChanged = true;

  recalcMinMaxRange();
}

void recalcMinMaxRange() {
  // recalc min/max
  measMin = DMAX;
  measMax = DMIN;
  // only consider visible min/max if not a graph cursor mode
  for (uint16_t i = graphCursMode ? 0 : max(0, (int16_t)measData.count() - SCREEN_WIDTH); i < measData.count(); i++) {
    float t = getTemp(i);
    if (t < measMin) measMin = t;
    if (t > measMax) measMax = t;
  }
  // recalc scale for graph
  float range = measMax - measMin;
  if (range >= 1) scale = (DISPLAYDATAHEIGHT - 1 - (GRAPHPADDINGTOP + GRAPHPADDINGBOT)) / range;  // use this for aligning graph vertically to the center
  else scale = 2.f;
}


// methods that perform temperature (un)normalization
// idx = {0 ... measData.count()-1}
float getTemp(const uint16_t idx) {
    // returns degrees of Celsium
    uint8_t temp = measData.get(idx);
    return (float)temp * MP_RANGE / MP_CAPN + MP_MIN;
}
void pushTemp(float temp) {
    // takes degrees of Celsium
    float tShifted = constrain(temp, MP_MIN, MP_MAX) - MP_MIN;
    uint8_t tMeas = (uint8_t)(tShifted * MP_CAPN / MP_RANGE);
    measData.push(tMeas);
}


void dbg_begin() {
  destroyDisplay();
  displayEnabled = false;
  uart.begin(9600);
}
void dbg_end() {
  uart.end();
  display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  initDisplay(false);
  displayEnabled = true;
}