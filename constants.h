// Main constants
#define TEMPAVERAGEN 20  // [0..255] - number of intermediate measurements
// #define PSEUDOFLOATPRECISION 10
#define MEASDATALENGTH 100  // bigger than min(SCREEN_WIDTH, 254) is not supported yet!

//#define EEPROMMEASBITS 5  // [1..8] - how many bits to use for each measurement when storing in EEPROM
//#define EEPROMMEASRANGE 32  // [2^EEPROMMEASBITS .. 256] - what range of temperatures (in degrees of celsium) each measurement covers
//#define EEPROMMEASMIN 0  // [-128 .. 127-EEPROMMEASRANGE] - minimum measured temperature
//#define EEPROMDATALASTIND 1015  // do not change!
//#define EEPROMLENGTH EEPROM.length()
#define EEPROMDATASTARTINDEX 5
#define EEPROMDATAENDINDEX EEPROM.length() - 2

// Hardware constants
#define THERMISTORPIN 0  // analog input pin for thermistor
#define R1 10000  // value of pull up resistor connected to thermistor
#define BTNLEFT_PIN 3
#define BTNRIGHT_PIN 2
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define BTNDEBOUNCE 40

// Timeouts
#define TEMPSHOWINTERVAL 500
#define TEMPSTOREINTERVALDEFAULT 1000
#define EEPROMSAVEMENUTIMEOUT 5000
#define BTNCLICKTIMEOUT 150  // timeout between consequent clicks
#define BTNHOLDTIMEOUT 400  // timeout for holding a button
#define BTNSTEPTIMEOUT 80  // timout between steps while holding button

// Display constants
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define DISPLAYPADDINGTOP 16
#define DISPLAYDATAHEIGHT SCREEN_HEIGHT - DISPLAYPADDINGTOP  // SCREEN_HEIGHT - DISPLAYPADDINGTOP
// #define GRAPHYOFFSET (64 - 16) / 2  // DISPLAYDATAHEIGHT / 2
#define DISPLAYLOGODURATION 1500
#define GRAPHPADDINGTOP 2  // extra padding for graph area (to make graph cursor visually more recognizable)
#define GRAPHPADDINGBOT 2

// Menu constants
#define MENULIVE      0
#define MENUGRAPH     1
#define MENUSETTINGS  2

// Menu: Graph constants
#define GRAPHBTNSTEPS4SPEED 15  // steps after which speed is increased by GRAPHBTNSPEED
#define GRAPHBTNSPEED 2

// Menu: Settings
#define SETTINGSDIMMING         0
#define SETTINGSGRAPHTIMEOUT    1
#define SETTINGSRESET           2
#define SETTINGSSAVE            3
#define SETTINGSLOAD            4
#define SETTINGSUSB             5

// USB
#define USBCMD_SENDDATA         0x09
#define USBCMD_SENDEEPROM       0x0F
#define USBCMD_SENDLIVE         0x12


// Steinhart–Hart equation (for thermistor) constants
#define C1 1.009249522e-03
#define C2 2.378405444e-04
#define C3 2.019202697e-07