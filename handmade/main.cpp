#include <windows.h>
#include <stdint.h>
#include <math.h>

void println(LPCSTR m) {
    OutputDebugStringA(m);
    OutputDebugStringA("\n");
}

typedef void(*pixel_update_fn)(uint32_t*, int, int, int);

struct Bitmap {
    BITMAPINFO info;
    void *memory;
    int height;
    int width;
    int bytesPerPixel;
};

bool globalIsRunning = false;
Bitmap globalBitmap;
int globalWindowWidth;
int globalWindowHeight;

int rectHeight(RECT rect) {
    return rect.bottom - rect.top;
}
int rectWidth(RECT rect) {
    return rect.right - rect.left;
}

bool inCircle(int xDiff, int yDiff, int radius) {
    return xDiff * xDiff + yDiff * yDiff < radius * radius;
}

// 0xxxRRGGBB (low bits are blue, middle green, high red, and then padding)
uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return b | (g << 8) | (r << 16);       
}

void render(int t, pixel_update_fn updatePixel) {
    int pitch = globalBitmap.width * globalBitmap.bytesPerPixel;
    uint8_t *row = (uint8_t *)globalBitmap.memory;

    for (int y = 0; y < globalBitmap.height; ++y) {
        uint32_t *pixel = (uint32_t *)row;
        for (int x = 0; x < globalBitmap.width; ++x) {
            updatePixel(pixel, x, y, t);
            ++pixel;
        }
        row += pitch;
    }
}

void renderGradientCircle(uint32_t *pixel, int x, int y, int t) {
    int centerWidth = globalWindowWidth / 2;
    int centerHeight = globalWindowHeight / 2;
    int xDiff = abs(x - centerWidth);
    int yDiff = abs(y - centerHeight);
    if (inCircle(xDiff, yDiff, 10)) {
        *pixel = rgb(250,0,0);
    } else {
        *pixel = rgb(55,55,55);
    }
}

void renderSinusCurve(uint32_t *pixel, int x, int y, int t) { 
    double v = sin((double)(x + t/50) / 30.0) * 50.0;
    int lineHeight = v + globalWindowHeight / 2;
    int curveWidth = 5;
    int dotWidth = 3;
    if (lineHeight - curveWidth < y && y < lineHeight + curveWidth)  {
        bool b = (t/2 % globalWindowWidth) - dotWidth < x && x < (t/2 % globalWindowWidth) + dotWidth;
        *pixel = rgb(255, 0, 0) * b + rgb(0, 0, 0) * (1-b);
    } else {
        *pixel = 0xffffffff;
    }
}

void renderWeirdGradient(uint32_t *pixel, int x, int y, int t) {
    float p = (float) x / (float) globalWindowWidth;
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
    float rg = gradient(200, 200, x, y, 500000);
    float gg = gradient(globalWindowWidth, 100, x, y, 500000);
    float bg = gradient(globalWindowWidth/2, 500, x, y, 500000);

    uint8_t r =  255 * rg;
    uint8_t g = 255 * gg;
    uint8_t b = 255 * bg;
    *pixel = rgb(r, g, b);
}

void freeMemoryIfUsed(void *memory) {
    if (memory != nullptr) {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
}

// Updates globals 
void updateBitmapSize(int width, int height) {
    globalBitmap.height = height;
    globalBitmap.width = width;
    globalBitmap.info.bmiHeader.biWidth = width;
    globalBitmap.info.bmiHeader.biHeight = -height;
}

void resizeBitmapMemory(int width, int height) {
    freeMemoryIfUsed(globalBitmap.memory);
    updateBitmapSize(width, height);

    // Allocate memory
    int bitmapMemorySize = (width*height)*globalBitmap.bytesPerPixel;
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
        } break;
        case WM_PAINT: {
            PAINTSTRUCT paint; 
            HDC deviceContext = BeginPaint(windowHandle, &paint);
            updateBuffer(deviceContext);
            EndPaint(windowHandle, &paint);
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

void setupGlobals() {
    globalBitmap.bytesPerPixel = 4;
    globalBitmap.info.bmiHeader.biSize = sizeof(globalBitmap.info.bmiHeader);
    globalBitmap.info.bmiHeader.biPlanes = 1;
    globalBitmap.info.bmiHeader.biBitCount = 32;
    globalBitmap.info.bmiHeader.biCompression = BI_RGB;
}

HWND setupWindowHandle(WNDCLASSA windowClass, HINSTANCE instance) {
    return CreateWindowExA(
        0,
        windowClass.lpszClassName,
        "Handmade",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
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

int WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCode){
    setupGlobals();

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

    // Main Loop
    globalIsRunning = true;
    int t = 0;
    while (globalIsRunning) {
        MSG message;
        while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE) != 0) {
            if (message.message == WM_QUIT) {
                globalIsRunning = false;
            }
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        // renderWeirdGradient(xOffset, yOffset, 60.0, 360.0);
        // render(t, *renderGradientCircle);
        // render(t, *renderWeirdGradient);
        // render(t, *renderSinusCurve);
        render(t, *renderWeirdGradient2);
        // render(t, *renderFlat);

        HDC deviceContext = GetDC(windowHandle);
        updateBuffer(deviceContext);
        ReleaseDC(windowHandle, deviceContext);

        t+= 20;
    }

    return 0;
}
