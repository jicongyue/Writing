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
	static HDC hdc; //窗口DC
	static POINT PointStart; //线的起点
	POINT PointEnd; //线的终点
	RECT rt;
	int iRow,jRow; //表示输出的行列(10行：400/40,15列：600/40)
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
		if(bWrite && !bDown) //表明处于写的状态且鼠标抬起
		{
			if(iElapse++ > 4) //表明写字结束
			{
				if(iWords >= 150) //输出区已满，清除输出区
				{
					rt.left = 1;
					rt.right = 599;
					rt.top = 31;
					rt.bottom = 429;
					FillRect(hMemDC,&rt,WHITE_BRUSH);
					iWords = 0;
				}
				iRow = iWords/15; //行
				jRow = iWords%15; //列
				hdc = GetDC(hwnd);
				StretchBlt(hMemDC,40*jRow+2,40*iRow+32,38,38,
					hdc,602,32,398,398,SRCCOPY);
				InvalidateRect(hwnd,NULL,TRUE);
				ReleaseDC(hwnd,hdc);
				bWrite = FALSE;
				bDown = FALSE;
				++iWords;
			}
		}
		return 0;

	case WM_KEYDOWN:
		if(VK_BACK == wParam) //按backspace键，删除输出区最后一个字
		{
			--iWords;
			iRow = iWords/15; //行
			jRow = iWords%15; //列
			rt.left = 40*jRow+2;
			rt.right = rt.left+38;
			rt.top = 40*iRow+32;
			rt.bottom = rt.top+38;
			FillRect(hMemDC,&rt,WHITE_BRUSH);
			InvalidateRect(hwnd,&rt,TRUE);
		}
		return 0;

	case WM_MOUSEMOVE:
		if(bWrite && bDown) //处于写字状态且鼠标左键按下
		{
			PointEnd.x = LOWORD(lParam);
			PointEnd.y = HIWORD(lParam);
			if(PointEnd.x > 600 && PointEnd.x < 1000
				&& PointEnd.y > 30 && PointEnd.y < 430)
			{
				hdc = GetDC(hwnd);
				MoveToEx(hdc,PointStart.x,PointStart.y,NULL);
				LineTo(hdc,PointEnd.x,PointEnd.y);
				PointStart.x = PointEnd.x;
				PointStart.y = PointEnd.y;
				ReleaseDC(hwnd,hdc);
			}
		}
		return 0;

	case WM_LBUTTONDOWN:
		PointStart.x = LOWORD(lParam);
		PointStart.y = HIWORD(lParam);
		if(PointStart.x > 600 && PointStart.x < 1000 
			&& PointStart.y > 30 && PointStart.y < 430)
		{
			bWrite = TRUE; //设置写的状态
			bDown = TRUE; //表明鼠标按下
			SetCapture(hwnd);
		}
		return 0;

	case WM_LBUTTONUP:
		bDown = FALSE; //表明鼠标抬起
		iElapse = 0; //鼠标抬起时开始计时
		ReleaseCapture();
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
		AnimateWindow(hwnd,500,AW_CENTER); //显示初始化动画
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
	Rectangle(hdcSrc,0,0,1000,430);
	MoveToEx(hdcSrc,0,30,NULL);
	LineTo(hdcSrc,1000,30);
	MoveToEx(hdcSrc,600,0,NULL);
	LineTo(hdcSrc,600,430);
	
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