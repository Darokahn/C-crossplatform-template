#include <stdio.h>
#include <sys/param.h>
#include <string.h>

#include "gameObjects.h"
#include "iofuncs.h"
#include "settings.h"
#include "assets.h"

int main() {
    startIO(320, 240, 3);
    inputStruct_t inputs;
    rect_t line = {0, 0, 320, 6};
    char* phrase = "Hello World! Awesome! ";
    int len = strlen(phrase);
    while (true) {
        updateIO();
        pollInputs(&inputs);

        
        int drawn = 0;
        while (drawn < len) {
            rect_t drawLine = line;
            drawn += drawString(phrase + drawn, &drawLine, basicFont, (pixel_t) {255, 255, 255}, (pixel_t) {0, 0, 0});

            line.x += drawLine.width;
            line.width -= drawLine.width;
            if (drawn < len) {
                line.x = 0;
                line.y += drawLine.height;
                line.width = 320;
            }
        }
        awaitNextTick();
    }
}
