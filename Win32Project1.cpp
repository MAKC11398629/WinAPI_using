// Win32Project1.cpp: ���������� ����� ����� ��� ����������.
//

#include "stdafx.h"
#include "Win32Project1.h"
#include <cmath>	  // sqrt
#include <map>		  // std::map
#include <vector>	  // std::vector
#include <string>     // std::wstring, std::to_wstring
#include <list>		  // �� ��� ��� �����, ������ �������� - ����� �����
#include <iterator> 
#include <ctime>
#include <cstdlib>
#include <algorithm>

#define IDM_NEW 228				// ������, ��������� ���������� ����� ��������� ������� ������ ������ ������ ������
#define MAX_LOADSTRING 100

// ��������� ���������� �������, ���������� � ���� ������ ����:
ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM				MyRegisterChildClass();
ATOM				MyRegisterFieldsClass();
ATOM				MyRegisterTimerClass();
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ChildProc(HWND, UINT, WPARAM, LPARAM);
TCHAR ChildClassName[MAX_LOADSTRING] = _T("WinChild");
TCHAR FieldsClassName[MAX_LOADSTRING] = _T("WinFields");
TCHAR TimerClassName[MAX_LOADSTRING] = _T("ITimer");

// ��������
void AddMenus(HWND);
// ��������� ���������� �������
bool CorrectNumberIsTaken(int to_compare);
bool IsGameFinished();
void InitFields();

// ���������� ����������:
HINSTANCE hInst;                                // ������� ���������
WCHAR szTitle[MAX_LOADSTRING];                  // ����� ������ ���������
WCHAR szWindowClass[MAX_LOADSTRING];            // ��� ������ �������� ����

// ���������� ���������� ��� ������ ����������
const int ALL_COUNT = 100;						// ������ ���������� ��������
const int RANK = int(sqrt(ALL_COUNT));			// ����������� ���������� ������� rank*rank
static HFONT newFont;							// �������
static HWND hChild[ALL_COUNT];					// ����� ������ ������� ������� ������
static HWND hWindow;							// ����� ����� �������� �������� ���� 
static HWND hTimer;								// �������� �������
int last_num = 0;								// = 0, �.�. � ��������� ��������� �� ������ �� ���� ������
int time_out = 0;								// ������ ����������
bool isGameFinished = false;					// ���� ��������� ����

// Dangerous! Requires high IQ 
std::map<HWND, int> links_m;					// ����� ����� ������� ������������ ������ � ��� ����� ������
std::vector<int> init_v;						// ������ �������������, �������� ��� ����� �� 1 �� allCount

/*		�������� ������ ���������� 
	1. ������������� �������� ���� ���������� ��������� �������:
		1.1. ������ ������ ������������� ��������� �������� ����� �������, 
			 ��� ������ �������� �������� �� 1 �� allCount ( ���������,
			 ���������� � ���������� ���������� ) ����������� ����� 1 ���.
		1.2. ������������� ������� �������, �������������� x^2*allCount ������.
		1.3. ��������������, ��� ������������ � ������� ��������� ������ �������
		     ��� ������ � ����� �� ���������� ����� � ����������� ���������� �����
			 � ���������� �������.

	2. ������� ����������� �������� ��������� �������:
		2.1. ������������ ��������� ��������������� ������ �� ��� ������
			 � �������� � ������������ ������������������.
		2.2. ��� ������� ������������� �� ������ � ������� ���������� ���������
			 ������ ������� ������� � ������� ������� ������.
			2.2.1. ���� ����� ������� ������ �� 1 ������ ������ ������� ������� ������, �� 
				   ������� ����������� ��� ������ � ����� � ������ ���������.
			2.2.2. ���� ����� ������� ������ �� ����� ������ ������� + 1, �� ������� ������������
		2.3. � ������, ���� ������������ ����� ������ �� ��� ������ �� ���������� 
			 ��� �����, �� ��������� ���� � �������� � ������, ����� - 
			 ������������ ��������� ����������� � ��������� ���� � ����������.
*/

// ���������� �������

// �������� �� ������������ �����
// ������������ true ���� ���� ������ ���������� ������
bool CorrectNumberIsTaken( int to_compare )
{
	if ( to_compare == last_num + 1 )
		return true;

	return false;
}

// �������� �� ����� ����
// ������������ true ���� ���� ������ ��������� ������
bool IsGameFinished()
{
	isGameFinished = last_num == ALL_COUNT ? true : false; // ��������� �������� ���� "���������" ? "��������, ���� ������" : "��������, ���� ����"
	return isGameFinished; 
}

// ������� ��� ������ ����, �������� ��� ����������, ������������� ������
void InitFields()
{
	isGameFinished = false;
	last_num = 0;
	links_m.clear();
	init_v.clear();
	time_out = pow( sqrt(double(ALL_COUNT)/10+double(ALL_COUNT)/(double(100)+ double(ALL_COUNT))), 6 ); // �����-�� ���������� ������� �������� ��������, �� ����� ���� �����
	// ��������� ������ �������� ������
	for (int i = 0; i < ALL_COUNT; i++)
		init_v.push_back( i+1 );

}

// ������� �������� �� ���� ���
// ���������� true ���� ������ ��������
bool IsTimeOut()
{
	if ( time_out == 0 )
	{
		isGameFinished = true;
		return true;
	}

	time_out--;
	return false;
}

LRESULT CALLBACK TimerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rt;

	std::string my_text = std::to_string( time_out ); // ������, ������������ ������� ������� �������� �� ���� ����

	switch (message)
	{
	case WM_CREATE:
		SetTimer(hWnd, 1, 1000, NULL);

		// ������ ������
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rt);
		SelectObject(hdc, newFont);
		DrawTextA(hdc, my_text.c_str(), my_text.length(), &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		EndPaint(hWnd, &ps);

		break;

	case WM_PAINT:
		// ��� ����
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rt);
		SelectObject(hdc, newFont);
		DrawTextA(hdc, my_text.c_str(), my_text.length(), &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		EndPaint(hWnd, &ps);

	case WM_TIMER:
		if (wParam == 1) // ���� ��������� ������, ������� ���������� ��, ����� ���� �� ������, ���� �� ����� ������� ��� ��� ����� ����
		{
			if (isGameFinished) // �� ���� ���� �����������, ���� �� ������, ������ ������ �����
			{
				KillTimer(hWnd, 1);
				return 0;
			}

			// ������������ ������, �������� �������� ��� ����, �� ��� ���������:
			// �������� - �� ����� (�) ������� �������
			InvalidateRect(hWnd, NULL, 1);
			UpdateWindow(hWnd);

			if (IsTimeOut()) // �� ��� ��, ���������, �����������:
			{
				MessageBox(hWnd, _T("�� ������ ���������"), _T("����� ����"), MB_OK | MB_ICONQUESTION);
				KillTimer(hWnd, 1);
			}
		}
		break;

	default: 
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}

LRESULT CALLBACK FieldsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int sx, sy;

	switch (message)
	{

	case WM_CREATE:
		MyRegisterChildClass();
		for(int i = 0; i < ALL_COUNT; i++)
			hChild[i] = CreateWindow(ChildClassName, NULL, WS_CHILD |
				WS_DLGFRAME | WS_VISIBLE, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
		break;
	case WM_SIZE:

		sx = LOWORD(lParam) / RANK;			 //������ ��������� ����
		sy = HIWORD(lParam) / RANK;			 //������ ��������� ����

		if (newFont) DeleteObject(newFont);	 //������ ��������
		newFont = CreateFont(min(sx, sy), 0, 0, 0, FW_NORMAL, 0, 0, 0,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Arial"));

		for (int i = 0; i < ALL_COUNT; i++)
		{
			// ������ �����-����� ������, ����� � ��� ������ �����
			MoveWindow(hChild[i], (i % RANK)*sx, (i / RANK) * sy, sx, sy, TRUE);
			UpdateWindow(hChild[i]);
		}
		break;

	default: return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}

LRESULT CALLBACK ChildProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rt;

	int my_num = 0;					// ����� ������(����� ��������������, ���� 0)
	auto it = links_m.find(hWnd);
	if (it != links_m.end())
		my_num = it->second;

	std::string str_num = std::to_string(my_num);			// ��������� ������������� ����� ������ ������
	int tmp = 0;											// ��� � ��� ��������� ��������, ��� ����������... ��� ������� ��� ����� ��������
	switch (message)
	{
	case WM_CREATE:
		srand(lParam);									// ������� ��������� ��������� ����� ���������� �� ��������� �������
		tmp = rand() % init_v.size();
		my_num = init_v.at(tmp);						// �������� ��������� �������� �������

		init_v.erase(init_v.begin() + tmp);				// ������� ��� �������� �� �������
		links_m.insert(std::make_pair(hWnd, my_num));	// ����������� �������� ������ � � ����

		break;
	case WM_LBUTTONDOWN:
		if(isGameFinished)
			break;
		if (CorrectNumberIsTaken(my_num))
		{
			last_num = my_num;
			InvalidateRect(hWnd, NULL, 1);
			UpdateWindow(hWnd);

			if (IsGameFinished())
				MessageBox(hWnd, _T("�� ������ ���������"), _T("����� ����"), MB_OK | MB_ICONQUESTION);
		}
		break;
	case WM_PAINT:
		if (my_num > last_num)
		{
			hdc = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &rt);
			SelectObject(hdc, newFont);
			DrawTextA(hdc, str_num.c_str(), str_num.length(), &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			EndPaint(hWnd, &ps);

		} //������� �������� ����
		else DefWindowProc(hWnd, message, wParam, lParam);
		break;
	default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


//
//  �������: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ����������:  ������������ ��������� � ������� ����.
//
//  WM_COMMAND � ���������� ���� ����������
//  WM_PAINT � ���������� ������� ����
//  WM_DESTROY � ��������� ��������� � ������ � ���������
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i;
	static int sx, sy;
	switch (message)
	{
	case WM_CREATE:
		AddMenus(hWnd); // ���������� �������(������ ������, �����)

		// ������ ������ �������� ������ ������� � �������� ����
		MyRegisterFieldsClass();
		hWindow = CreateWindow(FieldsClassName, NULL, WS_CHILD |
			WS_DLGFRAME | WS_VISIBLE, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);

		MyRegisterTimerClass();
		hTimer = CreateWindow(TimerClassName, NULL, WS_CHILD |
			WS_DLGFRAME | WS_VISIBLE, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);

		break;
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED) break; //������ ����������� ����

		sx = LOWORD(lParam);				 //������ ����
		sy = HIWORD(lParam);				 //������ ����

		MoveWindow(hTimer, 0, 0, sx, int(0.2*sy), TRUE);
		UpdateWindow(hTimer);

		MoveWindow(hWindow, 0, int(0.2*sy), sx, int(sy*0.8), TRUE);
		UpdateWindow(hWindow);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDM_NEW:						// �������� ���� �������
				// ������� ��������� ������ ����
				DestroyWindow( hWindow );		
				DestroyWindow( hTimer );

				// ��������� ���� ����������
				InitFields();

				// ���������� ���������� ������ ������
				hWindow = CreateWindow(FieldsClassName, NULL, WS_CHILD |
					WS_DLGFRAME | WS_VISIBLE, 0, int(0.2*sy), sx, int(sy*0.8), hWnd, NULL, hInst, NULL);

				hTimer = CreateWindow(TimerClassName, NULL, WS_CHILD |
					WS_DLGFRAME | WS_VISIBLE, 0, 0, sx, int(0.2*sy), hWnd, NULL, hInst, NULL);

				// ������ �������� ��� ���������� ���������, ������ - ��!
				InvalidateRect(hWnd, NULL, 1);
				UpdateWindow(hWnd);
				break;
		case IDM_EXIT: DestroyWindow(hWnd); break; 
		default: return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_DESTROY: PostQuitMessage(0); break;
	default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// ����-�������� ������� ������ (�� ���������)
ATOM MyRegisterTimerClass()
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = TimerProc;
	wcex.hInstance = hInst;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = TimerClassName;
	return RegisterClassEx(&wcex);
}

ATOM MyRegisterFieldsClass()
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = FieldsProc;
	wcex.hInstance = hInst;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = FieldsClassName;
	return RegisterClassEx(&wcex);
}

ATOM MyRegisterChildClass()
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = ChildProc;
	wcex.hInstance = hInst;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = ChildClassName;
	return RegisterClassEx(&wcex);
}

//
//  �������: MyRegisterClass()
//
//  ����������: ������������ ����� ����.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32PROJECT1));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WIN32PROJECT1);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	InitFields();

    // ������������� ���������� �����
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WIN32PROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // ��������� ������������� ����������:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32PROJECT1));

    MSG msg;

    // ���� ��������� ���������:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//   �������: InitInstance(HINSTANCE, int)
//
//   ����������: ��������� ��������� ���������� � ������� ������� ����.
//
//   �����������:
//
//        � ������ ������� ���������� ���������� ����������� � ���������� ����������, � �����
//        ��������� � ��������� �� ����� ������� ���� ���������.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // ��������� ���������� ���������� � ���������� ����������

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

// ���������� ��������� ��� ���� "� ���������".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void AddMenus(HWND hwnd) {

	HMENU hMenubar;
	HMENU hMenu;

	hMenubar = CreateMenu();
	hMenu = CreateMenu();

	AppendMenuW(hMenu, MF_STRING, IDM_NEW, L"&New");
	AppendMenuW(hMenu, MF_STRING, IDM_EXIT, L"&Exit");

	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenu, L"&Game");
	SetMenu(hwnd, hMenubar);
}