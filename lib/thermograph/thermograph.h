#ifndef THERMOGRAPH_H__
#define THERMOGRAPH_H__

#include <Arduino.h>

#include "display/display_manager.h"
#include "display/hardware/hwdisplay_ssd1351.h"

// Screen dimensions
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128 // Change this to 96 for 1.27" OLED.

#define SCLK_PIN GPIO_NUM_18 // CLK (or use constant SCK)
#define MOSI_PIN GPIO_NUM_23 // DIN (or use constant MOSI)
#define DC_PIN   GPIO_NUM_2
#define CS_PIN   GPIO_NUM_0
#define RST_PIN  GPIO_NUM_4

// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128 // Change this to 96 for 1.27" OLED.

#define SCLK_PIN GPIO_NUM_18 // CLK (or use constant SCK)
#define MOSI_PIN GPIO_NUM_23 // DIN (or use constant MOSI)
#define DC_PIN   GPIO_NUM_2
#define CS_PIN   GPIO_NUM_0
#define RST_PIN  GPIO_NUM_4

class ThermographApp {
public:
	enum class AppMode {
		NONE = 0,
		LIVEVIEW, // 
		STEALTH, // When autonomously woke up just to make a measurement
	};
public:
	ThermographApp() = default;

	void init();
	void tick();
private:
	void testLFS();
	void testDisplay();

	// void testlines(uint16_t color);
	// void testdrawtext(char *text, uint16_t color);
	// void testfastlines(uint16_t color1, uint16_t color2);
	// void testdrawrects(uint16_t color);
	// void testfillrects(uint16_t color1, uint16_t color2);
	// void testfillcircles(uint8_t radius, uint16_t color);
	// void testdrawcircles(uint8_t radius, uint16_t color);
	// void testtriangles();
	// void testroundrects();
	// void tftPrintTest();
	// void mediabuttons();
	// void lcdTestPattern(void);
private:
	// Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, CS_PIN, DC_PIN, MOSI_PIN, SCLK_PIN, RST_PIN);
	DisplayManager<HWDisplaySSD1351> _displayManager;
};

#endif // THERMOGRAPH_H__