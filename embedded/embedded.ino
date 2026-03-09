#include <TFT_eSPI.h>

#define ALPHAIGNORETHRESHOLD 10

#define FULLLCDWIDTH 320
#define FULLLCDHEIGHT 240

#define LCDWIDTH 265
#define LCDHEIGHT 208

#define TFT_MISO -1
#define TFT_MOSI  5
#define TFT_SCLK 18
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    4  // Data Command control pin
#define TFT_RST   2  // Reset pin (could connect to RST pin)
#define TFT_LED  19

#define THUMBSTICK_VCC 26
#define THUMBSTICK_GND 27

#define THUMBSTICK_Y    33
#define THUMBSTICK_X    32
#define THUMBSTICK_SW   25
#define SW_PIN 26

int globalfps;
int frameInterval;

TFT_eSPI tft = TFT_eSPI(FULLLCDHEIGHT, FULLLCDWIDTH);
TFT_eSprite tftSprite = TFT_eSprite(&tft);

extern "C" {
    #include "gameObjects.h"
    #include "iofuncs.h"
    #include "settings.h"
}


int nextTick;
void awaitNextTick() {
    while (millis() < nextTick);
    nextTick += frameInterval;
}

extern "C" int main();
extern "C" void updateIO() {
    tftSprite.pushSprite(0, 0);
}

extern "C" void* mallocDMA(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_DMA);
}

unsigned long nativeColor(pixel_t color) {
    return (color.r << 11) | ((color.g & 077) << 5) | ((color.b & 037));
}

void clearScreen(pixel_t color) {
    tft.fillScreen(nativeColor(color));
}

rect_t screenRect = (rect_t) {0, 0, LCDWIDTH, LCDHEIGHT};
extern "C" void drawImage(image_t* image, int x, int y, int width, int height) {
    rect_t destination = (rect_t) {
        .x = x,
        .y = y,
        .width = width,
        .height = height,
    };
    if (!rectsCollide(destination, screenRect)) return;
    float xStride = (float)image->width / destination.width;
    float yStride = (float)image->height / destination.height;
    int pixelSize = sizeof(pixel_t);
    int rowSize = image->width * pixelSize;
    uint8_t* bytes = (uint8_t*) image->pixels;
    for (int y = 0; y < destination.height; y++) {
        for (int x = 0; x < destination.width; x++) {
            int column = x * xStride;
            int row = y * yStride;
            bytes = (uint8_t*)image->pixels + (row * rowSize) + (column * pixelSize);
            uint8_t red = *(bytes++) * 31 / 255;
            uint8_t green = *(bytes++) * 63 / 255;
            uint8_t blue = *(bytes++) * 31 / 255;
            uint8_t alpha = *(bytes++);
            if (alpha <= ALPHAIGNORETHRESHOLD) continue;
            uint16_t color = red << 11 | green << 5 | blue;
            int drawX = x + destination.x;
            int drawY = y + destination.y;
            tftSprite.drawPixel(drawX, drawY, color);
        }
    }
}

const int DEADZONE = 1000;         // Joystick deadzone threshold

extern "C" int getInput(int index) {
    int vrx = -(analogRead(THUMBSTICK_X) - AXISMID);
    int vry = analogRead(THUMBSTICK_Y) - AXISMID;
    int sw1  = digitalRead(THUMBSTICK_SW) == LOW;  // Active-low button
    int sw2 = digitalRead(SW_PIN) == LOW;

    if (abs(vrx) < DEADZONE) vrx = 0;
    if (abs(vry) < DEADZONE) vry = 0;

    machineLog("%d, %d\r\n", vrx, vry);

    switch (index) {
        case 0: // UP
            return vrx;
        case 1: // RIGHT
            return vry;
        case 2: // SPACE / action button
            return sw1;
        case 3:
            return sw2;
    }
    return false;
}

void pollInputs(inputStruct_t* inputs) {
    inputs->xAxis = getInput(0);
    inputs->yAxis = getInput(1);
    inputs->action1 = getInput(2);
    inputs->action2 = getInput(3);
}

int getSeed() {
    return esp_random();
}

extern "C" void startIO(int screenWidth, int screenHeight, int fps) {
    globalfps = fps;
    frameInterval  = 1000 / globalfps;
    pinMode(THUMBSTICK_X, INPUT_PULLUP);
    pinMode(THUMBSTICK_Y, INPUT_PULLUP);
    pinMode(THUMBSTICK_SW, INPUT_PULLUP);
    pinMode(TFT_LED, OUTPUT);
    digitalWrite(TFT_LED, HIGH);
    pinMode(THUMBSTICK_VCC, OUTPUT);
    digitalWrite(THUMBSTICK_VCC, HIGH);
    pinMode(THUMBSTICK_GND, OUTPUT);
    digitalWrite(THUMBSTICK_GND, LOW);
    pinMode(SW_PIN, INPUT_PULLUP);
    tft.init();
    tft.setRotation(1);
    tftSprite.createSprite(LCDWIDTH, LCDHEIGHT);
    tft.fillScreen(TFT_BLUE);
    nextTick = millis() + frameInterval;
}

extern "C" int machineLog(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = Serial.vprintf(fmt, args);
    va_end(args);
    return n;
}

void loop() {
}

void setup() {
    delay(1000);
    Serial.begin(115200);
    machineLog("got here\n\n\n\r");
    main();
}
