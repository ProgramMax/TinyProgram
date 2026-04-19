// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "EntryPoint.hpp"


#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>


#include "DynamicLibrary.hpp"



// Globals used in WindowProcedure
typedef LRESULT (_stdcall *DefWindowProcA_type)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static DefWindowProcA_type def_window_proc_g;

typedef VOID (_stdcall *PostQuitMessage_type)(int nExitCode);
static PostQuitMessage_type post_quit_message_g;

typedef BOOL (_stdcall *DestroyWindow_type)(HWND hWnd);
static DestroyWindow_type destroy_window_g;


static LRESULT CALLBACK WindowProcedure(HWND window_handle, UINT message, WPARAM wparam, LPARAM lparam) {
	switch(message) {
	case WM_CLOSE:
		destroy_window_g(window_handle);
		break;
	case WM_DESTROY:
		post_quit_message_g(0);
		break;
	default:
		return def_window_proc_g(window_handle, message, wparam, lparam);
	}
	return 0;
}


extern "C" int _stdcall entry_point() {
	using namespace TinyProgram;

	//LPSTR command_line = GetCommandLine();
	//WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), ...)

	setup_dynamic_library_system();

	// Load user32.dll
	Expected<DynamicLibrary, open_dynamic_library_error::Enum> user32 = open_dynamic_library("user32.dll");
	if (!user32.has_value()) {
		return -1;
	}



	// Register the Window Class
	typedef HICON (_stdcall *LoadIconA_type)(HINSTANCE hInstance, LPCSTR lpIconName);
	const short LoadIconA_ordinal = 405;
	LoadIconA_type load_icon = user32->get_function(LoadIconA_ordinal, (LoadIconA_type)NULL);

	typedef HCURSOR (_stdcall *LoadCursorA_type)(HINSTANCE hInstance, LPCSTR lpCursorName);
	const short LoadCursorA_ordinal = 401;
	LoadCursorA_type load_cursor = user32->get_function(LoadCursorA_ordinal, (LoadCursorA_type)NULL);

	HMODULE instance = GetModuleHandle(NULL);
	const char class_name[] = "myWindowClass";
	WNDCLASSEX wc;
	
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = 0;
	wc.lpfnWndProc   = WindowProcedure;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = instance;
	wc.hIcon         = load_icon(NULL, IDI_APPLICATION);
	wc.hCursor       = load_cursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = class_name;
	wc.hIconSm       = load_icon(NULL, IDI_APPLICATION);

	typedef ATOM (_stdcall *RegisterClassExA_type)(const WNDCLASSEXA *unnamedParam1);
	const short RegisterClassExA_ordinal = 473;
	RegisterClassExA_type register_class_ex = user32->get_function(RegisterClassExA_ordinal, (RegisterClassExA_type)NULL);

	if (!register_class_ex(&wc)) {
		return -2;
	}



	// Before we create the window, set the function pointers called from within WindowProcedure()
	const short DefWindowProcA_ordinal = 133;
	def_window_proc_g   = user32->get_function(DefWindowProcA_ordinal, (DefWindowProcA_type)NULL);
	const short PostQuitMessage_ordinal = 465;
	post_quit_message_g = user32->get_function(PostQuitMessage_ordinal, (PostQuitMessage_type)NULL);
	const short DestroyWindow_ordinal = 142;
	destroy_window_g    = user32->get_function(DestroyWindow_ordinal, (DestroyWindow_type)NULL);



	// Create the window
	typedef HWND (_stdcall *CreateWindowExA_type)(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
	const short CreateWindowExA_ordinal = 91;
	CreateWindowExA_type create_window_ex = user32->get_function(CreateWindowExA_ordinal, (CreateWindowExA_type)NULL);

	HWND window_handle = create_window_ex(WS_EX_CLIENTEDGE, class_name, "Window title", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 240, 120, NULL, NULL, instance, NULL);
	if (window_handle == NULL) {
		return -3;
	}



	// TODO: Set the default GUI font instead of System font for all controls via:
	//SendMessage(window_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), FALSE);



	// Show the window
	typedef BOOL (_stdcall *ShowWindow_type)(HWND hWnd, int nCmdShow);
	const short ShowWindow_ordinal = 582;
	ShowWindow_type show_window = user32->get_function(ShowWindow_ordinal, (ShowWindow_type)NULL);

	typedef BOOL (_stdcall *UpdateWindow_type)(HWND hWnd);
	const short UpdateWindow_ordinal = 618;
	UpdateWindow_type update_window = user32->get_function(UpdateWindow_ordinal, (UpdateWindow_type)NULL);


	// TODO: Actually get the startup info: https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/ns-processthreadsapi-startupinfoa
	const int show_command = SW_SHOWDEFAULT;
	show_window(window_handle, show_command);
	update_window(window_handle);



	// Run the message pump
	typedef BOOL (_stdcall *GetMessageA_type)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
	const short GetMessageA_ordinal = 291;
	GetMessageA_type get_message = user32->get_function(GetMessageA_ordinal, (GetMessageA_type)NULL);

	typedef LRESULT (_stdcall *DispatchMessageA_type)(const MSG *lpMsg);
	const short DispatchMessageA_ordinal = 147;
	DispatchMessageA_type dispatch_message = user32->get_function(DispatchMessageA_ordinal, (DispatchMessageA_type)NULL);

	typedef BOOL (_stdcall *TranslateMessage_type)(const MSG *lpMsg);
	const short TranslateMessage_ordinal = 606;
	TranslateMessage_type translate_message = user32->get_function(TranslateMessage_ordinal, (TranslateMessage_type)NULL);


	MSG message;
	while (get_message(&message, NULL, 0, 0) > 0) {
		translate_message(&message);
		dispatch_message(&message);
	}


	return message.wParam;
}