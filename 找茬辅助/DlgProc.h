//==========================================================================
#include <Windows.h>
#include <WindowsX.h>
#include <tchar.h>

#include "resource.h"
//==========================================================================
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);	//对话框回调函数

void SetDlgIcon(HWND hDlg, UINT uID);	//设置对话框图标
BOOL OnInitDlg(HWND hDlg, HWND hwndFocus, LPARAM lParam);//初始化对话框, WM_INITDIALOG
void OnPaint(HWND hDlg);	//WM_PAINT
void OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);	//WM_COMMAND
void OnHotKey(HWND hDlg, int idHotKey, UINT fuModifiers, UINT vk);	//WM_HOTKEY
void OnKeyDown(HWND hDlg, UINT vk, BOOL fDown, int cRepeat, UINT flags);		//WM_KEYDOWN
void OnTimer(HWND hDlg, UINT id);	//WM_TIMER
void OnLButtonDown(HWND hDlg, BOOL fDoubleClick, int x, int y, UINT keyFlags);	//WM_LBUTTONDOWN

inline HWND GetTargetWnd(){		return FindWindow(NULL, _T("大家来找茬"));	}	//找茬窗口句柄

BYTE * GetBmpBuffer(HWND hDlg, DWORD &dwSize, bool fLeft = true);	//取得左或右两图的内容并返回
void Find(HWND hDlg);			//开始比对
void AdjustPos(HWND hDlg);		//调整窗口位置在第一幅图上面
//bool IsGamStarted();
int	GetRemainCount();			//获取找茬剩余数，用于自动刷新

bool EnableDebugPrivilege();	//提升SeDebug权限
bool IsVistaOrLater();			//判断系统是Vista后还是前
////////////////////////////////////////////////////////////////////////////