#include <time.h>

#include "settings.h"
#include "gameObjects.h"
#include "iofuncs.h"

#include "emscripten.h"

double nextTick;

EM_JS(double, getMillis, (), {
    return performance.now();
});

EM_JS(void, _startIO, (int screenWidth, int screenHeight, int fps), {
    clearNextFrame = false;
    canvas = document.querySelector("#canvas");
    canvas.width = screenWidth;
    canvas.height = screenHeight;
    ctx = canvas.getContext("2d", { alpha: true });
    imageCache = {};
});

EM_JS(void, _updateIO, (), {
    clearNextFrame = true;
});

EM_JS(void, _drawObject, (int x, int y, int width, int height, int xResolution, int yResolution, int id, void* pixels), {
    if (clearNextFrame) ctx.clearRect(0, 0, canvas.width, canvas.height);
    clearNextFrame = false;
    if (imageCache[id] != undefined) {
        ctx.drawImage(imageCache[id], x, y, width, height);
        return;
    }
    let size = width * height * 4;
    let pixelArray = new Uint8ClampedArray(Module.HEAPU8.buffer, pixels, size);
    let imageData = new ImageData(pixelArray, width, height);
    createImageBitmap(imageData).then(image => {
        imageCache[id] = image;
        ctx.drawImage(image, x, y, width, height);
    });
});

EM_JS(void, _awaitNextTick, (), {
});

int getSeed() {
    return time(NULL);
}

void pollInputs(inputStruct_t* input) {
    return;
}

void startIO(int screenWidth, int screenHeight, int fps) {
    nextTick = getMillis() + (1000.0f / fps);
    _startIO(screenWidth, screenHeight, fps);
}

void updateIO() {
    _updateIO();
}

void drawObject(interface drawable d) {
    gameObject_t* object = d.object;
    spriteSetTrait* spriteSet = d.sprites;
    spriteSheet_t* spriteSheet = &spriteSet->spriteSheets[spriteSet->currentSheet];
    image_t* image = getFrame(d);
    _drawObject(object->x, object->y, spriteSheet->width * spriteSheet->xScale, spriteSheet->height * spriteSheet->yScale, image->width, image->height, image->id, image->pixels);
}

void awaitNextTick() {
    _awaitNextTick();
    emscripten_sleep(nextTick - getMillis());
    nextTick += 1000.0f / FPS;
}
