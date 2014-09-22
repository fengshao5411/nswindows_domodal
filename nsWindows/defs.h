#ifndef __NS_DIALOGS__DEFS_H__
#define __NS_DIALOGS__DEFS_H__

#include <windows.h>

#define NSDFUNC __stdcall

typedef int nsFunction;

enum nsControlType
{
  NSCTL_UNKNOWN,
  NSCTL_BUTTON,
  NSCTL_EDIT,
  NSCTL_COMBOBOX,
  NSCTL_LISTBOX,
  NSCTL_RICHEDIT,
  NSCTL_RICHEDIT2,
  NSCTL_STATIC,
  NSCTL_LINK,
  NSCTL_TREE
};

struct nsWindowCallbacks
{
  nsFunction onBack;
  nsFunction onDropFiles;
};

#define WND_CALLBACK_IDX(x) (FIELD_OFFSET(struct nsWindowCallbacks, x)/sizeof(nsFunction))

struct nsControlCallbacks
{
  nsFunction onClick;
  nsFunction onChange;
  nsFunction onNotify;
  nsFunction onDropFiles;
};

#define CTL_CALLBACK_IDX(x) (FIELD_OFFSET(struct nsControlCallbacks, x)/sizeof(nsFunction))

#define USERDATA_SIZE 1024

struct nsControl
{
  HWND window;
  enum nsControlType type;
  char userData[USERDATA_SIZE];
  struct nsControlCallbacks callbacks;
  WNDPROC oldWndProc;
};

struct control
{
  struct nsControl* control;
};
/*
struct s_control
{
  struct control* controls;
};
*/
struct nsWindow
{
  HWND hwWindow;
  HWND hwParent;

  WNDPROC parentOriginalWndproc;

  BOOL rtl;

  struct nsWindowCallbacks callbacks;

  unsigned controlCount;

  struct nsControl* controls;
};
/*
struct s_window
{
  struct nsWindow* windows;
};

struct nsDialog
{
  HWND hwDialog;
  HWND hwParent;
};

struct s_dialog
{
  struct nsDialog* dialogs;
};
*/
#define NSCONTROL_ID_PROP "NSIS: nsControl pointer property"

#endif//__NS_DIALOGS__DEFS_H__
