#include <windows.h>

#include <nsis/pluginapi.h> // nsis plugin

#include "defs.h"
#include "input.h"
#include "rtl.h"
//#include "..\..\build\release\config\nsis-sconf.h"
//#include <nsis-sconf.h>
#define NSWINDOWS_MAX_STRLEN 1024

#ifndef ODS_NOACCEL
#define ODS_NOACCEL 0x0100
#define ODS_NOFOCUSRECT 0x0200
#endif
#ifndef DT_HIDEPREFIX
#define DT_HIDEPREFIX 0x00100000
#endif

//#define _WIN32_WINNT 0x0500
#ifndef GWL_EXSTYLE
	#define GWL_EXSTYLE		-20
#endif
#ifndef WS_EX_LAYERED
	#define WS_EX_LAYERED	0x00080000
#endif
#ifndef LWA_ALPHA
	#define LWA_ALPHA		0x00000002
#endif

HINSTANCE g_hInstance;
struct nsWindow g_window;
//struct s_window sWindow;

//struct nsDialog g_dialog;
//struct s_dialog sDialog;

//struct s_control sCtl;
struct nsControl* ctl;

extra_parameters* g_pluginParms;

struct nsControl* NSDFUNC GetControl(HWND hwCtl)
//struct s_control NSDFUNC GetControl(HWND hwCtl)
{
  unsigned id = (unsigned) GetProp(hwCtl, NSCONTROL_ID_PROP);

  if (id == 0 || id > g_window.controlCount)
  {
    return NULL;
  }

  return &g_window.controls[id - 1];
}

BOOL CALLBACK WindowProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
    case WM_CLOSE:
//  MessageBox(g_window.hwWindow,"WM_CLOSE",NULL,MB_OK);
      if (g_pluginParms->ExecuteCodeSegment(g_window.callbacks.onBack - 1, 0))
      {
        return FALSE;
      }
      else
    {
      //DestroyWindow(g_window.hwWindow);
      DestroyWindow(g_window.hwWindow);
        return FALSE;
    }

    // handle notifications
    case WM_COMMAND:
    {
      HWND hwCtl = GetDlgItem(g_window.hwWindow, LOWORD(wParam));
//      struct s_control sCtl;
//	  ctl = (struct nsControl*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct nsControl));
	  ctl = GetControl(hwCtl);

      if (ctl == NULL)
        break;

      if (HIWORD(wParam) == BN_CLICKED && (ctl->type == NSCTL_BUTTON || ctl->type == NSCTL_LINK))
      {
        if (ctl->callbacks.onClick)
        {
          pushint((int) hwCtl);
          g_pluginParms->ExecuteCodeSegment(ctl->callbacks.onClick - 1, 0);
        }
      }
      else if (HIWORD(wParam) == EN_CHANGE && ctl->type == NSCTL_EDIT)
      {
        if (ctl->callbacks.onChange)
        {
        pushint((int) hwCtl);
        g_pluginParms->ExecuteCodeSegment(ctl->callbacks.onChange - 1, 0);
      }
      }
      else if (HIWORD(wParam) == LBN_SELCHANGE && ctl->type == NSCTL_LISTBOX)
      {
        if (ctl->callbacks.onChange)
        {
        pushint((int) hwCtl);
        g_pluginParms->ExecuteCodeSegment(ctl->callbacks.onChange - 1, 0);
      }
      }
      else if ((HIWORD(wParam) == CBN_EDITUPDATE || HIWORD(wParam) == CBN_SELCHANGE)
                && ctl->type == NSCTL_COMBOBOX)
      {
        if (ctl->callbacks.onChange)
        {
        pushint((int) hwCtl);
        g_pluginParms->ExecuteCodeSegment(ctl->callbacks.onChange - 1, 0);
      }
      }
      else if (HIWORD(wParam) == STN_CLICKED && ctl->type == NSCTL_STATIC)
      {
        if (ctl->callbacks.onClick)
        {
        pushint((int) hwCtl);
        g_pluginParms->ExecuteCodeSegment(ctl->callbacks.onClick - 1, 0);
				}
      }

		  //ctl = NULL;
      break;
    }

    case WM_NOTIFY:
    {
      LPNMHDR nmhdr = (LPNMHDR) lParam;
      //struct nsControl* ctl = GetControl(nmhdr->hwndFrom);
//      struct s_control sCtl;
//	  sCtl.controls = (struct control*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct control));
	  ctl = GetControl(nmhdr->hwndFrom);

      if (ctl == NULL)
        break;

      if (!ctl->callbacks.onNotify)
        break;

      pushint((int) nmhdr);
      pushint(nmhdr->code);
      pushint((int) nmhdr->hwndFrom);
      g_pluginParms->ExecuteCodeSegment(ctl->callbacks.onNotify - 1, 0);
    }

    case WM_DROPFILES:
    {
  // get info from stack

      //HWND hwCtl = (HWND) popint();
      //nsFunction callback = (nsFunction) popint();
      //HWND hwCtl = (HWND) hwnd;
      CHAR szBuf[NSWINDOWS_MAX_STRLEN];
      //LPSTR szStr[256];
      HDROP hDrop = (HDROP)wParam;
			int iCount, index; 

      //if (!IsWindow(hwCtl))
			//	return FALSE;

			if (IsIconic(hwnd))
				ShowWindow(hwnd,SW_RESTORE);
			SetForegroundWindow(hwnd);

			//MessageBox(g_window.hwWindow,"DropFilesWndProc",NULL,MB_OK);
			//DragQueryFile(hDrop,0,szBuf,sizeof(szBuf));
			index = 0xFFFFFFFF;
			iCount = DragQueryFile(hDrop,index,szBuf,sizeof(szBuf));
			for (index = iCount - 1; index >=0; index--)
			{
				DragQueryFile(hDrop,index,szBuf,sizeof(szBuf));
				pushstring(szBuf);
			}
				//pushint((int) hwCtl);
				pushint((int) iCount);

				//MessageBox(g_window.hwWindow,szBuf,NULL,MB_OK);

			g_pluginParms->ExecuteCodeSegment(g_window.callbacks.onDropFiles - 1, 0);
    return FALSE;
  }

	// handle links
    case WM_DRAWITEM:
    {
      DRAWITEMSTRUCT* lpdis = (DRAWITEMSTRUCT*)lParam;
      RECT rc;
      char text[1024];

      // http://blogs.msdn.com/oldnewthing/archive/2005/05/03/414317.aspx#414357
      // says we should call SystemParametersInfo(SPI_GETKEYBOARDCUES,..) to make
      // sure, does not seem to be required, might be a win2k bug, or it might
      // only apply to menus
      BOOL hideFocus = (lpdis->itemState & ODS_NOFOCUSRECT);
      BOOL hideAccel = (lpdis->itemState & ODS_NOACCEL);

      //struct nsControl* ctl = GetControl(lpdis->hwndItem);
//      struct s_control sCtl;
//	  sCtl.controls = (struct control*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct control));
//	  ctl = (struct nsControl*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct nsControl));
	  ctl = GetControl(lpdis->hwndItem);

      if (ctl == NULL)
        break;

      // We need lpdis->rcItem later
      rc = lpdis->rcItem;

      // Get button's text
      text[0] = '\0';
      GetWindowText(lpdis->hwndItem, text, 1024);

      // Calculate needed size of the control
      DrawText(lpdis->hDC, text, -1, &rc, DT_VCENTER | DT_WORDBREAK | DT_CALCRECT);

      // Make some more room so the focus rect won't cut letters off
      rc.right = min(rc.right + 2, lpdis->rcItem.right);

      // Move rect to right if in RTL mode
      if (g_window.rtl)
      {
        rc.left += lpdis->rcItem.right - rc.right;
        rc.right += lpdis->rcItem.right - rc.right;
      }

      if (lpdis->itemAction & ODA_DRAWENTIRE)
      {
        DWORD xtraDrawStyle = (g_window.rtl ? DT_RTLREADING : 0);
        if (hideAccel)
          xtraDrawStyle |= DT_HIDEPREFIX;

        // Use blue unless the user has set another using SetCtlColors
        if (!GetWindowLong(lpdis->hwndItem, GWL_USERDATA))
          SetTextColor(lpdis->hDC, RGB(0,0,255));

        // Draw the text
        DrawText(lpdis->hDC, text, -1, &rc, xtraDrawStyle | DT_CENTER | DT_VCENTER | DT_WORDBREAK);
      }

      // Draw the focus rect if needed
      if (((lpdis->itemState & ODS_FOCUS) && (lpdis->itemAction & ODA_DRAWENTIRE)) || (lpdis->itemAction & ODA_FOCUS))
      {
        // NB: when not in DRAWENTIRE mode, this will actually toggle the focus
        // rectangle since it's drawn in a XOR way
        if (!hideFocus)
          DrawFocusRect(lpdis->hDC, &rc);
      }

      return TRUE;
    }

    // handle colors
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORLISTBOX:
      // let the NSIS window handle colors, it knows best
      return SendMessage(g_window.hwParent, Msg, wParam, lParam);

    // bye bye
    case WM_DESTROY:
    {
      unsigned i;
      for (i = 0; i < g_window.controlCount; i++)
      {
        RemoveProp(g_window.controls[i].window, NSCONTROL_ID_PROP);
      }
    break;
    }
  } 
  return FALSE;
}

//link 控件额外回调
LRESULT CALLBACK LinkWndProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
//  struct nsControl* ctl = GetControl(hwnd);
//  ctl = (struct nsControl*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct nsControl));
  ctl = GetControl(hwnd);

  if(ctl == NULL)
    return 0;

  if(Msg == WM_SETCURSOR)
  {
    SetCursor(LoadCursor(NULL, IDC_HAND));
    return TRUE;
  }

  return CallWindowProc(ctl->oldWndProc, hwnd, Msg, wParam, lParam);

//  ctl = NULL;
}

//onDropFiles 事件回调
LRESULT CALLBACK DropFilesWndProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
//  struct nsControl* ctl = GetControl(hwnd);
//  ctl = (struct nsControl*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct nsControl));
	ctl = GetControl(hwnd);

	if(ctl == NULL)
		return 0;

	if(Msg == WM_DROPFILES)
	if (ctl->callbacks.onDropFiles)
  {
		//CHAR szBuf[256];
		CHAR szBuf[NSWINDOWS_MAX_STRLEN];
		HDROP hDrop = (HDROP)wParam;
		int iCount, index; 

		index = 0xFFFFFFFF;

		//MessageBox(g_window.hwWindow,"DropFilesWndProc",NULL,MB_OK);
		//DragQueryFile(hDrop,0,szBuf,sizeof(szBuf));
		iCount = DragQueryFile(hDrop,index,szBuf,sizeof(szBuf));
			for (index = iCount - 1; index >=0; index--)
// 		for (index = 0; index < iCount; index++)  
			{
				DragQueryFile(hDrop,index,szBuf,sizeof(szBuf));
				pushstring(szBuf);
			}

     // Reset Change Notify
      //bPendingChangeNotify = FALSE;
     
		//MessageBox(g_window.hwWindow,szBuf,NULL,MB_OK);

			//MessageBox(g_window.hwWindow,"ctl->callbacks.onDropFiles",NULL,MB_OK);
			//pushint((int) hwCtl);
			pushint((int) iCount);
			//pushstring(szPath);
			//pushstring(szBuf);
			//MessageBox(g_window.hwWindow,"(ctl->callbacks.onDropFiles)",NULL,MB_OK);

			if (g_pluginParms->ExecuteCodeSegment(ctl->callbacks.onDropFiles - 1, 0))
				return TRUE;
	}

  return CallWindowProc(ctl->oldWndProc, hwnd, Msg, wParam, lParam);

//  ctl = NULL;
}


static UINT_PTR PluginCallback(enum NSPIM Msg)
{
  return 0;
}

void __declspec(dllexport) Create(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
//  MSG Msg;
  WNDCLASS WndClass;
  HWND dhwPlacementRect;
  //HWND whwPlacementRect;
  HWND hwParent; //add by zhfi
//  HWND hwWindow; //add by zhfi
	//对话框尺寸
  RECT drcPlacement;
	//父窗口窗口位置
  RECT wrcPlacement;
	//WINDOWPLACEMENT *lpwndpl;
		
  int X, Y, nWidth, nHeight;

  DWORD dwExStyle, dwStyle;
  //LPCTSTR lpClassName, lpWindowName;
  char *lpClassName;
  char *lpWindowName;


//  s_plugins = (char *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, g_stringsize * 2);
  lpClassName = (char *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, g_stringsize * 2);
  lpWindowName = (char *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, NSWINDOWS_MAX_STRLEN * 2);

  EXDLL_INIT();

//  MessageBox(hwndParent,"EXDLL_INIT",NULL,MB_OK);
  extra->RegisterPluginCallback(g_hInstance, PluginCallback);

//  MessageBox(hwndParent,"RegisterPluginCallback",NULL,MB_OK);
	//add by zhfi
	hwParent = (HWND) (popint());
//  MessageBox(hwndParent,"hwParent = (HWND) (popint())",NULL,MB_OK);
	if (IsWindow(hwParent))
    g_window.hwParent = hwParent;
  else
    g_window.hwParent = hwndParent;

	//获取窗口位置
	if (GetWindowRect(g_window.hwParent, &wrcPlacement))
		{
			X = wrcPlacement.left;
			Y = wrcPlacement.top;
			nWidth = wrcPlacement.right - wrcPlacement.left;
			nHeight = wrcPlacement.bottom - wrcPlacement.top;
		}
	else
		{
			X = 300;
			Y = 200;
			nWidth = 300;
			nHeight = 200;
		}
		
  g_pluginParms = NULL;
//  MessageBox(hwndParent,"g_pluginParms",NULL,MB_OK);

//  popstringn(lpWindowName, 0);
  dwExStyle = (DWORD) popint_or();
//  MessageBox(hwndParent,lpWindowName,NULL,MB_OK);
//  popstringn(lpWindowName, 0);
  dwStyle = (DWORD) popint_or();
//  MessageBox(hwndParent,lpWindowName,NULL,MB_OK);
  lpClassName = "#32770";
  popstringn(lpWindowName, NSWINDOWS_MAX_STRLEN);
//  MessageBox(hwndParent,lpWindowName,NULL,MB_OK);
  {
//    MEMSET (&WndClass,0,sizeof(WNDCLASS));
//    HeapAlloc(&WndClass, GMEM_MOVEABLE, sizeof(WNDCLASS));
    WndClass.style						= dwStyle;
    WndClass.lpfnWndProc			= WindowProc;
    WndClass.cbClsExtra       = 0;
    WndClass.cbWndExtra       = 0;
    WndClass.hInstance        = g_hInstance;
    WndClass.hIcon            = LoadIcon(NULL, "END");
    WndClass.hCursor					= LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground    = (HBRUSH)(GetStockObject(WHITE_BRUSH));
    WndClass.lpszMenuName			= NULL;
    WndClass.lpszClassName    = lpClassName;
//  MessageBox(hwndParent,"WndClass.lpszClassName",NULL,MB_OK);
    RegisterClass(&WndClass);
  }
//  MessageBox(hwndParent,"RegisterClass",NULL,MB_OK);
  
  g_window.hwWindow=CreateWindowEx(dwExStyle,
                                  WndClass.lpszClassName,
                                  lpWindowName,
                                  dwStyle,
                                  X,
                                  Y,
                                  nWidth,
                                  nHeight,
                                  hwParent,
                                  NULL,
                                  g_hInstance,
                                  NULL);

  if (g_window.hwWindow == NULL)
  {
    pushstring("error");
    return;
  }

//  MessageBox(hwndParent,"CreateWindowEx",NULL,MB_OK);
  
  g_pluginParms = extra;

  dhwPlacementRect = GetDlgItem(hwndParent, popint());
	if (IsWindow(dhwPlacementRect))
  if (GetWindowRect(dhwPlacementRect, &drcPlacement))
	{
		MapWindowPoints(NULL, hwndParent, (LPPOINT) &drcPlacement, 2);

		nWidth = drcPlacement.right - drcPlacement.left;
		nHeight = drcPlacement.bottom - drcPlacement.top;
	}

	X = X + (wrcPlacement.right - wrcPlacement.left - nWidth) / 2;
	Y = Y + (wrcPlacement.bottom - wrcPlacement.top - nHeight) / 2;

  SetWindowPos(
    g_window.hwWindow,
    0,
    X,
    Y,
    nWidth,
    nHeight,
    SWP_NOZORDER | SWP_NOACTIVATE| SWP_HIDEWINDOW
  );

  g_window.rtl = FALSE;

  g_window.controlCount = 0;
  g_window.controls = (struct nsControl*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 0);

//  g_window.callbacks.onBack = 0;

  ShowWindow(g_window.hwWindow, SW_HIDE);

  pushint((int) g_window.hwWindow);
}

void __declspec(dllexport) CreateControl(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  char *className;
  char *text;

  HWND hwItem;
  int x, y, width, height;
  DWORD style, exStyle;
  size_t id;

  // get info from stack

  className = (char *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, g_stringsize * 2);
  text = (char *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, NSWINDOWS_MAX_STRLEN * 2);
//  text = &className[g_stringsize];

  if (!className)
  {
    pushstring("error");
    return;
  }

  if (popstringn(className, 0))
  {
    pushstring("error");
    HeapFree(GetProcessHeap(), 0, className);
    return;
  }

  style = (DWORD) popint_or();
  exStyle = (DWORD) popint_or();

  PopPlacement(&x, &y, &width, &height);

  if (popstringn(text, 0))
  {
    pushstring("error");
    HeapFree(GetProcessHeap(), 0, className);
    return;
  }

  // create item descriptor

  id = g_window.controlCount;
  g_window.controlCount++;
  g_window.controls = (struct nsControl*) HeapReAlloc(
    GetProcessHeap(),
    HEAP_ZERO_MEMORY,
    g_window.controls,
    g_window.controlCount * sizeof(struct nsControl));

  if (!lstrcmpi(className, "BUTTON"))
    g_window.controls[id].type = NSCTL_BUTTON;
  else if (!lstrcmpi(className, "EDIT"))
    g_window.controls[id].type = NSCTL_EDIT;
  else if (!lstrcmpi(className, "COMBOBOX"))
    g_window.controls[id].type = NSCTL_COMBOBOX;
  else if (!lstrcmpi(className, "LISTBOX"))
    g_window.controls[id].type = NSCTL_LISTBOX;
  else if (!lstrcmpi(className, "RichEdit"))
    g_window.controls[id].type = NSCTL_RICHEDIT;
  else if (!lstrcmpi(className, "RICHEDIT_CLASS"))
    g_window.controls[id].type = NSCTL_RICHEDIT2;
  else if (!lstrcmpi(className, "STATIC"))
    g_window.controls[id].type = NSCTL_STATIC;
  else if (!lstrcmpi(className, "LINK"))
    g_window.controls[id].type = NSCTL_LINK;
  else
    g_window.controls[id].type = NSCTL_UNKNOWN;

  // apply rtl to style

  ConvertStyleToRTL(g_window.controls[id].type, &style, &exStyle);

  // create item's window

  hwItem = CreateWindowEx(
    exStyle,
    lstrcmpi(className, "LINK") ? className : "BUTTON",
    text,
    style,
    x, y, width, height,
    g_window.hwWindow,
    (HMENU) (1200 +id),
    g_hInstance,
    NULL);

  g_window.controls[id].window = hwItem;

  // remember id

  SetProp(hwItem, NSCONTROL_ID_PROP, (HANDLE) (id + 1));

  // set font

  SendMessage(hwItem, WM_SETFONT, SendMessage(hwndParent, WM_GETFONT, 0, 0), TRUE);

  // set the WndProc for the link control

  if(g_window.controls[id].type == NSCTL_LINK)
    g_window.controls[id].oldWndProc = (WNDPROC) SetWindowLong(hwItem, GWL_WNDPROC, (long) LinkWndProc);

  // push back result

  pushint((int) hwItem);

  // done

  HeapFree(GetProcessHeap(), 0, className);
}

// for backward compatibility (2.29 had CreateItem)
void __declspec(dllexport) CreateItem(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  CreateControl(g_window.hwParent, string_size, variables, stacktop, extra);
}

void __declspec(dllexport) SetUserData(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  HWND hwCtl;
//  struct nsControl* ctl;
//  struct s_control sCtl;
//  sCtl.controls = (struct control*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct control));
      
  // get info from stack

  hwCtl = (HWND) popint();

  if (!IsWindow(hwCtl))
  {
    popint(); // remove user data from stack
    return;
  }

  // get descriptor

//  ctl = GetControl(hwCtl);
//  ctl = (struct nsControl*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct nsControl));
  ctl = GetControl(hwCtl);

  if (ctl == NULL)
    return;

  // set user data

  popstringn(ctl->userData, USERDATA_SIZE);
//  ctl = NULL;
}

void __declspec(dllexport) GetUserData(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  HWND hwCtl;
  //struct nsControl* ctl;
//  struct s_control sCtl;
//  sCtl.controls = (struct control*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct control));

  // get info from stack

  hwCtl = (HWND) popint();

  if (!IsWindow(hwCtl))
  {
    pushstring("");
    return;
  }

  // get descriptor

//  ctl = GetControl(hwCtl);
//  ctl = (struct nsControl*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct nsControl));
  ctl = GetControl(hwCtl);

  if (ctl == NULL)
  {
    pushstring("");
    return;
  }

  // return user data

  pushstring(ctl->userData);

//  ctl = NULL;
}

void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
  // we use a timer proc instead of WM_TIMER to make sure no one messes with the ids but us
  g_pluginParms->ExecuteCodeSegment(idEvent - 1, 0);
}

void __declspec(dllexport) CreateTimer(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  UINT callback;
  UINT interval;

  // get info from stack

  callback = popint();
  interval = popint();

  if (!callback || !interval)
    return;

  // create timer

  SetTimer(
    g_window.hwWindow,
    callback,
    interval,
    TimerProc);
}

void nsdKillTimer(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  UINT id;

  // get timer id from stack

  id = popint();

  // kill timer

  KillTimer(g_window.hwWindow, id);
}

void NSDFUNC SetControlCallback(size_t callbackIdx)
{
  HWND hwCtl;
  nsFunction  callback;
  nsFunction* callbacks;
//  struct nsControl* ctl;
//  struct s_control sCtl;
//  sCtl.controls = (struct control*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct control));

  // get info from stack

  hwCtl = (HWND) popint();
  callback = (nsFunction) popint();

  if (!IsWindow(hwCtl))
    return;
  //MessageBox(g_window.hwWindow,"(IsWindow(hwCtl)",NULL,MB_OK);

  // get descriptor

//  ctl = GetControl(hwCtl);
//  ctl = (struct nsControl*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct nsControl));
  ctl = GetControl(hwCtl);

  if (ctl == NULL)
    return;

  // set callback

  callbacks = (nsFunction*) &ctl->callbacks;
  callbacks[callbackIdx] = callback;

//  ctl = NULL;
}

void NSDFUNC SetWindowCallback(size_t callbackIdx)
{
  nsFunction  callback;
  nsFunction* callbacks;

  // get info from stack

  callback = (nsFunction) popint();

  // set callback

  callbacks = (nsFunction*) &g_window.callbacks;
  callbacks[callbackIdx] = callback;
}

void __declspec(dllexport) OnClick(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  SetControlCallback(CTL_CALLBACK_IDX(onClick));
}

void __declspec(dllexport) OnChange(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  SetControlCallback(CTL_CALLBACK_IDX(onChange));
}

void __declspec(dllexport) OnNotify(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  SetControlCallback(CTL_CALLBACK_IDX(onNotify));
}

void __declspec(dllexport) onDropFiles(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{  
  HWND hwCtl;
  //LONG lwnd;
  nsFunction  callback;

  // get info from stack

  hwCtl = (HWND) popint();
  callback = (nsFunction) popint();

  if (!IsWindow(hwCtl))
    return;

  //lwnd = GetWindowLong(hwCtl, GWL_EXSTYLE);
  //SetWindowLong(hwCtl, GWL_EXSTYLE, lwnd | WS_EX_ACCEPTFILES);
	DragAcceptFiles(hwCtl, TRUE);

  pushint((nsFunction) callback);
//  pushint((int) hwCtl);

	if (hwCtl == g_window.hwWindow)
		SetWindowCallback(WND_CALLBACK_IDX(onDropFiles));
	else
		{
			//size_t id;
			//id = g_window.controlCount - 1;
			//MessageBox(g_window.hwWindow,"control dropfiles",NULL,MB_OK);
			//g_window.controls[id].oldWndProc = (WNDPROC) SetWindowLong(hwCtl, GWL_WNDPROC, (long) DropFilesWndProc);
			ctl = GetControl(hwCtl);
			if (ctl == NULL)
				return;
			ctl->oldWndProc = (WNDPROC) SetWindowLong(hwCtl, GWL_WNDPROC, (long) DropFilesWndProc);

			pushint((int) hwCtl);

			SetControlCallback(CTL_CALLBACK_IDX(onDropFiles));
		}
}

void __declspec(dllexport) OnBack(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{  
  SetWindowCallback(WND_CALLBACK_IDX(onBack));
}

void __declspec(dllexport) SetTransparent(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{  
WINUSERAPI
BOOL WINAPI SetLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
	HWND hwnd;
	BYTE bAlpha;
	LONG lwnd;

  EXDLL_INIT();

//  MessageBox(hwndParent,"EXDLL_INIT",NULL,MB_OK);
  extra->RegisterPluginCallback(g_hInstance, PluginCallback);

	hwnd = (HWND) (popint());
	bAlpha = popint() * 255 / 100;
	if (IsWindow(hwnd))
	{
		lwnd = GetWindowLong(hwnd, GWL_EXSTYLE);
/*	  if (lwnd < WS_EX_LAYERED)
	  {
			lwnd = lwnd + WS_EX_LAYERED;
			SetWindowLong(hwnd, GWL_EXSTYLE, lwnd);
	  }*/
		SetWindowLong(hwnd, GWL_EXSTYLE, lwnd | WS_EX_LAYERED);
		SetLayeredWindowAttributes(hwnd, 0, bAlpha, LWA_ALPHA);
	}
}

void __declspec(dllexport) Show(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
//unsigned i;
  // tell NSIS to remove old inner dialog and pass handle of the new inner dialog
  //SendMessage(hwndParent, WM_NOTIFY_CUSTOM_READY, (WPARAM) g_window.hwWindow, 0);
//add by zhfi
//  SendMessage(g_window.hwWindow, WM_NOTIFY_CUSTOM_READY, (WPARAM) g_window.hwWindow, 0);
//  SendMessage(g_window.hwWindow, WA_ACTIVE, 0, 0);
  ShowWindow(g_window.hwWindow, SW_HIDE);

  // message loop

//  while (g_window.hwWindow)
//  for (i = 0; i <= i_windows; i++)
//  i = i_windows;
  if (g_window.hwWindow)
  {
    MSG Msg;
    GetMessage(&Msg, NULL, 0, 0);
    if (!IsDialogMessage(g_window.hwWindow, &Msg))
    {
      TranslateMessage(&Msg);
      DispatchMessage(&Msg);
    }
  }
  // reset wndproc

//  SetWindowLong(hwndParent, DWL_DLGPROC, (long) g_window.parentOriginalWndproc);
//add by zhfi
  SetWindowLong(g_window.hwWindow, DWL_DLGPROC, (long) WindowProc);
//  ShowWindow(g_window.hwWindow, SW_SHOW);
//  SetWindowLong(g_window.hwWindow, DWL_DLGPROC, (long) g_window.parentOriginalWndproc);
  ShowWindow(g_window.hwWindow, SW_SHOW);
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance = (HINSTANCE) hInst;
  return TRUE;
}
