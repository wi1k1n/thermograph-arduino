#include "thermograph.h"
#include "configuration.h"
#include "version.h"
#include "sdk.h"

#include "FS.h"
#include "LittleFS.h"

#include <functional>

#define FORMAT_LITTLEFS_IF_FAILED true

static bool initWrapper(const char* moduleName, bool initResult) {
	LOGLN("Initializing ", moduleName, " -> ", initResult ? "done" : "failed");
	return initResult;
};

void ThermographApp::init() {
	Serial.begin(SERIAL_SPEED);
	LOGLN("Thermograph v", THERMOGRAPH_VERSION_MAJOR, ".", THERMOGRAPH_VERSION_MINOR);

	if (!initWrapper("DisplayManager", _displayManager.init()))
		return;
	_displayManager.activateLayout(DisplayLayoutKey::WELCOME);
	
	testDisplay();
	// testLFS();
}
void ThermographApp::tick() {
	_displayManager.tick();
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}
void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}
void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}
void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}
void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();
}
void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
    file.close();
}
void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\r\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("- file renamed");
    } else {
        Serial.println("- rename failed");
    }
}
void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}
void testFileIO(fs::FS &fs, const char * path){
    Serial.printf("Testing file I/O with %s\r\n", path);

    static uint8_t buf[512];
    size_t len = 0;
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }

    size_t i;
    Serial.print("- writing" );
    uint32_t start = millis();
    for(i=0; i<2048; i++){
        if ((i & 0x001F) == 0x001F){
          Serial.print(".");
        }
        file.write(buf, 512);
    }
    Serial.println("");
    uint32_t end = millis() - start;
    Serial.printf(" - %u bytes written in %u ms\r\n", 2048 * 512, end);
    file.close();

    file = fs.open(path);
    start = millis();
    end = start;
    i = 0;
    if(file && !file.isDirectory()){
        len = file.size();
        size_t flen = len;
        start = millis();
        Serial.print("- reading" );
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            if ((i++ & 0x001F) == 0x001F){
              Serial.print(".");
            }
            len -= toRead;
        }
        Serial.println("");
        end = millis() - start;
        Serial.printf("- %u bytes read in %u ms\r\n", flen, end);
        file.close();
    } else {
        Serial.println("- failed to open file for reading");
    }
}
void ThermographApp::testLFS() {
    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LittleFS Mount Failed");
        return;
    }

    createDir(LittleFS, "/mydir"); // Create a mydir folder
    writeFile(LittleFS, "/mydir/hello1.txt", "Hello1"); // Create a hello1.txt file with the content "Hello1"
    listDir(LittleFS, "/", 1); // List the directories up to one level beginning at the root directory
    deleteFile(LittleFS, "/mydir/hello1.txt"); //delete the previously created file
    removeDir(LittleFS, "/mydir"); //delete the previously created folder
    listDir(LittleFS, "/", 1); // list all directories to make sure they were deleted
    
    writeFile(LittleFS, "/hello.txt", "Hello "); //Create and write a new file in the root directory
    appendFile(LittleFS, "/hello.txt", "World!\r\n"); //Append some text to the previous file
    readFile(LittleFS, "/hello.txt"); // Read the complete file
    renameFile(LittleFS, "/hello.txt", "/foo.txt"); //Rename the previous file
    readFile(LittleFS, "/foo.txt"); //Read the file with the new name
    deleteFile(LittleFS, "/foo.txt"); //Delete the file
    testFileIO(LittleFS, "/test.txt"); //Testin
    deleteFile(LittleFS, "/test.txt"); //Delete the file
  
    Serial.println( "Test complete" ); 
}

void ThermographApp::testDisplay() {


	// return;
	// tft.begin();

	// Serial.println("init");

	// // You can optionally rotate the display by running the line below.
	// // Note that a value of 0 means no rotation, 1 means 90 clockwise,
	// // 2 means 180 degrees clockwise, and 3 means 270 degrees clockwise.
	// //tft.setRotation(1);
	// // NOTE: The test pattern at the start will NOT be rotated!  The code
	// // for rendering the test pattern talks directly to the display and
	// // ignores any rotation.

	// uint16_t time = millis();
	// tft.fillRect(0, 0, 128, 128, BLACK);
	// time = millis() - time;
	
	// Serial.println(time, DEC);
	// delay(500);
	
	// lcdTestPattern();
	// delay(500);
	
	// tft.invert(true);
	// delay(100);
	// tft.invert(false);
	// delay(100);

	// tft.fillScreen(BLACK);
	// tft.setRotation(2);
	// testdrawtext("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur adipiscing ante sed nibh tincidunt feugiat. Maecenas enim massa, fringilla sed malesuada et, malesuada sit amet turpis. Sed porttitor neque ut ante pretium vitae malesuada nunc bibendum. Nullam aliquet ultrices massa eu hendrerit. Ut sed nisi lorem. In vestibulum purus a tortor imperdiet posuere. ", WHITE);
	// delay(500);

	// // tft print function!
	// tftPrintTest();
	// delay(500);
	
	// //a single pixel
	// tft.drawPixel(tft.width()/2, tft.height()/2, GREEN);
	// delay(500);

	// // line draw test
	// testlines(YELLOW);
	// delay(500);    
 
	// // optimized lines
	// testfastlines(RED, BLUE);
	// delay(500);    


	// testdrawrects(GREEN);
	// delay(1000);

	// testfillrects(YELLOW, MAGENTA);
	// delay(1000);

	// tft.fillScreen(BLACK);
	// testfillcircles(10, BLUE);
	// testdrawcircles(10, WHITE);
	// delay(1000);
	 
	// testroundrects();
	// delay(500);
	
	// testtriangles();
	// delay(500);
	
	// Serial.println("done");
	// delay(1000);
}


// Option 2: must use the hardware SPI pins 
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be 
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)
//Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);

// float p = 3.1415926;
// void ThermographApp::testlines(uint16_t color) {
// 	 tft.fillScreen(BLACK);
// 	 for (uint16_t x=0; x < tft.width()-1; x+=6) {
// 		 tft.drawLine(0, 0, x, tft.height()-1, color);
// 	 }
// 	 for (uint16_t y=0; y < tft.height()-1; y+=6) {
// 		 tft.drawLine(0, 0, tft.width()-1, y, color);
// 	 }
	 
// 	 tft.fillScreen(BLACK);
// 	 for (uint16_t x=0; x < tft.width()-1; x+=6) {
// 		 tft.drawLine(tft.width()-1, 0, x, tft.height()-1, color);
// 	 }
// 	 for (uint16_t y=0; y < tft.height()-1; y+=6) {
// 		 tft.drawLine(tft.width()-1, 0, 0, y, color);
// 	 }
	 
// 	 tft.fillScreen(BLACK);
// 	 for (uint16_t x=0; x < tft.width()-1; x+=6) {
// 		 tft.drawLine(0, tft.height()-1, x, 0, color);
// 	 }
// 	 for (uint16_t y=0; y < tft.height()-1; y+=6) {
// 		 tft.drawLine(0, tft.height()-1, tft.width()-1, y, color);
// 	 }

// 	 tft.fillScreen(BLACK);
// 	 for (uint16_t x=0; x < tft.width()-1; x+=6) {
// 		 tft.drawLine(tft.width()-1, tft.height()-1, x, 0, color);
// 	 }
// 	 for (uint16_t y=0; y < tft.height()-1; y+=6) {
// 		 tft.drawLine(tft.width()-1, tft.height()-1, 0, y, color);
// 	 }
	 
// }

// void ThermographApp::testdrawtext(char *text, uint16_t color) {
// 	tft.setCursor(0,0);
// 	tft.setTextColor(color);
// 	tft.print(text);
// }

// void ThermographApp::testfastlines(uint16_t color1, uint16_t color2) {
// 	 tft.fillScreen(BLACK);
// 	 for (uint16_t y=0; y < tft.height()-1; y+=5) {
// 		 tft.drawFastHLine(0, y, tft.width()-1, color1);
// 	 }
// 	 for (uint16_t x=0; x < tft.width()-1; x+=5) {
// 		 tft.drawFastVLine(x, 0, tft.height()-1, color2);
// 	 }
// }

// void ThermographApp::testdrawrects(uint16_t color) {
//  tft.fillScreen(BLACK);
//  for (uint16_t x=0; x < tft.height()-1; x+=6) {
// 	 tft.drawRect((tft.width()-1)/2 -x/2, (tft.height()-1)/2 -x/2 , x, x, color);
//  }
// }

// void ThermographApp::testfillrects(uint16_t color1, uint16_t color2) {
//  tft.fillScreen(BLACK);
//  for (uint16_t x=tft.height()-1; x > 6; x-=6) {
// 	 tft.fillRect((tft.width()-1)/2 -x/2, (tft.height()-1)/2 -x/2 , x, x, color1);
// 	 tft.drawRect((tft.width()-1)/2 -x/2, (tft.height()-1)/2 -x/2 , x, x, color2);
//  }
// }

// void ThermographApp::testfillcircles(uint8_t radius, uint16_t color) {
// 	for (uint8_t x=radius; x < tft.width()-1; x+=radius*2) {
// 		for (uint8_t y=radius; y < tft.height()-1; y+=radius*2) {
// 			tft.fillCircle(x, y, radius, color);
// 		}
// 	}  
// }

// void ThermographApp::testdrawcircles(uint8_t radius, uint16_t color) {
// 	for (uint8_t x=0; x < tft.width()-1+radius; x+=radius*2) {
// 		for (uint8_t y=0; y < tft.height()-1+radius; y+=radius*2) {
// 			tft.drawCircle(x, y, radius, color);
// 		}
// 	}  
// }

// void ThermographApp::testtriangles() {
// 	tft.fillScreen(BLACK);
// 	int color = 0xF800;
// 	int t;
// 	int w = tft.width()/2;
// 	int x = tft.height();
// 	int y = 0;
// 	int z = tft.width();
// 	for(t = 0 ; t <= 15; t+=1) {
// 		tft.drawTriangle(w, y, y, x, z, x, color);
// 		x-=4;
// 		y+=4;
// 		z-=4;
// 		color+=100;
// 	}
// }

// void ThermographApp::testroundrects() {
// 	tft.fillScreen(BLACK);
// 	int color = 100;
	
// 	int x = 0;
// 	int y = 0;
// 	int w = tft.width();
// 	int h = tft.height();
// 	for(int i = 0 ; i <= 24; i++) {
// 		tft.drawRoundRect(x, y, w, h, 5, color);
// 		x+=2;
// 		y+=3;
// 		w-=4;
// 		h-=6;
// 		color+=1100;
// 		Serial.println(i);
// 	}
// }

// void ThermographApp::tftPrintTest() {
// 	tft.fillScreen(BLACK);
// 	tft.setCursor(0, 5);
// 	tft.setTextColor(RED);  
// 	tft.setTextSize(1);
// 	tft.println("Hello World!");
// 	tft.setTextColor(YELLOW);
// 	tft.setTextSize(2);
// 	tft.println("Hello World!");
// 	tft.setTextColor(BLUE);
// 	tft.setTextSize(3);
// 	tft.print(1234.567);
// 	delay(1500);
// 	tft.setCursor(0, 5);
// 	tft.fillScreen(BLACK);
// 	tft.setTextColor(WHITE);
// 	tft.setTextSize(0);
// 	tft.println("Hello World!");
// 	tft.setTextSize(1);
// 	tft.setTextColor(GREEN);
// 	tft.print(p, 6);
// 	tft.println(" Want pi?");
// 	tft.println(" ");
// 	tft.print(8675309, HEX); // print 8,675,309 out in HEX!
// 	tft.println(" Print HEX!");
// 	tft.println(" ");
// 	tft.setTextColor(WHITE);
// 	tft.println("Sketch has been");
// 	tft.println("running for: ");
// 	tft.setTextColor(MAGENTA);
// 	tft.print(millis() / 1000);
// 	tft.setTextColor(WHITE);
// 	tft.print(" seconds.");
// }

// void ThermographApp::mediabuttons() {
//  // play
// 	tft.fillScreen(BLACK);
// 	tft.fillRoundRect(25, 10, 78, 60, 8, WHITE);
// 	tft.fillTriangle(42, 20, 42, 60, 90, 40, RED);
// 	delay(500);
// 	// pause
// 	tft.fillRoundRect(25, 90, 78, 60, 8, WHITE);
// 	tft.fillRoundRect(39, 98, 20, 45, 5, GREEN);
// 	tft.fillRoundRect(69, 98, 20, 45, 5, GREEN);
// 	delay(500);
// 	// play color
// 	tft.fillTriangle(42, 20, 42, 60, 90, 40, BLUE);
// 	delay(50);
// 	// pause color
// 	tft.fillRoundRect(39, 98, 20, 45, 5, RED);
// 	tft.fillRoundRect(69, 98, 20, 45, 5, RED);
// 	// play color
// 	tft.fillTriangle(42, 20, 42, 60, 90, 40, GREEN);
// }

// /**************************************************************************/
// /*! 
// 		@brief  Renders a simple test pattern on the screen
// */
// /**************************************************************************/
// void ThermographApp::lcdTestPattern(void)
// {
// 	static const uint16_t PROGMEM colors[] =
// 		{ RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA, BLACK, WHITE };

// 	for(uint8_t c=0; c<8; c++) {
// 		tft.fillRect(0, tft.height() * c / 8, tft.width(), tft.height() / 8,
// 			pgm_read_word(&colors[c]));
// 	}
// }