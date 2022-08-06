#include <windows.h>
#include <GL/GL.h>

void println(LPCSTR m) {
    OutputDebugStringA(m);
    OutputDebugStringA("\n");
}

#pragma comment (lib, "opengl32.lib")
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
    MSG msg = {0}; 
    WNDCLASSW wc = {0}; 
    wc.lpfnWndProc = WndProc;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = L"oglversionchecksample";
    wc.style = CS_OWNDC;
    if ( !RegisterClassW(&wc) ) return 1;

    CreateWindowExW(0L, wc.lpszClassName, L"openglversioncheck", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 640, 480, 0, 0, hInstance, 0);

    while ( GetMessageW(&msg, NULL, 0, 0) > 0 ) DispatchMessageA( &msg );

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_CREATE:
		{
		PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
			PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
			32,                   // Colordepth of the framebuffer.
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,                   // Number of bits for the depthbuffer
			8,                    // Number of bits for the stencilbuffer
			0,                    // Number of Aux buffers in the framebuffer.
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};

		HDC deviceContext = GetDC(hWnd);

		int  pixelFormat = ChoosePixelFormat(deviceContext, &pfd); 
		SetPixelFormat(deviceContext, pixelFormat, &pfd);

		HGLRC renderingContext = wglCreateContext(deviceContext);
		wglMakeCurrent (deviceContext, renderingContext);

		MessageBoxA(0,(char*)glGetString(GL_VERSION), "OPENGL VERSION",0);

		//wglMakeCurrent(ourWindowHandleToDeviceContext, NULL); Unnecessary; wglDeleteContext will make the context not current
		wglDeleteContext(renderingContext);
		PostQuitMessage(0);
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;

}