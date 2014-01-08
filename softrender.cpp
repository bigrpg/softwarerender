// softrender.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <Vfw.h>
#include "resource.h"

#include "SREngine/Surface.h"
#include "SREngine/Device.h"
#include "SREngine/Primitive.h"
#include "SREngine/Model.h"

#define MAX_LOADSTRING 100

#pragma comment(lib,"Vfw32.lib")
#pragma comment(lib,"Winmm.lib")

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND g_hWnd = NULL;
int g_width = 800;
int g_height = 600;
sr::Surface * g_rt = NULL;
sr::Surface * g_zBuffer = NULL;
sr::Device * g_pDevice = NULL;
BITMAPINFOHEADER g_bih;
HDRAWDIB g_hDD;
imath::Matrix g_world;
sr::Object g_object;
sr::Object g_plane;
sr::Model * g_model;
bool g_rotate = true;
int g_cull = 0;
int g_texFilter = 1;
bool g_uvProjCorrect = true;
int g_zBufferMode = 3;


bool g_wireframe = false;
bool g_lightEnable = true;
bool g_directionLightEnable = true;
bool g_texEnable = true;
sr::BmpSurface * g_bmp = NULL;
imath::Vector3	g_lightDir(0,-0.5f,0.8f);

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void Draw_Text(void * pData,long dstX, long dstY, LPSTR lpszText, sr::Color textColor, long textAlpha, LPTSTR lpszFontName, long fontSize, BOOL bBold, BOOL bItalic);
BOOL Init();
void Fini();
void UpdateFrame();

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SOFTRENDER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	if(!Init())
		return FALSE;

	// Main message loop:
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// Test for quit message
			if (msg.message == WM_QUIT)
				break;

			// Translate any accelerator keys
			TranslateMessage(&msg);

			// Send message to window procedure
			DispatchMessage(&msg);
		}
		else
			UpdateFrame();

	}

	Fini();

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SOFTRENDER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;//MAKEINTRESOURCE(IDC_SOFTRENDER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	RECT rect = {0,0,g_width,g_height};
	AdjustWindowRect(&rect,style,FALSE);

   hWnd = CreateWindow(szWindowClass, szTitle, style,
	   0, 0, rect.right-rect.left, rect.bottom-rect.top, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   g_hWnd = hWnd;


   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

BOOL Init()
{
	g_model = new sr::Model;
	g_model->loadFromFile("airplane 2.x");
	//RenderTarget
	g_rt = new sr::Surface();
	g_rt->create(g_width,g_height,4);
	g_rt->clear();

	g_zBuffer = new sr::Surface();
	g_zBuffer->create(g_width,g_height,4);
	float z = 1.f;
	g_zBuffer->clear(*(unsigned int*)&z);

	g_pDevice = new sr::Device;
	g_pDevice->SetRenderTarget(g_rt);
	g_pDevice->SetZBuffer(g_zBuffer);
	g_pDevice->SetUVProjCorrect(g_uvProjCorrect);
	g_pDevice->SetAmbient(sr::Color(50,50,50,255));
	g_pDevice->SetDirectionLight(0,g_lightDir,0xffffff00);
	g_pDevice->DisableTex(!g_texEnable);

	g_bmp = new sr::BmpSurface;
	g_bmp->initFromFile("tex1.bmp");
	//sr::Color a = g_bmp->getPoint(152,161);
	//sr::Color b = g_bmp->getPoint(4,145);
	//sr::Color c = g_bmp->getPoint(2,3);

	//g_pDevice->SetCullMode(sr::CULLMODENONE);
	//g_pDevice->SetFillMode(sr::FILEMODEWIREFRAME);

	g_pDevice->SetProjTransform(3.14f/2.f,0.3f,500.f,4.f/3.f);
	g_pDevice->SetCameraTransform(imath::Vector3(0,2,-3),imath::Vector3(0,-4,8));

	const float size = 1.f;
	//front
	{
		static sr::Vertex vs[] = {
			{imath::Vector3(-size,-size,-size),	imath::Vector3(0,0,-1.f),imath::Vector2(0,1), 0xffff0000},
			{imath::Vector3(-size,size,-size),	imath::Vector3(-0.47f,0,-0.47f),imath::Vector2(0,0), 0xffffffff},
			{imath::Vector3(size,size,-size),	imath::Vector3(0,0,-1),imath::Vector2(1,0), 0xffffffff},
		};
		sr::Triangle& face = *((sr::Triangle*)vs);
		g_object.push_back(face);
	}
	{
		static sr::Vertex vs[] = {
			{imath::Vector3(size,size,-size),	imath::Vector3(0,0,-1),imath::Vector2(1,0), 0xffffffff},
			{imath::Vector3(size,-size,-size),	imath::Vector3(0,0,-1.f),imath::Vector2(1,1), 0xffffffff},
			{imath::Vector3(-size,-size,-size),	imath::Vector3(0,0,-1.f),imath::Vector2(0,1), 0xffff0000},
		};
		sr::Triangle& face = *((sr::Triangle*)vs);
		g_object.push_back(face);
	}
	//back
	{
		static sr::Vertex vs[] = {
			{imath::Vector3(size,size,size),	imath::Vector3(0,0,1.f),imath::Vector2(0,0), 0xffffffff},
			{imath::Vector3(-size,size,size),	imath::Vector3(0,0,1.f),imath::Vector2(1,0), 0xffffffff},
			{imath::Vector3(-size,-size,size),	imath::Vector3(0,0,1),imath::Vector2(1,1), 0xffffffff},
		};
		sr::Triangle& face = *((sr::Triangle*)vs);
		g_object.push_back(face);
	}
	{
		static sr::Vertex vs[] = {
			{imath::Vector3(size,size,size),	imath::Vector3(0,0,1.f),imath::Vector2(0,0), 0xffffffff},
			{imath::Vector3(-size,-size,size),	imath::Vector3(0,0,1),imath::Vector2(1,1), 0xffffffff},
			{imath::Vector3(size,-size,size),	imath::Vector3(0,0,1.f),imath::Vector2(0,1), 0xffffffff},
		};
		sr::Triangle& face = *((sr::Triangle*)vs);
		g_object.push_back(face);
	}
	//left
	{
		static sr::Vertex vs[] = {
			{imath::Vector3(-size,-size,size),	imath::Vector3(-1.f,0,0),imath::Vector2(0,1), 0xffffffff},
			{imath::Vector3(-size,size,size),	imath::Vector3(-1.f,0,0),imath::Vector2(0,0), 0xffffffff},
			{imath::Vector3(-size,size,-size),	imath::Vector3(-1.f,0,0),imath::Vector2(1,0), 0xffffffff},
		};
		sr::Triangle& face = *((sr::Triangle*)vs);
		g_object.push_back(face);
	}
	{
		static sr::Vertex vs[] = {
			{imath::Vector3(-size,-size,size),	imath::Vector3(-1.f,0,0),imath::Vector2(0,1), 0xffffffff},
			{imath::Vector3(-size,size,-size),	imath::Vector3(-1.f,0,0),imath::Vector2(1,0), 0xffffffff},
			{imath::Vector3(-size,-size,-size),	imath::Vector3(-1.f,0,0),imath::Vector2(1,1), 0xffffffff},
		};
		sr::Triangle& face = *((sr::Triangle*)vs);
		g_object.push_back(face);
	}
	//right
	{
		static sr::Vertex vs[] = {
			{imath::Vector3(size,-size,-size),	imath::Vector3(1.f,0,0),imath::Vector2(0,1), 0xffffffff},
			{imath::Vector3(size,size,-size),	imath::Vector3(1.f,0,0),imath::Vector2(0,0), 0xffffffff},
			{imath::Vector3(size,size,size),	imath::Vector3(1.f,0,0),imath::Vector2(1,0), 0xffffffff},
		};
		sr::Triangle& face = *((sr::Triangle*)vs);
		g_object.push_back(face);
	}
	{
		static sr::Vertex vs[] = {
			{imath::Vector3(size,-size,-size),	imath::Vector3(1.f,0,0),imath::Vector2(0,1), 0xffffffff},
			{imath::Vector3(size,size,size),	imath::Vector3(1.f,0,0),imath::Vector2(1,0), 0xffffffff},
			{imath::Vector3(size,-size,size),	imath::Vector3(1.f,0,0),imath::Vector2(1,1), 0xffffffff},
		};
		sr::Triangle& face = *((sr::Triangle*)vs);
		g_object.push_back(face);
	}
	//top
	{
		static sr::Vertex vs[] = {
			{imath::Vector3(size,size,-size),	imath::Vector3(0,1.f,0),imath::Vector2(0,1), 0xffffffff},
			{imath::Vector3(-size,size,-size),	imath::Vector3(0,1.f,0),imath::Vector2(0,0), 0xffffffff},
			{imath::Vector3(-size,size,size),	imath::Vector3(0,1.f,0),imath::Vector2(1,0), 0xffffffff},
		};
		sr::Triangle& face = *((sr::Triangle*)vs);
		g_object.push_back(face);
	}
	{
		static sr::Vertex vs[] = {
			{imath::Vector3(size,size,-size),	imath::Vector3(0,1.f,0),imath::Vector2(0,1), 0xffffffff},
			{imath::Vector3(-size,size,size),	imath::Vector3(0,1.f,0),imath::Vector2(1,0), 0xffffffff},
			{imath::Vector3(size,size,size),	imath::Vector3(0,1.f,0),imath::Vector2(1,1), 0xffffffff},
		};
		sr::Triangle& face = *((sr::Triangle*)vs);
		g_object.push_back(face);
	}
	//bottom
	{
		static sr::Vertex vs[] = {
			{imath::Vector3(size,-size,-size),	imath::Vector3(0,-1.f,0),imath::Vector2(0,1), 0xffffffff},
			{imath::Vector3(-size,-size,size),	imath::Vector3(0,-1.f,0),imath::Vector2(1,0), 0xffffffff},
			{imath::Vector3(-size,-size,-size),	imath::Vector3(0,-1.f,0),imath::Vector2(0,0), 0xffffffff},
		};
		sr::Triangle& face = *((sr::Triangle*)vs);
		g_object.push_back(face);
	}
	{
		static sr::Vertex vs[] = {
			{imath::Vector3(size,-size,-size),	imath::Vector3(0,-1.f,0),imath::Vector2(0,1), 0xffffffff},
			{imath::Vector3(size,-size,size),	imath::Vector3(0,-1.f,0),imath::Vector2(1,1), 0xffffffff},
			{imath::Vector3(-size,-size,size),	imath::Vector3(0,-1.f,0),imath::Vector2(1,0), 0xffffffff},
		};
		sr::Triangle& face = *((sr::Triangle*)vs);
		g_object.push_back(face);
	}



	//plane
	float planeHeight = 0.f;
	{
		float size = 12;
		static sr::Vertex vs[] = {
			{imath::Vector3(-size,planeHeight,-size),	imath::Vector3(0,1,0),imath::Vector2(0,0), 0xffffffff},
			{imath::Vector3(-size,planeHeight,size),	imath::Vector3(0,1,0),imath::Vector2(1,0), 0xffffffff},
			{imath::Vector3(size, planeHeight,-size),	imath::Vector3(0,1,0),imath::Vector2(0,1), 0xffffffff},
		};
		sr::Triangle& face = *((sr::Triangle*)vs);
		g_plane.push_back(face);
	}
	{
		float size = 12;
		static sr::Vertex vs[] = {
			{imath::Vector3(-size,planeHeight,size),	imath::Vector3(0,1,0),imath::Vector2(1,0), 0xffffffff},
			{imath::Vector3(size, planeHeight,size),	imath::Vector3(0,1,0),imath::Vector2(1,1), 0xffffffff},
			{imath::Vector3(size, planeHeight,-size),	imath::Vector3(0,1,0),imath::Vector2(0,1), 0xffffffff},
		};
		sr::Triangle& face = *((sr::Triangle*)vs);
		g_plane.push_back(face);
	}



	//ready for canvas device
	memset(&g_bih, 0, sizeof(BITMAPINFOHEADER));
	g_bih.biSize = sizeof(BITMAPINFOHEADER);
	g_bih.biWidth = g_width;
	g_bih.biHeight = g_height;
	g_bih.biPlanes = 1;
	g_bih.biBitCount = 32;
	g_bih.biCompression = BI_RGB;
	g_bih.biSizeImage = g_rt->size();
	g_hDD = DrawDibOpen();

	return !!g_hDD;
}

void Fini()
{
	DrawDibClose(g_hDD);
}

void UpdateFrame()
{
	static DWORD s_lastTime = timeGetTime();
	static DWORD frame = 0;
	static float fps = 0.f;
	++frame;
	DWORD timeBegin = timeGetTime();
	if(timeBegin - s_lastTime > 1000)
	{
		fps = frame*1000.f/(timeBegin - s_lastTime);
		frame = 0;
		s_lastTime = timeBegin;
	}

	g_pDevice->Clear(0xff0000ff,1.f);
	g_pDevice->ClearCount();

	static float angle = 0.f;
	if(g_rotate)
		angle += 0.01f;
	g_world.SetRotateY(angle);
	g_world.m31 = 1.f;
	g_pDevice->SetWorldTransform(g_world);
	g_pDevice->SetTexture(g_bmp);
	g_pDevice->renderObject(g_object);

	g_world.m30 = 3.f;
	g_world.m31 = 1.f;
	g_world.m32 = 3.f;
	g_pDevice->SetWorldTransform(g_world);
	g_model->Render(g_pDevice);

	BOOL ret;
	HDC hDC = GetDC(g_hWnd);

	static char szText[1024];
	sprintf_s(szText, sizeof(szText), "FPS:%.2f \nFace:%d",fps,g_pDevice->faceCount());
	Draw_Text(g_rt->pBuffer(),10, 10, szText, 0xffffffff, 100, L"Arial", 8, TRUE, FALSE);
	ret = DrawDibDraw(g_hDD, hDC, 0, 0, g_width, g_height, &g_bih, g_rt->pBuffer(), 0, 0, g_width, g_height, 0);

	ReleaseDC(g_hWnd, hDC);
}
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
void MoveFront(float d)
{
	imath::Vector3 dir = g_pDevice->GetCameraDir();
	//dir.y = 0.f;
	//dir.Normalize();
	imath::Vector3 pos = g_pDevice->GetCameraPosition();
	g_pDevice->SetCameraPosition(pos + dir*d);
}

void MoveRight(float d)
{
	imath::Vector3 dir = g_pDevice->GetCameraDir();
	imath::Vector3 up = g_pDevice->GetCameraUp();
	imath::Vector3 right = up.CrossProduct(dir);
	imath::Vector3 pos = g_pDevice->GetCameraPosition();
	g_pDevice->SetCameraPosition(pos + right*d);
}

void MoveUp(float d)
{
	imath::Vector3 up = g_pDevice->GetCameraUp();
	imath::Vector3 pos = g_pDevice->GetCameraPosition();
	g_pDevice->SetCameraPosition(pos + up*d);
}

void RotateRight(float d)
{
	imath::Matrix rot;
	rot.SetRotateY(d);
	imath::Vector3 dir = g_pDevice->GetCameraDir();
	dir = rot.TransformVector(dir);
	g_pDevice->SetCameraDir(dir);
}

void RotateUp(float d)
{
	imath::Vector3 dir = g_pDevice->GetCameraDir();
	float a = atanf(dir.y);
	dir.y = tanf(a+d);
	dir.Normalize();
	g_pDevice->SetCameraDir(dir);
}

void RotateLight(float d)
{
	imath::Matrix rot;
	rot.SetRotateY(d);
	g_lightDir = rot.TransformVector(g_lightDir);
	g_lightDir.Normalize();
	g_pDevice->SetDirectionLight(0,g_lightDir,0xffffff00);

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_ERASEBKGND :
		return 1L;
	case WM_KEYDOWN:
		{
			float speed = 0.5f;
			_TCHAR key = (_TCHAR)wParam;
			if(key == '1')
			{
				g_lightEnable = !g_lightEnable;
				g_pDevice->SetLightEnable(g_lightEnable);
			}
			else if(key == '2')
			{
				g_directionLightEnable = !g_directionLightEnable;
				g_pDevice->SetDirectionLightEnable(g_directionLightEnable);
			}
			else if(key == '3')
			{
				g_texEnable = !g_texEnable;
				g_pDevice->DisableTex(!g_texEnable);
			}
			else if(key == 'w' || key == 'W')
			{
				MoveFront(speed);
			}
			else if(key == 'a' || key == 'A')
			{
				MoveRight(-speed);
			}
			else if(key == 'd' || key == 'D')
			{
				MoveRight(speed);
			}
			else if(key == 's' || key == 'S')
			{
				MoveFront(-speed);
			}
			else if(key == 'e' || key == 'E')
			{
				MoveUp(speed);
			}
			else if(key == 'q' || key == 'Q')
			{
				MoveUp(-speed);
			}
			else if(key == 'b' || key == 'B')
			{
				g_cull = (g_cull+1)% sr::CULLMODEMAX;
				g_pDevice->SetCullMode( (sr::CULLMODE)g_cull);
			}
			else if(key == 'f' || key == 'F')
			{
				g_wireframe = !g_wireframe;
				g_pDevice->SetFillMode(g_wireframe ? sr::FILEMODEWIREFRAME : sr::FILEMODELSOLID);
			}
			else if(key == 't' || key == 'T')
			{
				g_texFilter = (g_texFilter+1)% sr::FILTERMODEMAX;
				g_pDevice->SetTextureFilterMode( (sr::TEXTUREFILTERMODE)g_texFilter);
			}
			else if(key == 'u' || key == 'U')
			{
				g_uvProjCorrect = !g_uvProjCorrect;
				g_pDevice->SetUVProjCorrect(g_uvProjCorrect);
			}
			else if(key == 'z' || key == 'Z')
			{
				g_zBufferMode = (g_zBufferMode+1)%sr::ZBUFFERMODEMAX;
				g_pDevice->SetZBufferMode((sr::ZBUFFERMODE)g_zBufferMode);
			}
			else if(key == VK_LEFT)
			{
				RotateRight(-3.14f/20.f);
			}
			else if(key == VK_RIGHT)
			{
				RotateRight(3.14f/20.f);
			}
			else if(key == VK_UP)
				RotateUp(3.14f/36.f);
			else if(key == VK_DOWN)
				RotateUp(-3.14f/36.f);
			else if(key == VK_HOME)
				RotateLight(-3.14f/18.f);
			else if(key == VK_END)
				RotateLight(3.14f/18.f);
			else if( key == 'i' || key == 'I')
				g_rotate = !g_rotate;
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

typedef long fixed;												// Our new fixed point type
#define itofx(x) ((x) << 8)										// Integer to fixed point
#define fxtoi(x) ((x) >> 8)										// Fixed point to integer
#define _RGB(r,g,b)	(((r) << 16) | ((g) << 8) | (b))			// Convert to RGB
#define ftofx(x) (long)((x) * 256)								// Float to fixed point
#define Divfx(x,y) (((x) << 8) / (y))							// Divide a fixed by a fixed
#define Mulfx(x,y) (((x) * (y)) >> 8)							// Multiply a fixed by a fixed


void Draw_Text(void * pData,long dstX, long dstY, LPSTR lpszText, sr::Color textColor, long textAlpha, LPTSTR lpszFontName, long fontSize, BOOL bBold, BOOL bItalic)
{
	// Check for valid bitmap
	if (lpszText != NULL)
	{
		// Draw text on the bitmap
		long iLen = (long)strlen(lpszText);
		HDC hDC = ::GetDC(NULL);
		long iFontHeight = -MulDiv(fontSize, ::GetDeviceCaps(hDC, LOGPIXELSY), 72);
		long iWeight = FW_NORMAL;
		if (bBold)
			iWeight = FW_BOLD;
		HFONT hFont = ::CreateFont(iFontHeight, 0, 0, 0, iWeight, bItalic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, lpszFontName);
		HFONT hOldFont = (HFONT)::SelectObject(hDC, hFont);
		SIZE sz;
		::GetTextExtentExPointA(hDC, lpszText, iLen, 0, NULL, NULL, &sz);
		GLYPHMETRICS gm;
		MAT2 m2 = {{0,1}, {0,0}, {0,0}, {0,1}};
		long iLetterOffset = 0;
		long iLineOffset = 0;
		for (long k=0; k<iLen; k++)
		{
			// Check for libe-break character
			if (lpszText[k] == '\n')
			{
				iLineOffset += sz.cy;
				iLetterOffset = 0;
			}
			else
			{
				// Get font letter info
				long iSize = ::GetGlyphOutlineA(hDC, lpszText[k], GGO_GRAY8_BITMAP, &gm, 0, NULL, &m2);

				// Check for valid letter
				if (lpszText[k] != ' ')
				{
					// Allocate font letter buffer
					LPBYTE lpFontBitmap = (LPBYTE)malloc(iSize*sizeof(BYTE));

					// Get font letter data
					::GetGlyphOutlineA(hDC, lpszText[k], GGO_GRAY8_BITMAP, &gm, iSize, lpFontBitmap, &m2);

					// Calculate letter params
					long iVerticalSkip = sz.cy - gm.gmBlackBoxY + (gm.gmBlackBoxY-gm.gmptGlyphOrigin.y);
					long iHorizontalSkip = gm.gmptGlyphOrigin.x;
					long pitch = gm.gmBlackBoxX;
					while ((pitch & 3) != 0)
						pitch++;

					// Draw single letter
					long iVerticalOffset = 0;
					long cy = (long)gm.gmBlackBoxY;
					long cx = (long)gm.gmBlackBoxX;
					LPDWORD lpData = (LPDWORD)pData;
					for (long i=0; i<cy; i++)
					{
						long iHorizontalOffset = 0;
						for (long j=0; j<cx; j++)
						{
							long index = iVerticalOffset + iHorizontalOffset;
							DWORD dwTotalOffset = dstX+j+iLetterOffset+iHorizontalSkip + (g_height-1-(dstY+i+iVerticalSkip+iLineOffset)) * g_width;
							sr::Color pixel = *(lpData + dwTotalOffset);
							fixed f_value = ftofx(lpFontBitmap[index]);
							fixed f_alpha = Mulfx(Divfx(f_value,ftofx(65.0f)),Divfx(itofx(textAlpha),ftofx(100.0f)));
							fixed f_1malpha = itofx(1) - f_alpha;
							fixed f_sred = itofx(textColor.r);
							fixed f_sgreen = itofx((textColor.g));
							fixed f_sblue = itofx((textColor.b));
							fixed f_dred = itofx((pixel.r));
							fixed f_dgreen = itofx((pixel.g));
							fixed f_dblue = itofx((pixel.b));
							BYTE red = (BYTE)fxtoi(Mulfx(f_alpha,f_sred) + Mulfx(f_1malpha,f_dred));
							BYTE green = (BYTE)fxtoi(Mulfx(f_alpha,f_sgreen) + Mulfx(f_1malpha,f_dgreen));
							BYTE blue = (BYTE)fxtoi(Mulfx(f_alpha,f_sblue) + Mulfx(f_1malpha,f_dblue));
//							SetPixel(dstX+j+iLetterOffset+iHorizontalSkip, dstY+i+iVerticalSkip+iLineOffset, _RGB(red, green, blue));
							*(lpData + dwTotalOffset) = _RGB(red, green, blue);

							// Increment horizontal offset
							iHorizontalOffset++;
						}

						// Increment vertical offset
						iVerticalOffset += pitch;
					}

					// Free font letter buffer
					free(lpFontBitmap);
				}

				// Increment letter horizontal offset
				iLetterOffset += gm.gmCellIncX;
			}
		}
		::SelectObject(hDC, hOldFont);
		::DeleteObject(hFont);
		::ReleaseDC(NULL, hDC);
	}
}