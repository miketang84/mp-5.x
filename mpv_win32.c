/*

    Minimum Profit - Programmer Text Editor

    Win32 driver.

    Copyright (C) 1991-2005 Angel Ortega <angel@triptico.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    http://www.triptico.com

*/

#include "config.h"

#ifdef CONFOPT_WIN32

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <commctrl.h>

#include "mpdm.h"
#include "mpsl.h"

#include "mp.h"

/*******************
	Data
********************/

/* the driver */
mpdm_t win32_driver = NULL;
mpdm_t win32_window = NULL;

/* the instance */
HINSTANCE hinst;

/* the windows */
HWND hwnd;
HWND hwtabs;

/* font handlers and metrics */
HFONT font_normal = NULL;
int font_width = 0;
int font_height = 0;

/*******************
	Code
********************/

static void update_window_size(void)
/* updates the viewport size in characters */
{
	RECT rect;
	int tx, ty;

	/* no font information? go */
	if(font_width == 0 || font_height == 0)
		return;

	GetClientRect(hwnd, &rect);

	/* calculate the size in chars */
	tx = ((rect.right - rect.left) / font_width) + 1;
	ty = ((rect.bottom - rect.top) / font_height) + 1;

	/* store the 'window' size */
	mpdm_hset_s(win32_window, L"tx", MPDM_I(tx));
	mpdm_hset_s(win32_window, L"ty", MPDM_I(ty));
}


int font_size = 14;
char * font_face = "Lucida Console";

static void build_fonts(HDC hdc)
/* build the fonts */
{
	TEXTMETRIC tm;
	int n;

	/* create fonts */
	n = -MulDiv(font_size, GetDeviceCaps(hdc, LOGPIXELSY), 72);

	font_normal = CreateFont(n, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, font_face);

	SelectObject(hdc, font_normal);
	GetTextMetrics(hdc, &tm);

	/* store sizes */
	font_height = tm.tmHeight;
	font_width = tm.tmAveCharWidth;

	update_window_size();
}


mpdm_t mpi_draw(mpdm_t v);

static void win32_draw(HWND hwnd, mpdm_t doc)
{
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect;
	RECT r2;
	mpdm_t d = NULL;
	int n, m;

	/* start painting */
	hdc = BeginPaint(hwnd, &ps);

	/* no document? end */
	if((d = mpi_draw(doc)) == NULL)
	{
		EndPaint(hwnd, &ps);
		return;
	}

	/* no font? construct it */
	if(font_normal == NULL)
		build_fonts(hdc);

	/* select defaults to start painting */
	SelectObject(hdc, font_normal);
	SetTextColor(hdc, 0x00000000);
	SetBkColor(hdc, 0x00ffffff);
/*
	r2.top += _tab_height;
	r2.bottom = r2.top + _mpv_font_height;
*/

	GetClientRect(hwnd, &rect);
	r2 = rect;
	r2.bottom = r2.top + font_height;

	for(n = 0;n < mpdm_size(d);n++)
	{
		mpdm_t l = mpdm_aget(d, n);

		r2.left = r2.right = rect.left;

		for(m = 0;m < mpdm_size(l);m++)
		{
			int attr;
			mpdm_t s;

			/* get the attribute and the string */
			attr = mpdm_ival(mpdm_aget(l, m++));
			s = mpdm_aget(l, m);

/*			SetTextColor(hdc,_inks[attr]);
			SetBkColor(hdc,_papers[attr]);

			SelectObject(hdc, color==MP_COLOR_COMMENT ?
				_font_italic :
				color==MP_COLOR_LOCAL ? _font_underline :
				_font_normal);
*/
			r2.right += mpdm_size(s) * font_width;

			DrawTextW(hdc, (wchar_t *)s->data,
				-1, &r2, DT_SINGLELINE|DT_NOPREFIX);
/*			TextOutW(hdc, r2.left, r2.top, s->data, mpdm_size(s));*/

			r2.left = r2.right;
		}

		r2.top += font_height;
		r2.bottom += font_height;
	}

	EndPaint(hwnd, &ps);
}


static void win32_vkey(int c)
/* win32 virtual key processing */
{
	wchar_t * ptr = NULL;
	static int maxed = 0;

/*	mpi_move_selecting=(GetKeyState(VK_SHIFT) & 0x8000);
*/
	if(GetKeyState(VK_CONTROL) & 0x8000 ||
	   GetKeyState(VK_MENU) & 0x8000)
	{
		switch(c)
		{
		case VK_UP:	ptr = L"ctrl-cursor-up"; break;
		case VK_DOWN:	ptr = L"ctrl-cursor-down"; break;
		case VK_LEFT:	ptr = L"ctrl-cursor-left"; break;
		case VK_RIGHT:	ptr = L"ctrl-cursor-right"; break;
		case VK_PRIOR:	ptr = L"ctrl-page-up"; break;
		case VK_NEXT:	ptr = L"ctrl-page-down"; break;
		case VK_HOME:	ptr = L"ctrl-home"; break;
		case VK_END:	ptr = L"ctrl-end"; break;
		case VK_SPACE:	ptr = L"ctrl-space"; break;

		case VK_DIVIDE:   ptr = L"ctrl-kp-divide"; break;
		case VK_MULTIPLY: ptr = L"ctrl-kp-multiply"; break;
		case VK_SUBTRACT: ptr = L"ctrl-kp-minus"; break;
		case VK_ADD:	  ptr = L"ctrl-kp-plus"; break;
		case VK_RETURN:   ptr = L"ctrl-enter"; break;

		case VK_F1:	ptr = L"ctrl-f1"; break;
		case VK_F2:	ptr = L"ctrl-f2"; break;
		case VK_F3:	ptr = L"ctrl-f3"; break;
		case VK_F4:	ptr = L"ctrl-f4"; break;
		case VK_F5:	ptr = L"ctrl-f5"; break;
		case VK_F6:	ptr = L"ctrl-f6"; break;
		case VK_F7:	ptr = L"ctrl-f7"; break;
		case VK_F8:	ptr = L"ctrl-f8"; break;
		case VK_F9:	ptr = L"ctrl-f9"; break;
		case VK_F10:	ptr = L"ctrl-f10"; break;
		case VK_F11:	ptr = L"ctrl-f11"; break;
		case VK_F12:
			SendMessage(hwnd, WM_SYSCOMMAND,
			maxed ? SC_RESTORE : SC_MAXIMIZE, 0);

			maxed ^= 1;

			break;
		}
	}
	else
	{
		switch(c)
		{
		case VK_UP:	ptr = L"cursor-up"; break;
		case VK_DOWN:	ptr = L"cursor-down"; break;
		case VK_LEFT:	ptr = L"cursor-left"; break;
		case VK_RIGHT:	ptr = L"cursor-right"; break;
		case VK_PRIOR:	ptr = L"page-up"; break;
		case VK_NEXT:	ptr = L"page-down"; break;
		case VK_HOME:	ptr = L"home"; break;
		case VK_END:	ptr = L"end"; break;

		case VK_TAB:	ptr = L"tab"; break;
		case VK_RETURN: ptr = L"enter"; break;
		case VK_BACK:	ptr = L"backspace"; break;
		case VK_DELETE: ptr = L"delete"; break;
		case VK_INSERT: ptr = L"insert"; break;

		case VK_DIVIDE:   ptr = L"kp-divide"; break;
		case VK_MULTIPLY: ptr = L"kp-multiply"; break;
		case VK_SUBTRACT: ptr = L"kp-minus"; break;
		case VK_ADD:	  ptr = L"kp-plus"; break;

		case VK_F1:	ptr = L"f1"; break;
		case VK_F2:	ptr = L"f2"; break;
		case VK_F3:	ptr = L"f3"; break;
		case VK_F4:	ptr = L"f4"; break;
		case VK_F5:	ptr = L"f5"; break;
		case VK_F6:	ptr = L"f6"; break;
		case VK_F7:	ptr = L"f7"; break;
		case VK_F8:	ptr = L"f8"; break;
		case VK_F9:	ptr = L"f9"; break;
		case VK_F10:	ptr = L"f10"; break;
		case VK_F11:	ptr = L"f11"; break;
		case VK_F12:	ptr = L"f12"; break;
		}
	}

	if(ptr != NULL)
	{
		mp_process_event(MPDM_S(ptr));
/*		is_wm_keydown = 1;*/
	}

	/* force redraw */
	InvalidateRect(hwnd, NULL, TRUE);
}


#define ctrl(c) ((c) & 31)

static void win32_akey(int k)
/* win32 alphanumeric key processing */
{
	wchar_t c[2];
	wchar_t * ptr = NULL;

/*	if (is_wm_keydown) return;*/
	switch(k)
	{
	case ctrl(' '):	ptr = L"ctrl-space"; break;
	case ctrl('a'): ptr = L"ctrl-a"; break;
	case ctrl('b'): ptr = L"ctrl-b"; break;
	case ctrl('c'): ptr = L"ctrl-c"; break;
	case ctrl('d'): ptr = L"ctrl-d"; break;
	case ctrl('e'): ptr = L"ctrl-e"; break;
	case ctrl('f'): ptr = L"ctrl-f"; break;
	case ctrl('g'): ptr = L"ctrl-g"; break;
	case ctrl('h'): ptr = L"ctrl-h"; break;
	case ctrl('i'): ptr = L"ctrl-i"; break;
	case ctrl('j'): ptr = L"ctrl-j"; break;
	case ctrl('k'): ptr = L"ctrl-k"; break;
	case ctrl('l'): ptr = L"ctrl-l"; break;
	case ctrl('m'): ptr = L"ctrl-m"; break;
	case ctrl('n'): ptr = L"ctrl-n"; break;
	case ctrl('o'): ptr = L"ctrl-o"; break;
	case ctrl('p'): ptr = L"ctrl-p"; break;
	case ctrl('q'): ptr = L"ctrl-q"; break;
	case ctrl('r'): ptr = L"ctrl-r"; break;
	case ctrl('s'): ptr = L"ctrl-s"; break;
	case ctrl('t'): ptr = L"ctrl-t"; break;
	case ctrl('u'): ptr = L"ctrl-u"; break;
	case ctrl('v'): ptr = L"ctrl-v"; break;
	case ctrl('w'): ptr = L"ctrl-w"; break;
	case ctrl('x'): ptr = L"ctrl-x"; break;
	case ctrl('y'): ptr = L"ctrl-y"; break;
	case ctrl('z'): ptr = L"ctrl-z"; break;

	default:
		/* this is probably very bad */
		c[0] = (wchar_t) k;
		c[1] = L'\0';
		ptr = c;
		break;
	}

	if(ptr != NULL)
		mp_process_event(MPDM_S(ptr));

	/* force redraw */
	InvalidateRect(hwnd, NULL, TRUE);
}


long STDCALL WndProc(HWND hwnd, UINT msg, UINT wparam, LONG lparam)
/* main window Proc */
{
	int x, y;
	LPNMHDR p;
	char * ptr = NULL;

	switch(msg)
	{
/*	case WM_CREATE:

		is_wm_keydown = 0;
		DragAcceptFiles(hwnd,TRUE);
		return(0);
*/
/*	case WM_DROPFILES:

		(void) load_dropped_files ((HANDLE) wparam, hwnd);
		return(0);
*/
/*	case WM_KEYUP:

		is_wm_keydown = 0;
		return(0);
*/
	case WM_KEYDOWN:

		win32_vkey(wparam);
		return(0);

	case WM_CHAR:

		win32_akey(wparam);
		return(0);

/*	case WM_VSCROLL:

		switch(LOWORD(wparam))
		{
		case SB_PAGEUP: ptr="move-page-up"; break;
		case SB_PAGEDOWN: ptr="move-page-down"; break;
		case SB_LINEUP: ptr="move-up"; break;
		case SB_LINEDOWN: ptr="move-down"; break;
 		case SB_THUMBPOSITION:
 		case SB_THUMBTRACK:
 		  sb_thumbscroll(LOWORD(wparam),HIWORD(wparam));
 		  return(0);
		}

		if(ptr != NULL)
			mpi_process('\0', NULL, ptr);

		return(0);
*/
	case WM_PAINT:
		win32_draw(hwnd, mp_get_active());
		return(0);

	case WM_SIZE:

		update_window_size();
		InvalidateRect(hwnd, NULL, TRUE);
		return(0);

/*		if (IsIconic(hwnd)) return 0;
		if(_mpv_font_width && _mpv_font_height)
		{
			_mpv_x_size=(LOWORD(lparam)/_mpv_font_width)+1;
			_mpv_y_size=((HIWORD(lparam)-_tab_height)/_mpv_font_height)+1;

		}

		MoveWindow(hwtabs,0,0,LOWORD(lparam),_tab_height,TRUE);

		return(0);
*/
/*	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:

		x=LOWORD(lparam);
		y=HIWORD(lparam) - _tab_height;

		x/=_mpv_font_width;
		y/=_mpv_font_height;

		mp_move_xy(_mp_active,x,y+_mp_active->vy);
		mp_move_bol(_mp_active);
		mp_move_to_visual_column(_mp_active,x);

		mpi_draw_all(_mp_active);

		switch(msg)
		{
		case WM_LBUTTONDOWN: ptr="mouse-left-button"; break;
		case WM_RBUTTONDOWN: ptr="mouse-right-button"; break;
		case WM_MBUTTONDOWN: ptr="mouse-middle-button"; break;
		}

		if(ptr != NULL)
			mpi_process('\0', ptr, NULL);

		return(0);
*/
/*	case WM_MOUSEWHEEL:

		if((int)wparam > 0)
			ptr="mouse-wheel-up";
		else
			ptr="mouse-wheel-down";

		if(ptr != NULL)
			mpi_process('\0', ptr, NULL);

		return(0);
*/
/*	case WM_COMMAND:

		_mpv_amenu(LOWORD(wparam));

		return(0);
*/
	case WM_CLOSE:

		DestroyWindow(hwnd);
		return(0);

	case WM_DESTROY:
		PostQuitMessage(0);
		return(0);

/*	case WM_NOTIFY:
		p=(LPNMHDR)lparam;

		if(p->code==TCN_SELCHANGE)
		{
			y=TabCtrl_GetCurSel(hwtabs);

			for(t=_mp_txts,x=0;t!=NULL;t=t->next,x++)
			{
				if(x==y)
				{
					_mp_active=t;
					break;
				}
			}

			mpi_draw_all(_mp_active);
		}

		return(0);
*/	}

	if(mp_exit_requested)
	{
		PostMessage(hwnd, WM_CLOSE, 0, 0);
		mp_exit_requested = 0;
	}

	return(DefWindowProc(hwnd, msg, wparam, lparam));
}


static mpdm_t win32_drv_startup(mpdm_t a)
{
/*	_mpv_create_colors();
*/

	return(NULL);
}


static mpdm_t win32_drv_shutdown(mpdm_t a)
{
	SendMessage(hwnd, WM_CLOSE, 0, 0);

	return(NULL);
}


static mpdm_t win32_drv_main_loop(mpdm_t a)
{
	MSG msg;

	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return(NULL);
}


int win32_drv_init(void)
{
	WNDCLASS wc;
	RECT r;

	win32_driver = mpdm_ref(MPDM_H(0));

	mpdm_hset_s(win32_driver, L"driver", MPDM_LS(L"win32"));
	mpdm_hset_s(win32_driver, L"startup", MPDM_X(win32_drv_startup));
	mpdm_hset_s(win32_driver, L"main_loop", MPDM_X(win32_drv_main_loop));
	mpdm_hset_s(win32_driver, L"shutdown", MPDM_X(win32_drv_shutdown));

	win32_window = MPDM_H(0);
	mpdm_hset_s(win32_driver, L"window", win32_window);

	mpdm_hset_s(mp, L"drv", win32_driver);

	InitCommonControls();

	/* register the window */
	wc.style = CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinst;
	wc.hIcon = LoadIcon(hinst,"MP_ICON");
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "minimumprofit5.x";

	if(!RegisterClass(&wc))
		return(0);

	/* create the window */
	hwnd = CreateWindow("minimumprofit5.x", "mp " VERSION,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VSCROLL,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
/*		NULL, _mmenu, hinst, NULL);*/
		NULL, NULL, hinst, NULL);

	if(hwnd == NULL)
		return(0);

/*	mpv_add_menu("");

	DrawMenuBar(hwnd);
*/
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	GetClientRect(hwnd, &r);

/*	hwtabs = CreateWindow(WC_TABCONTROL, "tab",
		WS_CHILD | TCS_TABS | TCS_SINGLELINE | TCS_FOCUSNEVER,
		0, 0, r.right - r.left, _tab_height, hwnd, NULL, hinst, NULL);

	SendMessage(hwtabs, WM_SETFONT, 
		(WPARAM) GetStockObject(ANSI_VAR_FONT), 0);

	ShowWindow(hwtabs, SW_SHOW);
	UpdateWindow(hwtabs);
*/

	return(1);
}

#else /* CONFOPT_WIN32 */

int win32_drv_init(void)
{
	/* no Win32 */
	return(0);
}

#endif /* CONFOPT_WIN32 */
