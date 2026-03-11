#include <stdio.h>
#include <sys/param.h>
#include <string.h>

#include "gameObjects.h"
#include "iofuncs.h"
#include "settings.h"
#include "assets.h"

#define CHARWIDTH 4
#define CHARHEIGHT 6
#define BUTTONPRESSLENGTH 5
#define BUTTONRAPIDLENGTH 1
#define BUTTONRAPIDINTERVAL 3
#define SCREENWIDTH 320
#define SCREENHEIGHT 240

struct letterSet terminalFont;

bool repeatButton(int held) {
    if (held == 1) return true; 
    if (held >= BUTTONPRESSLENGTH * BUTTONRAPIDINTERVAL) return (held % BUTTONRAPIDLENGTH) == 0;
    return false;
}

int drawStringTerminal(char* string, int lineLengths[], int* thisLine, int charWidth, int charHeight, int screenWidth, int screenHeight, pixel_t primaryColor, pixel_t backgroundColor, bool realDraw) {
    int printed = 0;
    int thisLineSave = *thisLine;
    int len = strlen(string);
    while (true) {
        int x = lineLengths[*thisLine] * charWidth;
        int y = *thisLine * charHeight;
        rect_t rect = {x, y, screenWidth - x, charHeight};
        int iterPrinted = drawString(string + printed, &rect, terminalFont, primaryColor, backgroundColor, charWidth, charHeight);
        printed += iterPrinted;
        if (realDraw) {
            lineLengths[*thisLine] += iterPrinted;
        }
        if (printed == len) break;
        ++*thisLine;
    }
    if (!realDraw) *thisLine = thisLineSave;
}

int main() {
    startIO(320, 240, 30);
    terminalFont = basicFont;
    inputStruct_t inputs;
    int leftHeld = 0;
    int rightHeld = 0;
    int action1Held = 0;
    int action2Held = 0;
    rect_t line = {0, 0, 320, CHARHEIGHT};
    int lineLengths[240 / CHARHEIGHT] = {0};
    int thisLine = 0;
    pixel_t green = {0, 255, 0};
    pixel_t black = {0, 0, 0};
    pixel_t white = {255, 255, 255};

    unsigned char thisCharacter = 'a';

    while (true) {
        updateIO();
        pollInputs(&inputs);
        char newString[2] = {0};

        if (inputs.xAxis < 0) leftHeld++;
        else leftHeld = 0;
        if (inputs.xAxis > 0) rightHeld++;
        else rightHeld = 0;
        if (inputs.action1) action1Held++;
        else action1Held = 0;
        if (inputs.action2) action2Held++;
        else action2Held = 0;
        if (repeatButton(rightHeld)) {
            thisCharacter = MIN(thisCharacter + 1, '~');
        }
        if (repeatButton(leftHeld)) {
            thisCharacter = MAX(thisCharacter - 1, ' ' - 2);
        }
        unsigned char tempCharacter = thisCharacter;
        if (tempCharacter == ' ' - 1) tempCharacter = '~';
        if (tempCharacter == ' ' - 2) tempCharacter = '>';
        newString[0] = tempCharacter;

        int fakeLine = thisLine;
        
        drawStringTerminal(newString, lineLengths, &fakeLine, CHARWIDTH, CHARHEIGHT, SCREENWIDTH, SCREENHEIGHT, green, black, false);

        if (repeatButton(action2Held)) {
            if (thisCharacter == ' ' - 1) {
                drawStringTerminal(" ", lineLengths, &thisLine, CHARWIDTH, CHARHEIGHT, SCREENWIDTH, SCREENHEIGHT, black, black, false);
                thisLine++;
            }
            else if (thisCharacter == ' ' - 2) {
                drawStringTerminal("    ", lineLengths, &thisLine, CHARWIDTH, CHARHEIGHT, SCREENWIDTH, SCREENHEIGHT, black, black, true);
            }
            else {
                drawStringTerminal(newString, lineLengths, &thisLine, CHARWIDTH, CHARHEIGHT, SCREENWIDTH, SCREENHEIGHT, white, black, true);
            }
        }
        if (repeatButton(action1Held)) {
            drawStringTerminal(" ", lineLengths, &thisLine, CHARWIDTH, CHARHEIGHT, SCREENWIDTH, SCREENHEIGHT, black, black, false);
            if (lineLengths[thisLine] == 0 && thisLine > 0) {
                thisLine--;
            }
            else if (lineLengths[thisLine] >= 1) {
                drawStringTerminal(" ", lineLengths, &thisLine, CHARWIDTH, CHARHEIGHT, SCREENWIDTH, SCREENHEIGHT, black, black, false);
                lineLengths[thisLine]--;
            }
        }
        awaitNextTick();
    }
}
