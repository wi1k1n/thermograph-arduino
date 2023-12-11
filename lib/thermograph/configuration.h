#ifndef CONFIGURATION_H__
#define CONFIGURATION_H__

// UART
#define SERIAL_SPEED 115200

// Display
#define DISPLAY_WIDTH 		128
#define DISPLAY_HEIGHT 		128 			// Change this to 96 for 1.27" OLED.
#define DISPLAY_PIN_SCLK 	GPIO_NUM_18 	// CLK (or use constant SCK)
#define DISPLAY_PIN_MOSI 	GPIO_NUM_23 	// DIN (or use constant MOSI)
#define DISPLAY_PIN_DC 		GPIO_NUM_2
#define DISPLAY_PIN_CS 		GPIO_NUM_0
#define DISPLAY_PIN_RST 	GPIO_NUM_4

#endif // CONFIGURATION_H__