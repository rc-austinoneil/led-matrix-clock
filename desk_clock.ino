#include "RTClib.h"
#include <MatrixHardware_ESP32_V0.h>    
#include <SmartMatrix.h>


#define COLOR_DEPTH 24                  // Choose the color depth used for storing pixels in the layers: 24 or 48 (24 is good for most sketches - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24)
const uint16_t kMatrixWidth = 64;       // Set to the width of your display, must be a multiple of 8
const uint16_t kMatrixHeight = 32;      // Set to the height of your display
const uint8_t kRefreshDepth = 36;       // Tradeoff of color quality vs refresh rate, max brightness, and RAM usage.  36 is typically good, drop down to 24 if you need to.  On Teensy, multiples of 3, up to 48: 3, 6, 9, 12, 15, 25, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48.  On ESP32: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save RAM, more to keep from dropping frames and automatically lowering refresh rate.  (This isn't used on ESP32, leave as default)
const uint8_t kPanelType = SM_PANELTYPE_HUB75_32ROW_MOD16SCAN;   // Choose the configuration that matches your panels.  See more details in MatrixCommonHub75.h and the docs: https://github.com/pixelmatix/SmartMatrix/wiki
const uint32_t kMatrixOptions = (SM_HUB75_OPTIONS_NONE);        // see docs for options: https://github.com/pixelmatix/SmartMatrix/wiki
const uint8_t kIndexedLayerOptions = (SM_INDEXED_OPTIONS_NONE);



SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer1, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer2, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer3, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);

RTC_DS1307 rtc;
const int defaultBrightness = (30*255)/100;     // dim: 35% brightness
char monthsOfTheYr[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jly", "Aug", "Spt", "Oct", "Nov", "Dec"};
char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tues", "Wed", "Thurs", "Fri", "Sat"};


void setup() {
  Serial.begin(57600);
  delay(1000);
  Serial.println("DS1307RTC Test");
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // Setup Matrix
  matrix.addLayer(&indexedLayer1);
  matrix.addLayer(&indexedLayer2);
  matrix.addLayer(&indexedLayer3);
  matrix.setRotation(rotation180);
  matrix.setBrightness(defaultBrightness);
  matrix.begin();
}

void loop() {
  // clear screen before writing new text
  indexedLayer1.fillScreen(0);
  indexedLayer2.fillScreen(0);
  indexedLayer3.fillScreen(0);
  DateTime now = rtc.now();
  String today = daysOfTheWeek[now.dayOfTheWeek()];

  if (((today == "Mon" || today == "Tues" || today == "Wed" || today == "Thurs") && ((now.hour() >= 8) && (now.hour() < 17))) || (today == "Sun" && ((now.hour() >= 15) && (now.hour() < 23)))) {
      work();
  }
  else {
      off_work();
  }
  delay(1000);
  }


void off_work() {
    char txtBuffer[12];
    DateTime now = rtc.now();
    
    //Time
    sprintf(txtBuffer, "%02d:%02d", now.hour(), now.minute());
    indexedLayer2.setFont(font8x13);
    indexedLayer2.setIndexedColor(1,{0x66, 0x02, 0x3c});
    indexedLayer2.drawString(13, 7, 1, txtBuffer);
    indexedLayer2.swapBuffers();
    // Date
    sprintf(txtBuffer, "%s %02d %04d", monthsOfTheYr[(now.month()-1)], now.day(), now.year());
    indexedLayer3.setFont(font5x7);
    indexedLayer3.setIndexedColor(1,{0x40,0x40,0x40});
    indexedLayer3.drawString(5, 20, 1, txtBuffer); 
    indexedLayer3.swapBuffers();
    // Blank
    sprintf(txtBuffer, "");
    indexedLayer1.setFont(font3x5);
    indexedLayer1.setIndexedColor(1,{0x66, 0x02, 0x3c});
    indexedLayer1.drawString(2.5, 25, 1, txtBuffer);
    indexedLayer1.swapBuffers();
}

void work() {
    DateTime now = rtc.now();
    char txtBuffer[12];
    char hours_left[20];
    int h;
    int m = (60 - now.minute());
    String today = daysOfTheWeek[now.dayOfTheWeek()];

    // Select Correct Shift End Time
    if (today == "Mon" || today == "Tues" || today == "Wed" || today == "Thurs") {
        h = (16 - (now.hour()));
    } 
    else if (today == "Sun") {
        h = (22 - (now.hour()));
    }

    // Time
    sprintf(txtBuffer, "%02d:%02d", now.hour(), now.minute());
    indexedLayer2.setFont(font8x13);
    indexedLayer2.setIndexedColor(1,{0x66, 0x02, 0x3c});
    indexedLayer2.drawString(13, 1, 1, txtBuffer);
    indexedLayer2.swapBuffers();

    // Date
    sprintf(txtBuffer, "%s %02d %04d", monthsOfTheYr[(now.month()-1)], now.day(), now.year());
    indexedLayer3.setFont(font5x7);
    indexedLayer3.setIndexedColor(1,{0x40,0x40,0x40});
    indexedLayer3.drawString(5, 14, 1, txtBuffer); 
    indexedLayer3.swapBuffers();

    // Time Left
    sprintf(hours_left, "%iHr %iMins Left", h, m);
    indexedLayer1.setFont(font3x5);
    indexedLayer1.setIndexedColor(1,{0x66, 0x02, 0x3c});
    indexedLayer1.drawString(2.5, 25, 1, hours_left);
    indexedLayer1.swapBuffers();
}