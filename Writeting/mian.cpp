#include <windows.h>

#define WRITEID 1
LRESULT CALLBACK WndProc(HWND hwnd,UINT message,
						 WPARAM wParam,LPARAM lParam);
void InitWindow(HDC hdcSrc); //��ʼ������

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

	hwnd = CreateWindow(szAppName,TEXT("д�ְ�"),
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
	static int iElapse; //��¼������ʱ��
	static BOOL bWrite; //��־�Ƿ�����д��
	static BOOL bDown; //��־��갴��
	static int iWords; //���㱣������ֵĸ���
	static HDC hMemDC; //�ڴ�DC
	static HBITMAP hMemBitmap; //�ڴ�λͼ
	static HDC hdc; //����DC
	static POINT PointStart; //�ߵ����
	POINT PointEnd; //�ߵ��յ�
	RECT rt;
	int iRow,jRow; //��ʾ���������(10�У�400/40,15�У�600/40)
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
			MessageBox(hwnd,TEXT("�����ڴ�DC�����ڴ�λͼʧ��"),TEXT(""),MB_OK);
			SendMessage(hwnd,WM_DESTROY,0,0);
		}
		SelectObject(hMemDC,hMemBitmap);
		InitWindow(hMemDC);
		ReleaseDC(hwnd,hdc);
		SetTimer(hwnd,WRITEID,1000,NULL);
		return 0;

	case WM_TIMER:
		if(bWrite && !bDown) //��������д��״̬�����̧��
		{
			if(iElapse++ > 4) //����д�ֽ���
			{
				if(iWords >= 150) //�������������������
				{
					rt.left = 1;
					rt.right = 599;
					rt.top = 31;
					rt.bottom = 429;
					FillRect(hMemDC,&rt,WHITE_BRUSH);
					iWords = 0;
				}
				iRow = iWords/15; //��
				jRow = iWords%15; //��
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
		if(VK_BACK == wParam) //��backspace����ɾ����������һ����
		{
			--iWords;
			iRow = iWords/15; //��
			jRow = iWords%15; //��
			rt.left = 40*jRow+2;
			rt.right = rt.left+38;
			rt.top = 40*iRow+32;
			rt.bottom = rt.top+38;
			FillRect(hMemDC,&rt,WHITE_BRUSH);
			InvalidateRect(hwnd,&rt,TRUE);
		}
		return 0;

	case WM_MOUSEMOVE:
		if(bWrite && bDown) //����д��״̬������������
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
			bWrite = TRUE; //����д��״̬
			bDown = TRUE; //������갴��
			SetCapture(hwnd);
		}
		return 0;

	case WM_LBUTTONUP:
		bDown = FALSE; //�������̧��
		iElapse = 0; //���̧��ʱ��ʼ��ʱ
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
		AnimateWindow(hwnd,500,AW_CENTER); //��ʾ��ʼ������
		InvalidateRect(hwnd,NULL,TRUE);
		return 0;
	}
	return DefWindowProc(hwnd,message,wParam,lParam);
}

void InitWindow(HDC hdcSrc) //��ʼ������
{
	RECT rt;
	
	//�����ڲ���
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
	DrawTextA(hdcSrc,"�����",-1,&rt,DT_CENTER|DT_VCENTER|DT_SINGLELINE);

	rt.left = 600;
	rt.right = 1000;
	rt.top = 0;
	rt.bottom = 30;
	DrawTextA(hdcSrc,"д����",-1,&rt,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
}