// Settings constants
#define TEMPSHOWINTERVAL 500
#define TEMPSTOREINTERVAL 500
#define TEMPAVERAGEN 20  // [0..255] - number of intermediate measurements
#define MEASDATALENGTH 32

#define DISPLAYDIMTIMEOUTDEFAULT 15000
// #define DISPLAYDIMENABLED  // uncomment this to enable


//#define EEPROMMEASBITS 5  // [1..8] - how many bits to use for each measurement when storing in EEPROM
//#define EEPROMMEASRANGE 32  // [2^EEPROMMEASBITS .. 256] - what range of temperatures (in degrees of celsium) each measurement covers
//#define EEPROMMEASMIN 0  // [-128 .. 127-EEPROMMEASRANGE] - minimum measured temperature
//#define EEPROMDATALASTIND 1015  // do not change!
//#define EEPROMLENGTH EEPROM.length()

// Hardware constants
#define THERMISTORPIN 0  // analog input pin for thermistor
#define R1 10000  // value of pull up resistor connected to thermistor
#define BTNLEFT_PIN 3
#define BTNRIGHT_PIN 2
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define BTNDEBOUNCE 40
#define BTNCLICKTIMEOUT 250  // timeout between consequent clicks
#define BTNHOLDTIMEOUT 500  // timeout for holding a button

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define DISPLAYPADDINGTOP 16
#define DISPLAYDATAHEIGHT 64-16  // SCREEN_HEIGHT - DISPLAYPADDINGTOP
// #define GRAPHYOFFSET (64 - 16) / 2  // DISPLAYDATAHEIGHT / 2
#define GRAPHPADDINGTOP 2
#define GRAPHPADDINGBOT 2

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <GyverButton.h>
#include <GyverTimer.h>
#include "thermistorMinim.h"
//#include <EEPROM.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

thermistor therm(THERMISTORPIN, R1);

GButton btnL(BTNLEFT_PIN, LOW_PULL);
GButton btnR(BTNRIGHT_PIN, LOW_PULL);

GTimer timerGetTemp;
GTimer timerShowTemp;
GTimer timerStoreTemp;
GTimer timerDisplayDim;

char tempStrBuf[6];
bool dimEnabled;
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
byte measMinB = INT8_MAX;
byte measMinInd = 0;
byte measMaxInd = 0;
float scale = 2.f;

// Menu variables
byte menuScreen = 1;  // 0 - live temp, 1 - graph, 2 - settings
bool forceMenuRedraw = false;  // can be set to force display methods to redraw. (display methods unset this flag!)
byte settingSelected = 0;
byte settingIsChanging = false;
// https://www.arduino.cc/reference/en/language/variables/utilities/progmem/
const char* const PROGMEM settingsOptsDimming[] = {"off ", " 5s ", "10s ", "15s ", "30s ", "60s "};
#define SETTINGSOPTSDIMMINGSIZE 6
byte settingsOptsDimmingCur = 0;
const char* const PROGMEM settingsOptsGraph[] = {"0.5s", "1s ", "5s ", "15s ", "30s ", " 1m ", " 2m ", " 5m "};
#define SETTINGSOPTSGRAPHSIZE 8
byte settingsOptsGraphCur = 0;

// EEPROM management variables
// x - not used, b - bit value
// [0..1015]    -> data
// [1022..1023] -> cursor value (12 bits: [xxxxbbbbbbbbbbbb])
// [1021]       -> resolution (8 bits: [bbbbbbbb])
// [1020]       -> range (8 bits)
// [1019]       -> min (8 bits)
//unsigned int eepromCursor = 0;

void setup() {
  Serial.begin(57600);

  // Serial.print("displayDimTimeout: ");
  // Serial.println(displayDimTimeout);
  // Serial.print("GRAPHYOFFSET: ");
  // Serial.println(GRAPHYOFFSET);

  initDisplay();
  initButtons();
  initTimers();
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
    storeMeasurement();    
  }

  if (btnL.isSingle()) {
    // Serial.println("btnL");
    // if button has been pushed while active (not sleeping)
    if (!displayWakeUp()) {
      // if settings screen
      if (menuScreen == 2) {
          // if in process of changing settings
          if (settingIsChanging) {
            incrementSettingSelected(-1);
          }
          // if not changing settings atm
          else {
            // Serial.println("btnL -> not changing");
            settingSelected = (settingSelected + 1) % 2;
          }
          forceMenuRedraw = true;
      }
    }
  }
  if (btnR.isSingle()) {
    // Serial.println("btnR");
    if (!displayWakeUp()) {
      if (menuScreen == 2) {
      // if in process of changing settings
        if (settingIsChanging) {
          incrementSettingSelected(1);
        } else {
          menuScreen = (menuScreen + 1) % 3;
        }
      }
      // if not changing settings
      else {
        menuScreen = (menuScreen + 1) % 3;
      }
      forceMenuRedraw = true;
    }
  }
  if (btnL.isHolded()) {
    // if settings screen
    if (menuScreen == 2) {
      acceptSettingsSelected();
      settingIsChanging = !settingIsChanging;
      forceMenuRedraw = true;
    }
  }

  if (menuScreen == 0) {
    displayLive(forceMenuRedraw);
  } else if (menuScreen == 1) {
    displayGraph(forceMenuRedraw);
  } else {
    displaySettings(forceMenuRedraw);
  }

  if (timerDisplayDim.isReady()) {
    dim(true);
  }
}

void initDisplay() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // 0x3c - display address
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.cp437(true);  // Use full 256 char 'Code Page 437' font
  
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(500);
  display.clearDisplay();
}
void initButtons() {
  btnL.setDebounce(BTNDEBOUNCE);
  btnR.setDebounce(BTNDEBOUNCE);
  btnL.setClickTimeout(BTNCLICKTIMEOUT);
  btnR.setClickTimeout(BTNCLICKTIMEOUT);
  btnL.setTimeout(BTNHOLDTIMEOUT);
  btnR.setTimeout(BTNHOLDTIMEOUT);
}
void initTimers() {
  timerShowTemp.setInterval(TEMPSHOWINTERVAL);
  timerStoreTemp.setInterval(TEMPSTOREINTERVAL);
  timerGetTemp.setInterval((min(TEMPSHOWINTERVAL, TEMPSTOREINTERVAL) - 1) / TEMPAVERAGEN);
  #ifdef DISPLAYDIMENABLED
    timerDisplayDim.setTimeout(displayDimTimeout);
    dimEnabled = true;
  #endif
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
void dim(bool v) {
  if (!dimEnabled) return;
  if (!v) {
    Serial.println(F("cl"));
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
  if (!dimEnabled) return;
  timerDisplayDim.start();
}

void incrementSettingSelected(int8_t dir) {
    int8_t newCur = settingsOptsDimmingCur + dir;
    settingsOptsDimmingCur = newCur < 0 ? (SETTINGSOPTSDIMMINGSIZE-1) : (newCur % SETTINGSOPTSDIMMINGSIZE);
}
void acceptSettingsSelected() {
  // Serial.println("accept");
  if (settingSelected == 0) {
    dimEnabled = true;
    if (settingsOptsDimmingCur == 1) timerDisplayDim.setInterval(5000);
    else if (settingsOptsDimmingCur == 2) timerDisplayDim.setInterval(10000);
    else if (settingsOptsDimmingCur == 3) timerDisplayDim.setInterval(15000);
    else if (settingsOptsDimmingCur == 4) timerDisplayDim.setInterval(30000);
    else if (settingsOptsDimmingCur == 5) timerDisplayDim.setInterval(60000);
    else dimEnabled = false;

    if (dimEnabled) {
      forceMenuRedraw = true;
      updateDimTimer();
    }
  } else {
  }
}

// show live temperature on display
void displayLive(bool forceMenuRedraw) {
  // do not waste cpu if display is isDimmed
  if (isDimmed) return;

  // skip execution until show timer is ready
  if (!forceMenuRedraw && !timerShowTemp.isReady())
    return;
  forceMenuRedraw = true;
  
  // calc averaged value of temperature
  dtostrf(temp, 6, 2, tempStrBuf);
  
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

  display.display();
}

void displayGraph(bool forceMenuRedraw) {
  if (isDimmed) return;

  /* === draw caption === */
  if (timerShowTemp.isReady() || forceMenuRedraw) {
    display.fillRect(0, 0, SCREEN_WIDTH, DISPLAYPADDINGTOP, SSD1306_BLACK);

    display.setCursor(0, 0);
    display.setTextColor(SSD1306_WHITE);  // Draw white text
    display.setTextSize(1);

    // do not show min/max until first storage iteration updates its default values
    if (cycled || curs > 0) {
      display.print(temp);
      display.print(" ");
      byte curX = display.getCursorX();
      display.print("l:");
      dtostrf(measMin, 6, 2, tempStrBuf);
      display.print(tempStrBuf);
      display.print(" ");
      dtostrf(measMax, 6, 2, tempStrBuf);
      display.print(tempStrBuf);
      display.setCursor(curX, 8);
      display.print("g:");
      dtostrf(measMinG, 6, 2, tempStrBuf);
      display.print(tempStrBuf);
      display.print(" ");
      dtostrf(measMaxG, 6, 2, tempStrBuf);
      display.print(tempStrBuf);
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
    byte prevY = SCREEN_HEIGHT - GRAPHPADDINGBOT - (measData[(i < MEASDATALENGTH ? i : 0)] - measMinB) * scale;
    for (byte x = 1;; i++, x++) {
      if (i == MEASDATALENGTH) i = 0;  // wrap around i
      if (i == curs) break;  // stop condition
      byte y = SCREEN_HEIGHT - GRAPHPADDINGBOT - (measData[i] - measMinB) * scale;
      display.drawLine(prevX, prevY, x, y, SSD1306_WHITE);
      prevX = x;
      prevY = y;
    }
  }

  forceMenuRedraw = true;
  display.display();
}
void displaySettings(bool forceMenuRedraw) {
  if (isDimmed) return;

  if (forceMenuRedraw) {
    display.clearDisplay();
    display.setTextSize(1);

    // dimming timeout
    display.setCursor(0, DISPLAYPADDINGTOP+1);
    if (!settingIsChanging && settingSelected == 0) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      display.drawRect(display.getCursorX(),display.getCursorY()-1, 15*5, 10, SSD1306_WHITE);
    }
    else {
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    }
    display.print(F("       Dimming:"));
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.print(" ");
    if (settingIsChanging && settingSelected == 0) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      display.drawRect(display.getCursorX(),display.getCursorY()-1, 4*5, 10, SSD1306_WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    }
    display.print((char*)pgm_read_word(&(settingsOptsDimming[settingsOptsDimmingCur])));
    
    display.setCursor(0, (DISPLAYPADDINGTOP+1) + 12);
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    if (!settingIsChanging && settingSelected == 1) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    }
    display.print(F(" Graph timeout:"));
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.print(" ");
    if (settingIsChanging && settingSelected == 1) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    }
    display.print((char*)pgm_read_word(&(settingsOptsGraph[settingsOptsGraphCur])));
    
    display.display();
  }
  forceMenuRedraw = true;
}

void measurePartial() {
  tempValPartAverage = (tempValPartAverage * tempValN + analogRead(THERMISTORPIN)) / (tempValN + 1);
  tempValN++;
}

void storeMeasurement() {
  // calculate temp out of partial average
  temp = therm.computeTemp(tempValPartAverage);
  
  // reset partial average variables
  tempValPartAverage = 0;
  tempValN = 0;
  
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
    measMinB = measMin;
    float range = measMax - measMin;
    if (range >= 0.5f) scale = (DISPLAYDATAHEIGHT - 1 - (GRAPHPADDINGTOP + GRAPHPADDINGBOT)) / range;  // use this for aligning graph vertically to the center
    else scale = 2.f;
    
    // Serial.print("; scale: ");
    // Serial.println(scale);
  }
}
