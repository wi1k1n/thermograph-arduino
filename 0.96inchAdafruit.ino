// Settings constants
#define TEMPSHOWINTERVAL 250
#define TEMPSTOREINTERVAL 250
#define TEMPAVERAGEN 10  // [0..255] - number of intermediate measurements

#define MEASDATALENGTH 128


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

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define DISPLAYPADDINGTOP 16
#define DISPLAYDATAHEIGHT SCREEN_HEIGHT-DISPLAYPADDINGTOP

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

// temperature variables
float tempValPartAverage;  // partially averaged value of analogRead(thermistor)
byte tempValN = 0;  // number of currently got measurements
// TODO: change this to int8 in a correct way
float temp;  // degrees of Celsium

int8_t measData[MEASDATALENGTH];
byte curs = 0;
bool cycled = false;
bool measChanged = true;
int8_t measMin = INT8_MAX;
byte measMinInd = 0;
byte measMaxInd = 0;
int8_t measMax = INT8_MIN;
byte scale = 1;



// Menu variables
byte menuState = 1;  // 0 - live temp, 1 - graph, 2 - settings

// EEPROM management variables
// x - not used, b - bit value
// [0..1015]    -> data
// [1022..1023] -> cursor value (12 bits: [xxxxbbbbbbbbbbbb])
// [1021]       -> resolution (8 bits: [bbbbbbbb])
// [1020]       -> range (8 bits)
// [1019]       -> min (8 bits)
//unsigned int eepromCursor = 0;

void setup() {
  Serial.begin(9600);

  Serial.print("MEASDATALENGTH: ");
  Serial.println(MEASDATALENGTH);

  initDisplay();

  timerShowTemp.setInterval(TEMPSHOWINTERVAL);
  timerStoreTemp.setInterval(TEMPSTOREINTERVAL);
  timerGetTemp.setInterval((min(TEMPSHOWINTERVAL, TEMPSTOREINTERVAL) - 1) / TEMPAVERAGEN);
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

  bool menuJustChanged = false;
  if (btnL.isSingle()) {}
  if (btnR.isSingle()) {
    menuState = (menuState + 1) % 3;
    menuJustChanged = true;
  }

  if (menuState == 0) {
    displayLive(menuJustChanged);
  } else if (menuState == 1) {
    displayGraph(menuJustChanged);
  } else {
    displaySettings();
  }
}

void initDisplay() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // 0x3c - display address
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(1000);
  
  display.cp437(true);  // Use full 256 char 'Code Page 437' font
  // Clear the buffer
  display.clearDisplay();
}

// show live temperature on display
void displayLive(bool menuJustChanged) {
  // skip execution until show timer is ready
  if (!menuJustChanged && !timerShowTemp.isReady())
    return;
  
  // calc averaged value of temperature
  temp = tempValN == 0 ? measData[curs - 1] : temp;
  
//  Serial.print("showTemp:  ");
//  Serial.println(temp);
  
  display.clearDisplay();
  
  display.setCursor(0, DISPLAYPADDINGTOP);
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setTextSize(3);
  display.print(temp);

  display.setCursor(display.getCursorX(), DISPLAYPADDINGTOP - 4);
  display.setTextSize(2);
  display.print('o');
  display.display();
}
void displayGraph(bool menuJustChanged) {
  if (timerShowTemp.isReady()) {
    display.clearDisplay();
    
    /* === draw caption === */
    display.setCursor(0, 0);
    display.setTextColor(SSD1306_WHITE);  // Draw white text
    display.setTextSize(2);
    display.print(temp);
  }

  // skip execution if no new data or not forced by menyJustChanged
  if (!measChanged && !menuJustChanged) return;
  measChanged = false;

  /* === draw graph === */
  // i iterates from 0 to cursor (or from cursor+1 up to cursor, wrapping around the MEASDATALENGTH)
  byte i = cycled ? (curs + 1) : 0;
  // Serial.print("curs: ");
  // Serial.print(curs);
  // Serial.print("; i: ");
  // Serial.println(i);
  byte prevX = 0;
  // constrain is needed to correctly fit into screen
  byte prevY = constrain((measData[0] - measMin) * scale, 0, DISPLAYDATAHEIGHT - 1) + DISPLAYPADDINGTOP;
  for (byte x = 1;; i++, x++) {
    if (i == MEASDATALENGTH) i = 0;  // wrap around i
    if (i == curs) break;  // stop condition
    byte y = constrain((measData[i] - measMin) * scale, 0, DISPLAYDATAHEIGHT - 1) + DISPLAYPADDINGTOP;
    // Serial.println(y);
    display.drawLine(prevX, prevY, x, y, SSD1306_WHITE);
    prevX = x;
    prevY = y;
  }
  // Serial.print(" -> ");
  // Serial.println(i);
  
  display.display();
}
void displaySettings() {
  display.clearDisplay();
  
  display.setCursor(0, 16);
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setTextSize(2);
  display.print("Settings!");
  
  display.display();
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
  
  // store
  measData[curs] = temp;
  
  if ((int)temp < measMin) {
    measMin = temp;
    measMinInd = curs;
    byte range = measMax - measMin;
    if (range > 0) scale = DISPLAYDATAHEIGHT / range;
    Serial.print("minUpd: ");
    Serial.print(measMin);
    Serial.print("; scale: ");
    Serial.println(scale);
  }
  if ((int)temp > measMax) {
    measMax = temp;
    measMaxInd = curs;
    byte range = measMax - measMin;
    if (range > 0) scale = DISPLAYDATAHEIGHT / range;
    Serial.print("maxUpd: ");
    Serial.print(measMax);
    Serial.print("; scale: ");
    Serial.println(scale);
  }
  measChanged = true;
  if (curs == MEASDATALENGTH - 1) {
    cycled = true;
    Serial.println("cycled!");
    curs = 0;
  } else {
    curs++;
  }
}
