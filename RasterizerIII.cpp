// RasterizerIII.cpp : Defines the entry point for the application.
//
//This is for Github test
#include "stdafx.h"
#include "RasterizerIII.h"
#include "Camera.h"
#include "Geometry.h"
#include "Device.h"
#include "ObjParser.h"

#define MAX_LOADSTRING 100

//屏幕渲染参数
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int BITS = 32;				//每个像素的位数  
//const int TEXTURE_WIDTH = 200;//148;
//const int TEXTURE_HEIGHT = 200;// 149;
int tmp = 1;

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

//Windows GDI 全局变量
HDC screen_hdc;
HWND screen_hwnd;
HDC hCompatibleDC; //兼容HDC  
HBITMAP hCompatibleBitmap; //兼容BITMAP  
HBITMAP hOldBitmap; //旧的BITMAP    
HBITMAP hTextureBitmap;//纹理图
HBITMAP hNormalBitmap;//法线图
//UINT TextureBuffer[TEXTURE_WIDTH * TEXTURE_HEIGHT];
BITMAPINFO binfo; //BITMAPINFO结构体  
BITMAPINFO binfoTex;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

//begin渲染函数的声明
void Renderinit();
void RenderMain();
void RenderEnd();
void Display();
void Input(WPARAM wParam);
void isLoadnow();
//end

//主函数开始
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_RASTERIZERIII, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RASTERIZERIII));

	Renderinit();//渲染初始化

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		RenderMain();//渲染回调
		//Sleep(33);
	}
	RenderEnd();//渲染对象析构

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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RASTERIZERIII));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_RASTERIZERIII);
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

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   //yangfan Windows GDI 2018年7月12日15:54:11begin
   RECT  rectProgram, rectClient;
   GetWindowRect(hWnd, &rectProgram);   //获得程序窗口位于屏幕坐标
   GetClientRect(hWnd, &rectClient);      //获得客户区坐标
   //非客户区宽,高
   int nWidth = rectProgram.right - rectProgram.left - (rectClient.right - rectClient.left);
   int nHeiht = rectProgram.bottom - rectProgram.top - (rectClient.bottom - rectClient.top);
   nWidth += SCREEN_WIDTH;
   nHeiht += SCREEN_HEIGHT;
   rectProgram.right = nWidth;
   rectProgram.bottom = nHeiht;
   int showToScreenx = GetSystemMetrics(SM_CXSCREEN) / 2 - nWidth / 2;    //居中处理
   int showToScreeny = GetSystemMetrics(SM_CYSCREEN) / 2 - nHeiht / 2;
   MoveWindow(hWnd, showToScreenx, showToScreeny, rectProgram.right, rectProgram.bottom, false);

   memset(&binfo, 0, sizeof(BITMAPINFO));
   binfo.bmiHeader.biBitCount = BITS;      //每个像素多少位，也可直接写24(RGB)或者32(RGBA)  
   binfo.bmiHeader.biCompression = BI_RGB;
   binfo.bmiHeader.biHeight = -SCREEN_HEIGHT;
   binfo.bmiHeader.biPlanes = 1;
   binfo.bmiHeader.biSizeImage = 0;
   binfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   binfo.bmiHeader.biWidth = SCREEN_WIDTH;

   memset(&binfoTex, 0, sizeof(BITMAPINFO));
   binfoTex.bmiHeader.biBitCount = BITS;      //每个像素多少位，也可直接写24(RGB)或者32(RGBA)  
   binfoTex.bmiHeader.biCompression = BI_RGB;
   binfoTex.bmiHeader.biHeight = -TEXTURE_HEIGHT;
   binfoTex.bmiHeader.biPlanes = 1;
   binfoTex.bmiHeader.biSizeImage = 0;
   binfoTex.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   binfoTex.bmiHeader.biWidth = TEXTURE_WIDTH;

   //获取屏幕HDC  
   screen_hwnd = hWnd;
   screen_hdc = GetDC(screen_hwnd);

   //获取兼容HDC和兼容Bitmap,兼容Bitmap选入兼容HDC(每个HDC内存每时刻仅能选入一个GDI资源,GDI资源要选入HDC才能进行绘制)  
   hCompatibleDC = CreateCompatibleDC(screen_hdc);
   hCompatibleBitmap = CreateCompatibleBitmap(screen_hdc, SCREEN_WIDTH, SCREEN_HEIGHT);
   hOldBitmap = (HBITMAP)SelectObject(hCompatibleDC, hCompatibleBitmap);
   //yangfan 2018年7月12日end


   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	TCHAR msg[] = _T("Hello World!");

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
		case ID_LOAD_OBJ:
			tmp = 100;//测试变量
			isLoadnow();
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		//TextOut(hdc, 0, 500, msg, _tcslen(msg));
		// TODO: Add any Drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		Input(wParam);
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

//    Renderer渲染器

//视频缓存
unsigned int Buffer[SCREEN_WIDTH*SCREEN_HEIGHT];
//渲染装置
//Device *device;
//相机
Camera *camera;

void Renderinit(){
	//Vertex3 *vlist = (Vertex3*)malloc(sizeof(Vertex3) * 8);
	//int vCount = 8;
	//Triangle *tList = (Triangle*)malloc(sizeof(Triangle) * 12);
	//int tCount = 12;
	//ShaderVS *vs=new ShaderVS(vlist, vCount, tList, tCount);
	//vs->InitShader();
	//
	//Vertex3 *vertexList = vs->vertexList;
	//int vertexCount = vs->vertexCount;
	//Triangle *triangleList = vs->triangleList;
	//int triangleCount = vs->triangleCount;

	////立方体8个顶点
	//int vertexCount = 8;
	//Vertex3 *vertexList = (Vertex3*)malloc(sizeof(Vertex3) * vertexCount);
	//
	vertexList[0].vertex = { 1, -1, 1, 1 };
	vertexList[1].vertex = { -1, -1, 1, 1 };
	vertexList[2].vertex = { -1, 1, 1, 1 };
	vertexList[3].vertex = { 1, 1, 1, 1 };
	vertexList[4].vertex = { 1, -1, -1, 1 };
	vertexList[5].vertex = { -1, -1, -1, 1 };
	vertexList[6].vertex = { -1, 1, -1, 1 };
	vertexList[7].vertex = { 1, 1, -1, 1 };
	for (int i = 0; i < vertexCount; i++){
		vertexList[i].color = Color(0xFFFFFFFF);//底色刷成白色
		vertexList[i].normal = { -5.0, 0.0, 0.0 };
	}
	vertexList[6].normal = { 0, 0, 0 };

	////立方体 6个面，每个面2个三角形，共12个三角形
	//int triangleCount = 12;
	//Triangle *triangleList = (Triangle*)malloc(sizeof(Triangle) * triangleCount);
	//
	triangleList[0].index[0] = 0;
	triangleList[0].index[1] = 1;
	triangleList[0].index[2] = 2;
	triangleList[0].uv[0] = { 0.0, 1.0 };
	triangleList[0].uv[1] = { 0.0, 0.0 };
	triangleList[0].uv[2] = { 1.0, 0.0 };

	triangleList[1].index[0] = 2;
	triangleList[1].index[1] = 3;
	triangleList[1].index[2] = 0;
	triangleList[1].uv[0] = { 1.0, 0.0 };
	triangleList[1].uv[1] = { 1.0, 1.0 };
	triangleList[1].uv[2] = { 0.0, 1.0 };

	triangleList[2].index[0] = 6;
	triangleList[2].index[1] = 5;
	triangleList[2].index[2] = 4;
	triangleList[2].uv[0] = { 0.0, 1.0 };
	triangleList[2].uv[1] = { 0.0, 0.0 };
	triangleList[2].uv[2] = { 1.0, 0.0 };

	triangleList[3].index[0] = 4;
	triangleList[3].index[1] = 7;
	triangleList[3].index[2] = 6;
	triangleList[3].uv[0] = { 1.0, 0.0 };
	triangleList[3].uv[1] = { 1.0, 1.0 };
	triangleList[3].uv[2] = { 0.0, 1.0 };

	triangleList[4].index[0] = 7;
	triangleList[4].index[1] = 4;
	triangleList[4].index[2] = 0;
	triangleList[4].uv[0] = { 0.0, 1.0 };
	triangleList[4].uv[1] = { 0.0, 0.0 };
	triangleList[4].uv[2] = { 1.0, 0.0 };

	triangleList[5].index[0] = 0;
	triangleList[5].index[1] = 3;
	triangleList[5].index[2] = 7;
	triangleList[5].uv[0] = { 1.0, 0.0 };
	triangleList[5].uv[1] = { 1.0, 1.0 };
	triangleList[5].uv[2] = { 0.0, 1.0 };

	triangleList[6].index[0] = 1;
	triangleList[6].index[1] = 5;
	triangleList[6].index[2] = 6;
	triangleList[6].uv[0] = { 0.0, 1.0 };
	triangleList[6].uv[1] = { 0.0, 0.0 };
	triangleList[6].uv[2] = { 1.0, 0.0 };

	triangleList[7].index[0] = 6;
	triangleList[7].index[1] = 2;
	triangleList[7].index[2] = 1;
	triangleList[7].uv[0] = { 1.0, 0.0 };
	triangleList[7].uv[1] = { 1.0, 1.0 };
	triangleList[7].uv[2] = { 0.0, 1.0 };

	triangleList[8].index[0] = 5;
	triangleList[8].index[1] = 1;
	triangleList[8].index[2] = 0;
	triangleList[8].uv[0] = { 0.0, 1.0 };
	triangleList[8].uv[1] = { 0.0, 0.0 };
	triangleList[8].uv[2] = { 1.0, 0.0 };

	triangleList[9].index[0] = 0;
	triangleList[9].index[1] = 4;
	triangleList[9].index[2] = 5;
	triangleList[9].uv[0] = { 1.0, 0.0 };
	triangleList[9].uv[1] = { 1.0, 1.0 };
	triangleList[9].uv[2] = { 0.0, 1.0 };

	triangleList[10].index[0] = 3;
	triangleList[10].index[1] = 2;
	triangleList[10].index[2] = 6;
	triangleList[10].uv[0] = { 0.0, 1.0 };
	triangleList[10].uv[1] = { 0.0, 0.0 };
	triangleList[10].uv[2] = { 1.0, 0.0 };

	triangleList[11].index[0] = 6;
	triangleList[11].index[1] = 7;
	triangleList[11].index[2] = 3;
	triangleList[11].uv[0] = { 1.0, 0.0 };
	triangleList[11].uv[1] = { 1.0, 1.0 };
	triangleList[11].uv[2] = { 0.0, 1.0 };
	////

	char* name = "object0";
	Vector3 obj0Pos = { 2.0, 3.0, 5.0, 1 };//Clip测试坐标0.0, 0.0, 5.0, 1     //2.0, 3.0, 5.0, 1
	//GameObject(Vector3 pos, int id, int attribute, char* name, int vertexCount, 
	//	Vertex3* localVertexList, int triangleCount, Triangle* triangleList)
	obj0 = new GameObject(obj0Pos, 0, 0, name, vertexCount, vertexList, triangleCount, triangleList); //objectPosition, 0, 0, name, 8, vertexList, 12, triangleList
	obj0->state = RENDER_STATE_WIREFRAME;
	
	//载入纹理图
	hTextureBitmap = (HBITMAP)LoadImage(hInst, L"brick_DIFF.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	//获取纹理的像素 纹理默认TEXTURE_HEIGHT=149, TextureBuffer=148
	GetDIBits(screen_hdc, hTextureBitmap, 0, TEXTURE_HEIGHT, TextureBuffer, (BITMAPINFO*)&binfoTex, DIB_RGB_COLORS);

	//载入法线图hNormalBitmap
	hNormalBitmap = (HBITMAP)LoadImage(hInst, L"brick_NRM.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	GetDIBits(screen_hdc, hNormalBitmap, 0, TEXTURE_HEIGHT, NormalBuffer, (BITMAPINFO*)&binfoTex, DIB_RGB_COLORS);


	//材质 纹理默认TEXTURE_HEIGHT=149, TextureBuffer=148
	Material *m = new Material(TextureBuffer, TEXTURE_WIDTH, TEXTURE_HEIGHT);
	m->id = 0;
	m->color = Color(0xFFFFFFFF);
	m->Kambient = 0.5;
	m->Kdiffuse = 0.9;
	m->Kspecular = 0.1;
	m->shininess = 1.2;
	m->colorAmb = Color(0xFFFFFFFF)*m->Kambient;
	m->colorDif = Color(0xFFFFFFFF)*m->Kdiffuse;
	m->colorSpe = Color(0xFFFFFFFF)*m->Kspecular;
	m->norBuffer = NormalBuffer;
	for (int i = 0; i < 12; i++){
		triangleList[i].material = m;
	}

	Vector3 camerPos = { 0.0f, 0.0f, 0.0f, 1 };//Clip测试坐标0.0f, -5.5f, 0.0f, 1 
	Vector3 v = { 0, 1, 0.0, 0 };
	//Vector3 obj = { 2, 0, 4, 1 };
	camera = new Camera(camerPos, obj0Pos, v, 1, 10, 90, 1, SCREEN_WIDTH * 1.0f / SCREEN_HEIGHT);

	device = new Device(camera, SCREEN_WIDTH, SCREEN_HEIGHT, Buffer, RENDER_STATE_WIREFRAME, 0xFFFF0000, 0xFFFF0000);
	

	Light *ambientLight = device->addLight(LIGHTTYPE_AMBIENT, 0xFFB0B0B0);//环境光颜色
	Light *directionLight = device->addLight(LIGHTTYPE_DIRECTION, 0xFFFFFFFF);//平行光颜色（漫反射+镜面反射）
	((DirectionLight*)directionLight)->dirLight = { 5.0, 0.0, 0.0 };
	
	device->AddObjectList(obj0);

	Vector3 objPostest = { 0,0,0, 1 };//2.0, 3.0, 5.0, 1 
	GameObject* objTest = new GameObject(objPostest, 2, 1, "tes2t", vertexCount, vertexList, triangleCount, triangleList);
	//device->AddObjectList(objTest);
}

void isLoadnow(){
	loadnow = !loadnow;
}

void Display(){
	//Windows GDI 将颜色渲到屏幕上 
	SetDIBits(screen_hdc, hCompatibleBitmap, 0, SCREEN_HEIGHT, Buffer, (BITMAPINFO*)&binfo, DIB_RGB_COLORS);
	BitBlt(screen_hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hCompatibleDC, 0, 0, SRCCOPY);
}

void RenderMain(){
	device->Render();
	//device->RenderTexture(TextureBuffer,TEXTURE_WIDTH,TEXTURE_HEIGHT);//渲染一张贴图
	Display();
	Sleep(33);
}

void RenderEnd(){
	delete device;
	delete camera;
}

void Input(WPARAM wParam){
	switch (wParam){
	case VK_UP:
		if (camera != nullptr){
			Vector3 pos = camera->GetPosCamera();
			camera->SetPosCamera(pos.x, pos.y + 0.2, pos.z);
		}
		break;
	case VK_DOWN:
		if (camera != nullptr){
			Vector3 pos = camera->GetPosCamera();
			camera->SetPosCamera(pos.x, pos.y - 0.2, pos.z);
		}
		break;
	case VK_LEFT:
		if (camera != nullptr){
			Vector3 pos = camera->GetPosCamera();
			camera->SetPosCamera(pos.x - 0.2, pos.y, pos.z);
		}
		break;
	case VK_RIGHT:
		if (camera != nullptr){
			Vector3 pos = camera->GetPosCamera();
			camera->SetPosCamera(pos.x + 0.3, pos.y, pos.z);
		}
		break;
	case VK_SHIFT:
		if (camera != nullptr){
			Vector3 pos = camera->GetPosCamera();
			camera->SetPosCamera(pos.x, pos.y, pos.z + 0.3);
		}
		break;
	case VK_CONTROL:
		if (camera != nullptr){
			Vector3 pos = camera->GetPosCamera();
			camera->SetPosCamera(pos.x, pos.y, pos.z - 0.3);
		}
		break;
	case VK_SPACE:
		if (camera != nullptr){
			camera->SetPosCamera(0.0f, 0.0f, 0.0f);
		}
		break;
	}
}