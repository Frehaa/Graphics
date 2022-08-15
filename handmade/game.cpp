#include "game.h"

#include <stdint.h>

typedef void(*pixel_update_fn)(uint32_t*, int, int, int);


bool inCircle(int xDiff, int yDiff, int radius) {
    return xDiff * xDiff + yDiff * yDiff < radius * radius;
}

// 0xxxRRGGBB (low bits are blue, middle green, high red, and then padding)
uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return b | (g << 8) | (r << 16);
}

void render(int t, pixel_update_fn updatePixel, GameBitmap bitmap) {
    int pitch = bitmap.width;
    uint32_t *pixel = (uint32_t *)bitmap.memory;

    for (int y = 0; y < bitmap.height; ++y) {
        for (int x = 0; x < bitmap.width; ++x) {
            updatePixel(pixel, x, y, t);
            ++pixel;
        }
    }
}


// void renderGradientCircle(uint32_t *pixel, int x, int y, int t) {
//     int centerWidth = globalWindowWidth / 2;
//     int centerHeight = globalWindowHeight / 2;
//     int xDiff = abs(x - centerWidth);
//     int yDiff = abs(y - centerHeight);
//     if (inCircle(xDiff, yDiff, 10)) {
//         *pixel = rgb(250,0,0);
//     } else {
//         *pixel = rgb(55,55,55);
//     }
// }

// void renderSinusCurve(uint32_t *pixel, int x, int y, int t) { 
//     double v = sinf((float)(x + t/50) / 30.0) * 50.0;
//     int lineHeight = v + globalWindowHeight / 2;
//     int curveWidth = 5;
//     int dotWidth = 3;
//     if (lineHeight - curveWidth < y && y < lineHeight + curveWidth)  {
//         bool b = (t/2 % globalWindowWidth) - dotWidth < x && x < (t/2 % globalWindowWidth) + dotWidth;
//         *pixel = rgb(255, 0, 0) * b + rgb(0, 0, 0) * (1-b);
//     } else {
//         *pixel = 0xffffffff;
//     }
// }

void renderWeirdGradient(uint32_t *pixel, int x, int y, int t) {
    float p = (float) x / 255.0f;
    uint8_t r = 100 * (1-p) + 255 * p;
    uint8_t g = 50 * (1-p) + 150 * p;
    uint8_t b = 0 * (1-p) + 40 * p;
    *pixel = rgb(r, g, b);
}

float gradient(int cx, int cy, int x, int y, float maxSqDist) {
    int diffX = cx - x;
    int diffY = cy - y;

    float sqDist = diffX * diffX + diffY * diffY;
    if (sqDist > maxSqDist) {
        return 0;
    } else {
        return (1 - (sqDist / maxSqDist));
    }
}

void renderFlat(uint32_t* pixel, int x, int y, int t) {
    *pixel = rgb(0, 0, 255);
}

void renderWeirdGradient2(uint32_t *pixel, int x, int y, int t) {
    float rg = gradient(200, 200, x, y, 100000);
    float gg = gradient(500, 100, x, y, 100000);
    float bg = gradient(450, 500, x, y, 100000);

    uint8_t r =  255 * rg;
    uint8_t g = 255 * gg;
    uint8_t b = 255 * bg;
    *pixel = rgb(r, g, b);
}

void initializeGameState(GameMemory gameMemory) {

}

void updateAndRenderGame(GameMemory gameMemory, GameInput gameInput, GameBitmap bitmap, SoundBuffer gameSound) {
    GameState *gameState = (GameState *) gameMemory.persistent;

    render(0, *renderWeirdGradient, bitmap);
}
