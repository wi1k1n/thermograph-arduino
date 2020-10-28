// Main constants
#define TEMPAVERAGEN 20  // [0..63] - number of intermediate measurements
// #define MEASDATALENGTH 80  // bigger than min(SCREEN_WIDTH, 254) is not supported yet!
#define MEASDATABYTELENGTH 80  // number of bytes to allocate (final available length: MEASDATABYTELENGTH * 8 / MP_CAP)

// Manual precision constants
#define MP_CAP 4  // manual precision capacity: {3, 4, 5, 6, 7, 8}
#define MP_MIN 17  // manual precision minimum: {-61 ... 127}
#define MP_MAX 32  // manual precision maximum: {-56 ... 132}
#define MP_RANGE (MP_MAX-MP_MIN)
#define MP_CAPN (0xFF >> (8 - MP_CAP))  // max number of different values using MP_CAP bits

// EEPROM indices
#define EEPROMDATASTARTINDEX 7
#define EEPROMDATAENDINDEX EEPROM.length() - 2

// Hardware constants
#define R1 10000  // value of pull up resistor connected to thermistor
// Limit measures to the following values
#define TEMPMIN -61  // minimum possible temperature that can be measured
#define TEMPMIN_A 14  // value on 

// Pins
#define THERMISTORPIN 0  // analog input pin for thermistor
#define BTNLEFT_PIN 3
#define BTNRIGHT_PIN 2
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

// Timeouts
#define TEMPSHOWINTERVAL 500
#define TEMPSTOREINTERVALDEFAULT 1000
#define EEPROMSAVEMENUTIMEOUT 5000
#define BTNDEBOUNCE 40
#define BTNCLICKTIMEOUT 150  // timeout between consequent clicks
#define BTNHOLDTIMEOUT 400  // timeout for holding a button
#define BTNSTEPTIMEOUT 80  // timout between steps while holding button

// Display constants
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define DISPLAYPADDINGTOP 16
#define DISPLAYDATAHEIGHT (SCREEN_HEIGHT - DISPLAYPADDINGTOP)  // SCREEN_HEIGHT - DISPLAYPADDINGTOP
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
#define USBCMD_PING             0x02
#define USBCMD_SENDDATA         0x09
#define USBCMD_SENDEEPROM       0x0F
#define USBCMD_SENDLIVESTART    0x12
#define USBCMD_SENDLIVESTOP     0x13

#define USBSTATUS_BEGIN         (F("begin"))
#define USBSTATUS_END           (F("end"))
#define USBSTATUS_PONG          (F("pong"))



// Thermistor definitions
#define AMIN 14  // minimum valid thermistor value
#define AMAX 985  // maximum valid thermistor value
// #define ARNG (AMAX-AMIN)

#define C1 1.009249522e-03  // Steinhart–Hart equation (for thermistor) constants
#define C2 2.378405444e-04
#define C3 2.019202697e-07

#define LR2(a) (log(R1 * (1023.f - a) / a))
// min/max valid values (in degrees)
#define DMIN (1.f / (C1 + C2*LR2(AMIN) + C3*LR2(AMIN)*LR2(AMIN)*LR2(AMIN)) - 273.15)
#define DMAX (1.f / (C1 + C2*LR2(AMAX) + C3*LR2(AMAX)*LR2(AMAX)*LR2(AMAX)) - 273.15)
// #define DRNG (DMAX-DMIN)

#define GETTEMP (constrain(analogRead(THERMISTORPIN), AMIN, AMAX))