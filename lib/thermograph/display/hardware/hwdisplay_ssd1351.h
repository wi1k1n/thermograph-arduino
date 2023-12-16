#ifndef HWDISPLAY_SSD1351_H__
#define HWDISPLAY_SSD1351_H__

#include <Arduino.h>
#include <Adafruit_SSD1351.h>

#include <memory>

#include "display/hardware/hwdisplay_interface.h"

class HWDisplaySSD1351 : public HWDisplayInterface {
public:
	HWDisplaySSD1351() = default;
	
	bool init() override;
	void tick() override;
	void disable() override;
	void display() override;

	bool isEnabled() const override;
	
	// void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
	// void fillScreen(uint16_t color) override;

	// void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override;
	// void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;

	// void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) override;
	// void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) override;
	// void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) override;
	// void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color) override;
	// void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) override;
	// void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) override;
	// void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) override;
	// void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) override;
	void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color) override;
	void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg) override;
	void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) override;
	void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg) override;
	// void drawXBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color) override;
	// void drawGrayscaleBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h) override;
	// void drawGrayscaleBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h) override;
	// void drawGrayscaleBitmap(int16_t x, int16_t y, const uint8_t bitmap[], const uint8_t mask[], int16_t w, int16_t h) override;
	// void drawGrayscaleBitmap(int16_t x, int16_t y, uint8_t *bitmap, uint8_t *mask, int16_t w, int16_t h) override;
	// void drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h) override;
	// void drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h) override;
	// void drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], int16_t w, int16_t h) override;
	// void drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, uint8_t *mask, int16_t w, int16_t h) override;
	// void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) override;
	// void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y) override;
	// void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) override;
	// void getTextBounds(const __FlashStringHelper *s, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) override;
	// void getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) override;
	// void setTextSize(uint8_t s) override;
	// void setTextSize(uint8_t sx, uint8_t sy) override;
	// void setFont(const GFXfont *f = NULL) override;
	// void setCursor(int16_t x, int16_t y) override;
	// void setTextColor(uint16_t c) override;
	// void setTextColor(uint16_t c, uint16_t bg) override;
	// void setTextWrap(bool w) override;

private:
	std::unique_ptr<Adafruit_SSD1351> _displayHW;
	
	const uint8_t BLACK = 0x0000;
	const uint8_t GREEN = 0x07E0;
	const uint8_t WHITE = 0xFFFF;
};

#endif // HWDISPLAY_SSD1351_H__