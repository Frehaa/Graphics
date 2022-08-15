
#include "game.h"

#include <windows.h>
#include <math.h>
#include <dsound.h>
#include <stdint.h>

#define PI 3.141592653589793238462643383279502884197169399375108209749445923078164062
#define ARRAY_COUNT(a) (sizeof(a) / sizeof(a[0]))

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDs, LPUNKNOWN pUnkPointer)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

void println(LPCSTR m) {
    OutputDebugStringA(m);
    OutputDebugStringA("\n");
}

#define BYTES_PER_PIXEL 4
struct Bitmap {
    BITMAPINFO info;
    void *memory;
    int height;
    int width;
};

#include "win32_audio.cpp"

// ======== AUDIO DONE ======================

bool globalIsRunning = false;
Bitmap globalBitmap;
int globalWindowWidth;
int globalWindowHeight;
int globalCursorX;
int globalCursorY;

void freeMemoryIfUsed(void *memory) {
    if (memory != nullptr) {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
}

void updateBitmapSize(int width, int height) {
    // Updates bitmap globals 
    globalBitmap.height = height;
    globalBitmap.width = width;
    globalBitmap.info.bmiHeader.biWidth = width;
    globalBitmap.info.bmiHeader.biHeight = -height;
}

void resizeBitmapMemory(int width, int height) {
    freeMemoryIfUsed(globalBitmap.memory);
    updateBitmapSize(width, height);

    // Allocate memory
    int bitmapMemorySize = (width*height)*BYTES_PER_PIXEL;
    globalBitmap.memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

void updateBuffer(HDC deviceContext) {
    StretchDIBits(deviceContext,
        0, 0, globalWindowWidth, globalWindowHeight,
        0, 0, globalBitmap.width, globalBitmap.height,
        globalBitmap.memory,
        &globalBitmap.info,
        DIB_RGB_COLORS, SRCCOPY
    );    
}

void handleKeyInput(WPARAM wParam, LPARAM lParam) {
    uint32_t vkCode = wParam;
    bool wasDown = (lParam >> 30) & 0x01;
    switch (vkCode) {
        case 'W': {
            if (wasDown) {
                println("Was Down");
            } else {
                println("Was Not Down");
            }
        } break;
        case VK_ESCAPE: {
            globalIsRunning = false;
        } break;
        default:
            break;
    }
}

LRESULT CALLBACK windowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;
    switch (message) {
        case WM_SIZE: {
            RECT clientRect;
            if (GetClientRect(windowHandle, &clientRect) == false) { 
                println("GetClientRect failed");
                globalIsRunning = false; 
                break;
            }
            globalWindowWidth = clientRect.right;
            globalWindowHeight = clientRect.bottom;
            resizeBitmapMemory(globalWindowWidth, globalWindowHeight);
        } break;
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP: {
            handleKeyInput(wParam, lParam);
        } break;
        case WM_DESTROY: {
            println("WM_DESTROY");
            globalIsRunning = false;
        } break;
        case WM_CLOSE: {
            println("WM_CLOSE");
            globalIsRunning = false;
        } break;
        case WM_ACTIVATEAPP: {
            println("WM_ACTIVATE");
            SetCursor(NULL);
        } break;
        case WM_PAINT: {
            PAINTSTRUCT paint; 
            HDC deviceContext = BeginPaint(windowHandle, &paint);
            updateBuffer(deviceContext);
            EndPaint(windowHandle, &paint);
        } break;
        case WM_MOUSEMOVE: {
            uint16_t x = lParam;
            uint16_t y = lParam >> 16;

            char buffer[256];
            wsprintfA(buffer, "x: %d - y:%d", x, y);
            println(buffer);

            globalCursorX = x;
            globalCursorY = y;



        } break;
        case WM_MOUSEACTIVATE: {

        } break;
        case WM_LBUTTONDOWN: {

        }break;
        case WM_MBUTTONDOWN: {
            println("MouseDown");
            SetCapture(windowHandle);

        } break;
        case WM_MBUTTONDBLCLK: {

        } break;
        case WM_MBUTTONUP: {
            println("MouseUp");
            ReleaseCapture();

        } break;
        case WM_MOUSEWHEEL: {

        } break;

        default: {
            result = DefWindowProc(windowHandle, message, wParam, lParam);
            // println("WM_DEFAULT");
            // println((LPCSTR) &message);
        } break;
    }
    return result;
}

WNDCLASSA setupWindowClass(HINSTANCE instance) {
    WNDCLASSA windowClass = {};
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "Handmade";
    return windowClass;
}

void setupBitmapGlobals() {
    globalBitmap.info.bmiHeader.biSize = sizeof(globalBitmap.info.bmiHeader);
    globalBitmap.info.bmiHeader.biPlanes = 1; // Must be set to 1 as per doc
    globalBitmap.info.bmiHeader.biBitCount = 8 * BYTES_PER_PIXEL;
    globalBitmap.info.bmiHeader.biCompression = BI_RGB; // Uncompressed format
}

HWND setupWindowHandle(WNDCLASSA windowClass, HINSTANCE instance) {
    // DWORD style = WS_VISIBLE | (1 << 32); 
    // char buffer[256];
    // wsprintf(buffer, "test style: %d", style);
    // println(buffer);
    return CreateWindowExA(
        0,
        windowClass.lpszClassName,
        "Handmade",
        WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        instance,
        0
    );
}


int64_t getPerformanceCountFrequency() {
    LARGE_INTEGER performanceFrequency;
    QueryPerformanceFrequency(&performanceFrequency); // Never fails on XP or later
    return performanceFrequency.QuadPart;
}

void drawDot(Bitmap bitmap, Point position, int radius, uint32_t color) {
    uint32_t *pixel = (uint32_t *) bitmap.memory;
    pixel += position.x + bitmap.width * position.y;

    *pixel = color;
    *(pixel - 1) = color;
    *(pixel + 1) = color;
    *(pixel - bitmap.width) = color;
    *(pixel + bitmap.width) = color;
}

void drawLine(Bitmap bitmap, Point start, Point end, int width, uint32_t color) { 
    int pitch = bitmap.width;

    uint32_t *pixel = (uint32_t *) bitmap.memory;
    pixel += start.x + pitch * start.y;

    int dx = end.x - start.x;
    int dy = end.y - start.y;
    int yi = 1;
    int xi = 1;

    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }
    if (dx < 0) {
        xi = -1;
        dx = -dx;
    }

    int steps; // Number + 1 pixels to color. Is based on the longest axis  
    int step;  // The addition to move to the next pixel along the longest axis
    int errorStep; // Step to take in the "error" along the short axis. When the error goes over the threshold it is corrected by moving along the shorter axis
    int sideStep; // The addition to move to the next pixel along the shorter axis
    if (dx >= dy) {
        steps = dx;
        step = xi;
        errorStep = 2 * dy;
        sideStep = pitch * yi;
    } else {
        steps = dy;
        step = pitch * yi;
        errorStep = 2 * dx;
        sideStep = xi;
    }

    int D = errorStep - steps; 
    for (int _ = 0; _ <= steps; ++_) {
        *pixel = color;
        pixel += step;
        if (D > 0) {
            pixel += sideStep;
            D -= 2*steps;
        }
        D += errorStep;
    }
}

void drawLine(Bitmap bitmap, Line line, int width, uint32_t color) {
    drawLine(bitmap, line.a, line.b, width, color);
}

float_t length(Point a, Point b) {
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}

float_t length(Line line) {
    return length(line.a, line.b);
}

float_t circumference(Triangle triangle) {
    return length(triangle.a, triangle.b) + length(triangle.a, triangle.c) + length(triangle.b, triangle.c);
}

void drawTriangle(Bitmap bitmap, Triangle triangle, int width, uint32_t color) {
    drawLine(bitmap, triangle.a, triangle.b, width, color);
    drawLine(bitmap, triangle.a, triangle.c, width, color);
    drawLine(bitmap, triangle.b, triangle.c, width, color);
}

void moveTriangleToLine(Triangle &triangle, Line line) {
    int newX = 0; 
    int newY = 0;
    Point newC(newX, newY);
    triangle.c = newC;
    triangle.a = line.a;
    triangle.b = line.b;
}

void drawTriangleFracRec(Bitmap bitmap, Triangle triangle, int width, uint32_t color) {
    float_t threshold = 10;
    if (circumference(triangle) < threshold) return;
}

void drawTriangleFractal(Bitmap bitmap, Triangle triangle, int width, uint32_t color) {
    drawTriangle(bitmap, triangle, width, color);

    Triangle tab(triangle);
    Line ab(triangle.a, triangle.b);
    moveTriangleToLine(tab, ab);
    drawTriangle(bitmap, tab, width, color);
    
    Triangle tac(triangle);
    Line ac(triangle.a, triangle.c);
    moveTriangleToLine(tac, ac);
    drawTriangle(bitmap, tac, width, color);

    Triangle tbc(triangle);
    Line bc(triangle.b, triangle.c);
    moveTriangleToLine(tac, bc);
    drawTriangle(bitmap, tbc, width, color);
}

float_t radToDegree(float_t rad) {
    return (rad / PI) * 180.0f;
}

float_t degreeToRad(float_t degree) {
    return (degree / 180.0f) * PI;
}

Point rotateAroundRad(Point p, float_t rad, Point c) {
    // Transform to 0 space
    float_t x = p.x - c.x;
    float_t y = p.y - c.y;
    // Calc degrees
    float_t rc = cosf(rad);
    float_t rs = -sinf(rad);
    // Do fancy quantum rotation
    return Point(c.x + (x * rc - y * rs), c.y + (y * rc + x * rs));
}

Point rotateAroundDeg(Point p, float_t degree, Point c) {
    return rotateAroundRad(p, degreeToRad(degree), c);
}

void drawLineFractal(Bitmap bitmap, Line line, int width, uint32_t color) {
    float_t threshold = 2.0f;
    if (length(line) < threshold) return;
    drawLine(bitmap, line, 1, color);

    int dx = line.a.x - line.b.x;
    int dy = line.a.y - line.b.y;
    Point newEnd(line.b.x + (dx / 2), line.b.y + (dy / 2));
    Line newLine(line.b, rotateAroundDeg(newEnd, -90.0, line.b));
    // drawLine(bitmap, newLine, 1, color);
    drawLineFractal(bitmap, newLine, width, color);
}

void drawCubicPolynomialFromPoints(Point p0, Point p1, Point p2, Point p3, uint32_t steps) {
    // First we do Lagrange's interpolation to get the function

    // Then sample the function for "steps" points

    // Then we draw lines between each sample
}

void drawCubitPolynomialFromHermite(Point p0, Point p1, Point v0, Point v1, uint32_t steps) {
    // TODO: Do this using some fancy linear algebra multiplication
    auto h0 = [](int t) { return 2*t*t*t - 3*t*t + 1;};
    auto h1 = [](int t) { return -2*t*t*t + 3*t*t;};
    auto h2 = [](int t) { return t*t*t - 2*t*t + t;};
    auto h3 = [](int t) { return t*t*t - t*t;};
    
    // Then sample the function for "steps" points

}

void drawLineToCursor(HWND windowHandle, Bitmap bitmap) {
    int pitch = bitmap.width;
    uint32_t *pixel = (uint32_t *)bitmap.memory;

    for (int y = 0; y < bitmap.height; ++y) {
        for (int x = 0; x < bitmap.width; ++x) {
            *pixel = 0xFFFFFF;
            ++pixel;
        }
    }
    POINT point;
    if (GetCursorPos(&point) == false) {
        println("GetCursorPos error");
        return;
    }

    WINDOWPLACEMENT windowPlacement;
    windowPlacement.length = sizeof(WINDOWPLACEMENT);
    if (GetWindowPlacement(windowHandle, &windowPlacement) == false) {
        println("GetWindowPlacement error");
        return;
    }

    int cursorX = globalCursorX;
    int cursorY = globalCursorY;
    int centerX = globalWindowWidth / 2;
    int centerY = globalWindowHeight / 2;

    char buffer[256];
    wsprintfA(buffer, "x: %d - y:%d", cursorX, cursorY);
    println(buffer);

    int dx = cursorX - centerX;
    int dy = cursorY - centerY;
    int yi = 1;
    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }

    int D = 2*dy - dx;
    int y = centerY;

    pixel = (uint32_t *) bitmap.memory;
    pixel += cursorX + bitmap.width * cursorY;
    if (bitmap.memory <= pixel && pixel <= ((uint32_t*)bitmap.memory + bitmap.width + bitmap.height * bitmap.width)) {
        *((uint32_t*)bitmap.memory)  = (255 << 16);
        // *((uint32_t*)globalBitmap.memory - 1 + globalBitmap.width + globalBitmap.height * globalBitmap.width) = (255 << 16);
        *pixel = 255;
        *(pixel - 1) = 255;
        *(pixel - 2) = 255;
        *(pixel + 1) = 255;
        *(pixel + 2) = 255;
        *(pixel - bitmap.width) = 255;
        *(pixel - 2 *bitmap.width) = 255;
        *(pixel + bitmap.width) = 255;
        *(pixel + 2* bitmap.width) = 255;
    }

    pixel = (uint32_t *) bitmap.memory;
    pixel += centerX + bitmap.width * centerY;

    *pixel = 255;
    *(pixel - 1) = 255 << 16;
    *(pixel + 1) = 255 << 16;
    *(pixel - bitmap.width) = 255 << 16;
    *(pixel + bitmap.width) = 255 << 16;



    // for (int i = 0; i < dx; ++i) {
    //     pixel += 1;
    //     *pixel = 0;
    //     if (D > 0) {
    //         pixel += pitch * yi;
    //         D += 2*(dy - dx);
    //     }
    //     D += 2 * dy;
    // }

}

int WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCode){
    setupBitmapGlobals();

    float_t a = cosf(PI);
    float_t b = sinf(PI);

    float_t r0 = degreeToRad(90);
    float_t r1 = degreeToRad(180);
    float_t r2 = degreeToRad(360);
    float_t r3 = degreeToRad(-90);

    float_t c = cosf(r0);
    float_t d = sinf(r0);

    float_t e = cosf(r1);
    float_t f = sinf(r1);

    float_t g = cosf(r2);
    float_t h = sinf(r2);

    float_t i = cosf(r3);
    float_t j = sinf(r3);


    WNDCLASSA windowClass = setupWindowClass(instance);
    if (!RegisterClassA(&windowClass)) {
        // Error 
        println("Register Class failed");
        return 0;
    }

    HWND windowHandle = setupWindowHandle(windowClass, instance);
    if (!windowHandle) {
        // Error 
        println("Create Window failed");
        return 0;
    }
    long style = GetWindowLongA(windowHandle, -16L);
    // style &= -12582913L;
    SetWindowLongA(windowHandle, -16L, style);

    HDC deviceContext = GetDC(windowHandle); // TODO: Make sure this is not an issue

    int samplesPerSecond = 48000;
    int audioBufferSeconds = 1;
    AudioWrapper audioWrapper(windowHandle, samplesPerSecond, audioBufferSeconds, sineSoundSampler);

    GameMemory gameMemory;
    gameMemory.persistent = VirtualAlloc(0, 64 * 1024 * 1024, MEM_COMMIT, PAGE_READWRITE);
    gameMemory.temporal = VirtualAlloc(0, 64 * 1024 * 1024, MEM_COMMIT, PAGE_READWRITE);
    initializeGameState(gameMemory);


    // Main Loop
    globalIsRunning = true;
    int t = 0;
    int64_t performanceCountFrequency = getPerformanceCountFrequency();
    uint64_t lastCycleCount = __rdtsc();
    LARGE_INTEGER lastCounter;
    QueryPerformanceCounter(&lastCounter);
    while (globalIsRunning) {
        MSG message;
        while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE) != 0) {
            if (message.message == WM_QUIT) {
                globalIsRunning = false;
            }
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        // render(t, *renderWeirdGradient);
        GameBitmap gBitmap;
        gBitmap.memory = globalBitmap.memory;
        gBitmap.height = globalBitmap.height;
        gBitmap.width = globalBitmap.width;

        GameInput gameInput;
        SoundBuffer soundBuffer;

        // drawLineToCursor(windowHandle);

        uint32_t blue = 255;
        uint32_t red = 255 << 16;
        uint32_t white = 0xFFFFFFFF;

        Point position(globalWindowWidth / 2, globalWindowHeight / 2);
        drawDot(globalBitmap, position, 1, blue);

        int centerX = 200;
        int centerY = 200;
        int diff = 100;

        Line lines[16] =
        {
            Line(centerX, centerY, centerX + diff, centerY), // Horizontal line left to right    --->
            Line(centerX, centerY, centerX - diff, centerY), // Horizontal line right to left <----
            Line(centerX, centerY, centerX, centerY + diff), // Vertical line down V
            Line(centerX, centerY, centerX, centerY - diff), // Vertical line up ^

            Line(centerX, centerY, centerX + (diff/2), centerY - diff), 
            Line(centerX, centerY, centerX + diff, centerY - (diff /2)), 

            Line(centerX, centerY, centerX + diff, centerY + diff), // Diagonal line top left to bottom right ---->  V

            Line(centerX, centerY, centerX + (diff/2), centerY + diff), 
            Line(centerX, centerY, centerX + diff, centerY + (diff /2)), 

            Line(centerX, centerY, centerX - diff, centerY + diff), // Diagonal line top right to bottom left <----- V

            Line(centerX, centerY, centerX - (diff/2), centerY - diff), 
            Line(centerX, centerY, centerX - diff, centerY - (diff /2)), 

            Line(centerX, centerY, centerX - (diff/2), centerY + diff), 
            Line(centerX, centerY, centerX - diff, centerY + (diff /2)), 

            Line(centerX, centerY, centerX + diff, centerY - diff), // Diagonal line bottom left to top right ---->  ^
            Line(centerX, centerY, centerX - diff, centerY - diff), // Diagonal line bottom right to top left <----  ^
        };

        // drawLine(globalBitmap, lines[0], 1, red);
        // drawLine(globalBitmap, lines[1], 1, blue);

        for (int i = 0; i < ARRAY_COUNT(lines); i++) {
            drawLine(globalBitmap, lines[i], 1, red);
        }
        
        Triangle tri(Point(500, 500), Point(700, 500), Point(600, 400));
        drawTriangleFractal(globalBitmap, tri, 1, (255 << 8) + 255);

        Line l(100, 600, 400, 600);
        drawLineFractal(globalBitmap, l, 1, white);


        // updateAndRenderGame(gameMemory, gameInput, gBitmap, soundBuffer);
        updateBuffer(deviceContext);
        // audioWrapper.tick(t);

        LARGE_INTEGER endCounter;
        QueryPerformanceCounter(&endCounter);

        int64_t counterElapsed = endCounter.QuadPart - lastCounter.QuadPart;
        int32_t naPerFrame = (int32_t)((1000000*counterElapsed) / performanceCountFrequency);
        int32_t msPerFrame = naPerFrame / 1000;
        int32_t fps = 1000000 / naPerFrame;

        uint64_t endCycleCount = __rdtsc();
        uint64_t elapsedCycleCount = endCycleCount - lastCycleCount;


        POINT point;
        bool res = GetCursorPos(&point);
        WINDOWPLACEMENT windowPlacement;
        windowPlacement.length = sizeof(WINDOWPLACEMENT);
        res = GetWindowPlacement(windowHandle, &windowPlacement);
        
        
        
        RECT pos = windowPlacement.rcNormalPosition;

        char buffer[512];
        wsprintfA(buffer, "ms per frame %d, frame per second %d, elapsed mega cycles %d, point (%d, %d), window ((%d, %d), (%d, %d))", msPerFrame, fps, elapsedCycleCount / (1000 * 1000), point.x, point.y, pos.left, pos.top, pos.right, pos.bottom);
        // println(buffer);
        // SetWindowTextA(windowHandle, buffer);

        lastCounter = endCounter;
        lastCycleCount = endCycleCount;
        t+= 1;
    }

    return 0;
}
