//==========================================================================
#include "DlgProc.h"
#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")
//==========================================================================
#define COUNT_ADDR_W732	(DWORD)0x004A6994
#define COUNT_ADDR_XP32 (DWORD)0x004A6994

#define LEFT_BORDER		8	//找茬左图距窗口边
#define TOP_SPACE		192	//顶端距图片上部
#define MIDDLE_SPACE	9	//两幅图之间距离
#define IMAGE_WIDTH		500	//图片宽度
#define IMAGE_HEIGHT	450	//高度

#define TIMER_DEF	1
#define TIMER_NEW	2

UINT elapsedTime = 0;
int lastCount = 5;
bool fShowTitle = true;
bool fAutoPos = true;

bool fIsXP = false;

HBITMAP ghBmp = NULL;
WORD wPlanes = 0,
	wBitCount = 0;

bool fDefColor = true;
///////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		HANDLE_MSG(hDlg, WM_INITDIALOG,	OnInitDlg);
		HANDLE_MSG(hDlg, WM_PAINT,		OnPaint);
		HANDLE_MSG(hDlg, WM_COMMAND,	OnCommand);
		HANDLE_MSG(hDlg, WM_HOTKEY,		OnHotKey);
		HANDLE_MSG(hDlg, WM_KEYDOWN,	OnKeyDown);
		HANDLE_MSG(hDlg, WM_TIMER,		OnTimer);
		HANDLE_MSG(hDlg, WM_LBUTTONDOWN, OnLButtonDown);
	}
	return FALSE;
}

void SetDlgIcon(HWND hDlg, UINT uID)
{
	HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE);
	HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(uID));

	SendMessage(hDlg, WM_SETICON, TRUE,  (LPARAM)hIcon);
	SendMessage(hDlg, WM_SETICON, FALSE, (LPARAM)hIcon);
}

BOOL OnInitDlg(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	InitCommonControls();

	EnableDebugPrivilege();	//XP!

	fIsXP = !IsVistaOrLater();

	SetDlgIcon(hDlg, IDI_APP);

	SetTimer(hDlg, TIMER_DEF, 10, NULL);

	Find(hDlg);

	RegisterHotKey(hDlg, 0x1020, 0, VK_F5);
	RegisterHotKey(hDlg, 0x0104, 0, VK_F6);
	RegisterHotKey(hDlg, 0x0405, 0, VK_F7);
	RegisterHotKey(hDlg, 0x10F0, 0, VK_F8);
	RegisterHotKey(hDlg, 0x05A0, 0, VK_F9);

	return TRUE;
}

void OnPaint(HWND hDlg)
{
	if (ghBmp)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hDlg, &ps);

		HDC hdcMem = CreateCompatibleDC(hdc);
		HGDIOBJ hOldObj = SelectObject(hdcMem, ghBmp);
		BitBlt(hdc, 0, 0, 500, 450, hdcMem, 0, 0, SRCCOPY);

		SelectObject(hdcMem, hOldObj);
		DeleteDC(hdcMem);

		EndPaint(hDlg, &ps);
	}
}

void OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case IDCANCEL:
		if(ghBmp)
		{
			DeleteObject(ghBmp);
			ghBmp = NULL;
		}

		EndDialog(hDlg, id);
		break;
	}
}

void OnHotKey(HWND hDlg, int idHotKey, UINT fuModifiers, UINT vk)
{
	if (vk == VK_F5)
	{
		Find(hDlg);
	}
	else if ( vk == VK_F6)
	{
		if (IsIconic(hDlg))
			SendMessage(hDlg, WM_SYSCOMMAND, SC_RESTORE, 0);
		else
			SendMessage(hDlg, WM_SYSCOMMAND, SC_MINIMIZE, 0);
	}
	else if(vk == VK_F7)
	{
		fAutoPos = !fAutoPos;

		if(fAutoPos)
			AdjustPos(hDlg);
	}
	else if (vk == VK_F8)
	{
		DWORD dwStyle = (DWORD)GetWindowLong(hDlg, GWL_STYLE);

		if (fShowTitle)
		{
			dwStyle &= ~WS_SYSMENU;
			dwStyle &= ~WS_CAPTION;
			fShowTitle = false;
			SetWindowPos(hDlg, 0, 0, 0, IMAGE_WIDTH - 1, IMAGE_HEIGHT -1, SWP_NOZORDER | SWP_NOMOVE);
		}
		else
		{
			dwStyle |= WS_SYSMENU | WS_CAPTION;
			fShowTitle = true;
			SetWindowPos(hDlg, 0, 0, 0, IMAGE_WIDTH - 1,
				IMAGE_HEIGHT -1 + GetSystemMetrics(SM_CYSIZE), SWP_NOZORDER | SWP_NOMOVE);
		}
		SetWindowLong(hDlg, GWL_STYLE, dwStyle);
		if (fIsXP) //XP下不会自动显示标题栏，隐藏一下再显示就OK了
		{
			ShowWindow(hDlg, SW_HIDE);
			ShowWindow(hDlg, SW_SHOW);
		}
		if(fAutoPos)
			AdjustPos(hDlg);
	}
	else if( vk == VK_F9)
	{
		fDefColor = !fDefColor;
		Find(hDlg);
	}
}

void OnKeyDown(HWND hDlg, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	if (vk == VK_F1)
	{
		TCHAR szInfo[] =
			_T("快捷键：\r\n")
			_T(" －F5：刷新窗口并重新“找茬”\r\n")
			_T(" －F6：最小化、还原窗口，想看“答案”，摁摁，自己找，也摁摁！\r\n")
			_T(" －F7：固定（取消）窗口位置在第一幅图上面，默认是固定的，方便对准“茬”位置\r\n")
			_T(" －F8：隐藏、显示窗口标题栏，没有标题栏又定位在左边图片上，想想什么效果\r\n")
			_T(" －F9：换种颜色，以便查看“茬”在红色区域的\r\n")
			_T("\r\n注：如出现“乱斑”，刷新即可\r\n")
			_T("\r\n\t\t\t最后修改时间：Aug. 13, 2010")
			;

		MessageBox(hDlg, szInfo, _T("帮助"), MB_ICONINFORMATION);
	}
}

void OnTimer(HWND hDlg, UINT id)
{
	if (id == TIMER_DEF)
	{
		if(!GetTargetWnd())	return ;

		if (fAutoPos)
		{
			AdjustPos(hDlg);
		}
		int count = GetRemainCount();
		if ( count == 0 )
		{
			SetTimer(hDlg, TIMER_NEW, 970, NULL);
			lastCount = 5;
			return ;
		}
		else if (count == 5 && lastCount != count)
		{
			SetTimer(hDlg, TIMER_NEW, 970, NULL);
			lastCount = 5;
			return ;
		}
		
		lastCount = count;
	}
	else if (id == TIMER_NEW)
	{
		Find(hDlg);
		elapsedTime++;
		if (elapsedTime == 2)
		{
			Find(hDlg);
			elapsedTime = 0;
			KillTimer(hDlg, TIMER_NEW);
		}
	}
}

void OnLButtonDown(HWND hDlg, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	//实现单击鼠标时可拖动窗口
	PostMessage(hDlg, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(x, y));
}

BYTE * GetBmpBuffer(HWND hDlg, DWORD &dwSize, bool fLeft/* = true*/)
{
	HWND hWnd = GetTargetWnd();
	if (!hWnd)	return NULL;

	HDC hdcZC = GetWindowDC(hWnd);
	HDC hdcMemZC = CreateCompatibleDC(hdcZC);

	HBITMAP hBitmap = CreateCompatibleBitmap(hdcZC, IMAGE_WIDTH, IMAGE_HEIGHT);
	HGDIOBJ hOldObj = SelectObject(hdcMemZC, hBitmap);

	int rightSpace = (fLeft ? 0: IMAGE_WIDTH + MIDDLE_SPACE);

	BitBlt(hdcMemZC, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, hdcZC, LEFT_BORDER + rightSpace, TOP_SPACE, SRCCOPY);

	SelectObject(hdcMemZC, hOldObj);
	DeleteDC(hdcMemZC);

	BITMAPINFO bmi = {0};
	BITMAPINFO *pbmi;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

	GetDIBits(hdcZC, hBitmap, 0, 0, NULL, &bmi, DIB_RGB_COLORS);

	BYTE *pBits;
	ULONG sizBMI;
	dwSize = bmi.bmiHeader.biSizeImage;

	if ((pBits = (PBYTE)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, bmi.bmiHeader.biSizeImage)) == NULL)
		return NULL; 

	pbmi = &bmi;
	switch (bmi.bmiHeader.biBitCount)
	{        
	case 24:
		sizBMI = sizeof(BITMAPINFOHEADER);
		break; 
	case 16:         
	case 32:             
		sizBMI = sizeof(BITMAPINFOHEADER) + sizeof(DWORD) * 3;
		break;         
	default:             
		sizBMI = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1<<bmi.bmiHeader.biBitCount);
		break;
	}

	PBYTE pjTmp, pjTmpBmi; 
	if (sizBMI != sizeof(BITMAPINFOHEADER))
	{ 
		ULONG sizTmp;
		if ((pbmi = (PBITMAPINFO)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizBMI )) == NULL)
			return NULL;

		pjTmp = (PBYTE)pbmi;
		pjTmpBmi = (PBYTE)&bmi;
		sizTmp = sizeof(BITMAPINFOHEADER);

		while(sizTmp--)
			*(((PBYTE)pjTmp)++) = *((pjTmpBmi)++); 
	}

	wPlanes = bmi.bmiHeader.biPlanes;
	wBitCount = bmi.bmiHeader.biBitCount;

	GetDIBits(hdcZC, hBitmap, 0, pbmi->bmiHeader.biHeight, (LPSTR)pBits, pbmi, DIB_RGB_COLORS);

	ReleaseDC(hWnd, hdcZC);
	DeleteObject(hBitmap);

	return pBits;
}

void Find(HWND hDlg)
{
	HWND hWnd = GetTargetWnd();
	if (!hWnd)	return ;

	if (fIsXP)
	{
		SetForegroundWindow(hWnd);
		ShowWindow(hDlg, SW_HIDE);
		Sleep(100);
	}
	//移走鼠标
	//防止鼠标也弄在图片上
	POINT pt;
	GetCursorPos(&pt);
	SetCursorPos(pt.x, 150);	//两点一线最近……
	Sleep(30);

	DWORD dwSize;
	BYTE *leftBmpBuffer = GetBmpBuffer(hDlg, dwSize);
	BYTE *rightBmpBuffer = GetBmpBuffer(hDlg, dwSize, false);

	SetCursorPos(pt.x, pt.y);	//还原鼠标原来位置

	if (fIsXP)
		ShowWindow(hDlg, SW_SHOW);

	//for(DWORD i=0; i<dwSize; i+=1)
	//{
	//	if (leftBmpBuffer[i] != rightBmpBuffer[i])
	//	{
	//		leftBmpBuffer[i] = RGB(0, 0, 255);
	//	}
	//}

	for (DWORD i = 2; i < IMAGE_HEIGHT - 2; i++)
	{
		for (DWORD j = 2; j < IMAGE_WIDTH - 2; j+=2)
		{
			if (*((DWORD*)leftBmpBuffer + i * IMAGE_WIDTH + j) != *((DWORD*)rightBmpBuffer + i * IMAGE_WIDTH + j))
			{
				*((DWORD*)leftBmpBuffer + i * IMAGE_WIDTH + j + 0 ) = fDefColor ? RGB(0, 0, 255) : RGB(255, 0, 0);//颜色为B G R
			}
		}
	}

	for (DWORD i = 0; i < IMAGE_HEIGHT / 2; i++)
	{
		for (DWORD j = 0; j < IMAGE_WIDTH; j++)
		{
			DWORD temp = *((DWORD*)leftBmpBuffer + i * IMAGE_WIDTH + j);
			*((DWORD*)leftBmpBuffer + i * IMAGE_WIDTH + j) = *((DWORD*)leftBmpBuffer + IMAGE_WIDTH * (IMAGE_HEIGHT - 1 - i) + j);
			*((DWORD*)leftBmpBuffer + IMAGE_WIDTH * (IMAGE_HEIGHT - 1 - i) + j) = temp;
		}
	}

	if (ghBmp)
		DeleteObject(ghBmp);

	ghBmp = CreateBitmap(500, 450, wPlanes, wBitCount, leftBmpBuffer);

	InvalidateRect(hDlg, NULL, TRUE);

	GlobalFree(leftBmpBuffer);
	GlobalFree(rightBmpBuffer);
}

void AdjustPos(HWND hDlg)
{
	HWND hWnd = GetTargetWnd();
	if (!hWnd)	return ;
	RECT rect;
	GetWindowRect(hWnd, &rect);
	int x = rect.left + LEFT_BORDER;
	int y = rect.top + 192;

	x -= fShowTitle ? GetSystemMetrics(SM_CXBORDER) : 0;
	y -= (fShowTitle ? GetSystemMetrics(SM_CYSIZE) : 0);

	//防止窗口跑到显示器外
	if ( x <= 0 || x >= GetSystemMetrics(SM_CXSCREEN))
		x = 750;
	if ( y <= 0 || y >= GetSystemMetrics(SM_CYSCREEN))
		y = 50;

	SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

//bool IsGamStarted()
//{
//	HWND hWnd = GetTargetWnd();
//	if (!hWnd)	return false;
//
//	return true;
//}

int	GetRemainCount()
{
	HWND hWnd = GetTargetWnd();
	if (!hWnd)
	{
		//MessageBox(GetActiveWindow(), _T("Can't find target window!"),
		//	_T("Info"), MB_ICONINFORMATION);
		return -1;
	}

	DWORD dwPID = 0;
	GetWindowThreadProcessId(hWnd, &dwPID);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
	if (!hProcess)
	{
		//TCHAR info[100];
		//wsprintf(info, L"Failed to OpenProcess with code[%d]", GetLastError());
		//MessageBox(0, info, L"test", 0);

		return -1;
	}

	/*unsigned char*/int count = 0;
	if (!ReadProcessMemory(hProcess, (LPCVOID)(fIsXP ? COUNT_ADDR_XP32 : COUNT_ADDR_W732),
		(LPVOID)&count, sizeof(int), NULL))
	{
		//TCHAR info[100];
		//wsprintf(info, L"Failed to ReadProcessMemory with code[%d]", GetLastError());
		//MessageBox(0, info, L"test", 0);
		CloseHandle(hProcess);
		return -1;
	}

	CloseHandle(hProcess);

	return count;
}

bool EnableDebugPrivilege()
{
	HANDLE hToken;
	HANDLE hProcess = GetCurrentProcess();

	if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) )
		return false;

	TOKEN_PRIVILEGES tkp;

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid) )
		return false;

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0))
	{
		CloseHandle(hToken);
		return false;
	}
	CloseHandle(hToken);
	return true;
}

bool IsVistaOrLater()
{
	OSVERSIONINFO osvi = {0};
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (!GetVersionEx(&osvi))
		return false;

	return osvi.dwMajorVersion >= 6;
}