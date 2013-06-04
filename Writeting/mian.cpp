#include <windows.h>

#define WRITEID 1
LRESULT CALLBACK WndProc(HWND hwnd,UINT message,
						 WPARAM wParam,LPARAM lParam);
void InitWindow(HDC hdcSrc); //初始化窗口

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrev,
				   PSTR szCmdLine,int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("write");
	HWND hwnd;
	MSG msg;
	WNDCLASS wndclass;

	wndclass.style = CS_HREDRAW|CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if(!RegisterClass(&wndclass))
	{
		MessageBox(NULL,TEXT("register fialed"),TEXT(""),MB_OK);
		return 0;
	}

	hwnd = CreateWindow(szAppName,TEXT("写字板"),
		WS_POPUP|WS_CAPTION|WS_SYSMENU| WS_MINIMIZEBOX,
		150,100,1006,458,
		NULL,NULL,hInstance,NULL);

	ShowWindow(hwnd,iCmdShow);
	UpdateWindow(hwnd);

	while(GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd,UINT message,
						 WPARAM wParam,LPARAM lParam)
{
	static int iElapse; //记录经过的时间
	static BOOL bWrite; //标志是否正在写字
	static BOOL bDown; //标志鼠标按下
	static int iWords; //计算保存输出字的个数
	static HDC hMemDC; //内存DC
	static HBITMAP hMemBitmap; //内存位图
	RECT rt;
	HDC hdc;
	PAINTSTRUCT ps;

	switch(message)
	{
	case WM_CREATE:
		iElapse = 0;
		bWrite = FALSE;
		bDown = FALSE;
		iWords = 0;
		
		hdc = GetDC(hwnd);
		GetClientRect(hwnd,&rt);
		hMemDC = CreateCompatibleDC(hdc);
		hMemBitmap = CreateCompatibleBitmap(hdc,rt.right-rt.left,rt.bottom-rt.top);
		if(NULL == hMemDC || NULL == hMemBitmap)
		{
			MessageBox(hwnd,TEXT("创建内存DC或者内存位图失败"),TEXT(""),MB_OK);
			SendMessage(hwnd,WM_DESTROY,0,0);
		}
		SelectObject(hMemDC,hMemBitmap);
		InitWindow(hMemDC);
		ReleaseDC(hwnd,hdc);
		SetTimer(hwnd,WRITEID,1000,NULL);
		return 0;

	case WM_TIMER:
		return 0;

	case WM_MOUSEMOVE:
		return 0;

	case WM_LBUTTONDOWN:
		return 0;

	case WM_LBUTTONUP:
		return 0;

	case WM_PAINT:
		hdc = BeginPaint (hwnd,&ps);
		BitBlt(hdc,0,0,1000,430,hMemDC,0,0,SRCCOPY);
		EndPaint (hwnd,&ps);
		return 0;

	case WM_DESTROY:
		if(NULL != hMemBitmap)
		{
			DeleteObject(hMemBitmap);
			hMemBitmap = NULL;
		}
		if(NULL != hMemDC)
		{
			DeleteObject(hMemDC);
			hMemDC = NULL;
		}
		KillTimer(hwnd,WRITEID);
		PostQuitMessage(0);
		return 0;

	case WM_SHOWWINDOW:
		AnimateWindow(hwnd,500,AW_CENTER);
		InvalidateRect(hwnd,NULL,TRUE);
		return 0;
	}
	return DefWindowProc(hwnd,message,wParam,lParam);
}

void InitWindow(HDC hdcSrc) //初始化窗口
{
	RECT rt;
	
	//画窗口布局
	rt.left = 0;
	rt.right = 1000;
	rt.top = 0;
	rt.bottom = 430;
	FillRect(hdcSrc,&rt,WHITE_BRUSH);
	Rectangle(hdcSrc,0,0,600,430);
	Rectangle(hdcSrc,600,0,1000,430);
	Rectangle(hdcSrc,0,0,1000,30);
	
	rt.left = 0;
	rt.right = 600;
	rt.top = 0;
	rt.bottom = 30;
	DrawTextA(hdcSrc,"输出区",-1,&rt,DT_CENTER|DT_VCENTER|DT_SINGLELINE);

	rt.left = 600;
	rt.right = 1000;
	rt.top = 0;
	rt.bottom = 30;
	DrawTextA(hdcSrc,"写字区",-1,&rt,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
}