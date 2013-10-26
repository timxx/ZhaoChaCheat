//==========================================================================
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\
	"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
//==========================================================================
#include "DlgProc.h"
//==========================================================================

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE, LPTSTR, int)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc, NULL);
	return 0;
}
//==========================================================================