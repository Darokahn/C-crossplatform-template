#include <stdio.h>
#include <sys/param.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "gameObjects.h"
#include "iofuncs.h"
#include "settings.h"
#include "assets.h"
#include "libraries/commonInterfaces/interfaces.h"

#define CHARWIDTH 4
#define CHARHEIGHT 6
#define BUTTONPRESSLENGTH 5
#define BUTTONRAPIDLENGTH 1
#define BUTTONRAPIDINTERVAL 3
#define SCREENWIDTH 320
#define SCREENHEIGHT 240

struct letterSet terminalFont;

typedef struct {
    uint16_t axisDeadzone;
    union {
        uint16_t inputs[6];
        struct {
            uint16_t A;
            uint16_t B;
            uint16_t xAxisRepeat;
            uint16_t yAxisRepeat;
            uint16_t xAxis;
            uint16_t yAxis;
        };
    };
} controllerState_t;

void doNothing() {return;}

int controllerState_write(controllerState_t* state, uint8_t* input, size_t size) {
    struct event* e = (void*) input;
    while (size >= sizeof *e) {
        if (e->type == schemaButton) {
            int index = e->button.index;
            state->inputs[index] = e->button.value;
        }
        else if (e->type == schemaAxis) {
            int index = e->axis.index;
            int midpointDist = e->axis.value - axisMidpoint;
            int repeatValue = 1;
            if (abs(midpointDist) <= state->axisDeadzone) repeatValue = 0;
            state->inputs[2 + (index - 4)] = repeatValue;
            state->inputs[index] = e->axis.value;
        }
        e += 1;
        size -= sizeof *e;
    }
    return size;
}

void controllerState_inc(controllerState_t* state) {
    for (int i = 0; i < 4; i++) {
        uint16_t* input = state->inputs + i;
        if (!*input) continue;
        if (*input == UINT16_MAX) {
            *input = 1;
        }
        ++*input;
    }
}

write_vt controllerState_write_vt = {
    .write=erase controllerState_write,
    .printf=erase doNothing,
};

controllerState_t* controllerState_init(controllerState_t* state, uint16_t deadzone) {
    *state = (controllerState_t) {0};
    state->axisDeadzone = deadzone;
    struct schemaEntry schema[] = {
        {.type=schemaButton, .index=0, .hints="A"},
        {.type=schemaButton, .index=1, .hints="B"},
        {.type=schemaAxis, .index=4, .hints="joystick1X"},
        {.type=schemaAxis, .index=5, .hints="joystick1Y"},
    };
    registerInput(schema, 4, (write_i) {.base=state, .write=&controllerState_write_vt}, NULL);
    return state;
}

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
    controllerState_t inputs;
    controllerState_init(&inputs, 200);
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
        controllerState_inc(&inputs);
        updateIO();
        printf("inputs: %d, %d, %d, %d, %d, %d\n", inputs.A, inputs.B, inputs.xAxisRepeat, inputs.yAxisRepeat, inputs.xAxis, inputs.yAxis);
        char newString[2] = {0};

        if (repeatButton(inputs.xAxisRepeat)) {
            int direction = 1;
            if (inputs.xAxis < axisMidpoint) direction = -1;
            thisCharacter += direction;
            if (thisCharacter < ' ' - 2) thisCharacter = ' ' - 2;
            else if (thisCharacter > '~') thisCharacter = '~';
        }
        unsigned char tempCharacter = thisCharacter;
        if (tempCharacter == ' ' - 1) tempCharacter = '~';
        if (tempCharacter == ' ' - 2) tempCharacter = '>';
        newString[0] = tempCharacter;

        int fakeLine = thisLine;
        
        drawStringTerminal(newString, lineLengths, &fakeLine, CHARWIDTH, CHARHEIGHT, SCREENWIDTH, SCREENHEIGHT, green, black, false);

        if (repeatButton(inputs.B)) {
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
        if (repeatButton(inputs.A)) {
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
