#ifndef __GAMEH

struct Point
{
    int x;
    int y;

    Point(int x, int y) : x(x), y(y){}
};

struct Line
{
    Point a;
    Point b;

    Line(int x0, int y0, int x1, int y1) : a(Point(x0, y0)), b(Point(x1, y1)) {}
    Line(Point p0, Point p1) : a(p0), b(p1) {}
};

struct Circle
{
    Point center;
    int radius;
};

struct Triangle
{
    Point a;
    Point b;
    Point c;

    Triangle(Point a, Point b, Point c): a(a), b(b), c(c) {}
};

struct Rectangle
{
    Point topLeft;
    Point bottomLeft;
};

struct GameBitmap {
    void *memory;
    int height;
    int width;
    int bytesPerPixel;
};

struct GameState {
    int x;
    int y;
};

struct GameMemory {
    void *persistent;
    void *temporal;
};

struct GameInput {

};

struct SoundBuffer {

};

void initializeGameState(GameMemory gameState);
void updateAndRenderGame(GameMemory gameState, GameInput gameInput, GameBitmap bitmap, SoundBuffer soundBuffer);

#define __GAMEH
#endif