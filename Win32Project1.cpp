// Win32Project1.cpp: определяет точку входа для приложения.
//

#include "stdafx.h"
#include "Win32Project1.h"
#include <cmath>	  // sqrt
#include <map>		  // std::map
#include <vector>	  // std::vector
#include <string>     // std::wstring, std::to_wstring
#include <list>		  // их тут так много, впадлу вычищать - пусть будут
#include <iterator> 
#include <ctime>
#include <cstdlib>
#include <algorithm>

#define IDM_NEW 228				// МАКРОС, позволяет определить когда произошло нажатие кнопки начать игрулю заного
#define MAX_LOADSTRING 100

// Отправить объявления функций, включенных в этот модуль кода:
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

// аддменюс
void AddMenus(HWND);
// Прототипы логических функций
bool CorrectNumberIsTaken(int to_compare);
bool IsGameFinished();
void InitFields();

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

// Глобальные переменные для логики приложения
const int ALL_COUNT = 100;						// Полное количество клеточек
const int RANK = int(sqrt(ALL_COUNT));			// Размерность квадратной матрицы rank*rank
static HFONT newFont;							// Шрифтик
static HWND hChild[ALL_COUNT];					// Здесь храним объекты детских окошек
static HWND hWindow;							// Здесь будет основное дочернее окно 
static HWND hTimer;								// Окошечко таймера
int last_num = 0;								// = 0, т.к. в начальном состоянии не нажата ни одна клетка
int time_out = 0;								// таймер логический
bool isGameFinished = false;					// флаг окончания игры

// Dangerous! Requires high IQ 
std::map<HWND, int> links_m;					// здесь будем хранить соответствие окошка к его числу внутри
std::vector<int> init_v;						// Вектор инициализации, содержит все числа от 1 до allCount

/*		Описание логики приложения 
	1. Инициализация игрового поля происходит следующим образом:
		1.1. Каждой клетке присваивается случайное значение таким образом, 
			 что каждое числовое значение от 1 до allCount ( константа,
			 задаваемая в глобальных переменных ) встречается ровно 1 раз.
		1.2. Инициализация объекта таймера, отсчитывающего x^2*allCount секунд.
		1.3. Предполагается, что пользователю в среднем требуется больше времени
		     для поиска и клика на правильный номер с увеличением количества полей
			 в квадратной степени.

	2. Решение головоломки проходит следующим образом:
		2.1. Пользователю требуется последовательно нажать на все клетки
			 с номерами в возрастающей последовательности.
		2.2. При нажатии пользователем на клетку с номером происходит сравнение
			 номера текущей позиции с номером нажатой клетки.
			2.2.1. Если номер нажатой клетки на 1 больше номера прошлой нажатой клетки, то 
				   нажатие считывается как верное и число в клетке пропадает.
			2.2.2. Если номер нажатой клетки не равен номеру нажатой + 1, то нажатие игнорируется
		2.3. В случае, если пользователь успел нажать на все клетки за отведенное 
			 ему время, то выводится окно с надписью о победе, иначе - 
			 пользователь считается проигравшим и выводится окно с поражением.
*/

// Логические функции

// Проверка на правильность клика
// возвращается true если была нажата правильная кнопка
bool CorrectNumberIsTaken( int to_compare )
{
	if ( to_compare == last_num + 1 )
		return true;

	return false;
}

// Проверка на конец игры
// возвращается true если была нажата последняя кнопка
bool IsGameFinished()
{
	isGameFinished = last_num == ALL_COUNT ? true : false; // тернарный оператор вида "выражение" ? "значение, если правда" : "значение, если ложь"
	return isGameFinished; 
}

// Функция для старта игры, обнуляет все показатели, устанавливает таймер
void InitFields()
{
	isGameFinished = false;
	last_num = 0;
	links_m.clear();
	init_v.clear();
	time_out = pow( sqrt(double(ALL_COUNT)/10+double(ALL_COUNT)/(double(100)+ double(ALL_COUNT))), 6 ); // какая-то непонятная формула рандомно выведена, но вроде норм пашет
	// Заполняем вектор номерами клеток
	for (int i = 0; i < ALL_COUNT; i++)
		init_v.push_back( i+1 );

}

// Функция проверки на тайм аут
// Возарвщает true если таймер кончился
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

	std::string my_text = std::to_string( time_out ); // строка, показывающая сколько времени осталось до тайм аута

	switch (message)
	{
	case WM_CREATE:
		SetTimer(hWnd, 1, 1000, NULL);

		// рисуем залупу
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rt);
		SelectObject(hdc, newFont);
		DrawTextA(hdc, my_text.c_str(), my_text.length(), &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		EndPaint(hWnd, &ps);

		break;

	case WM_PAINT:
		// тут тоже
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rt);
		SelectObject(hdc, newFont);
		DrawTextA(hdc, my_text.c_str(), my_text.length(), &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		EndPaint(hWnd, &ps);

	case WM_TIMER:
		if (wParam == 1) // если прозвенел таймер, который установили мы, иначе ниче не делаем, мало ли какие таймеры ещё тут могут быть
		{
			if (isGameFinished) // ну если игра закончилась, ниче не делаем, только таймер убьём
			{
				KillTimer(hWnd, 1);
				return 0;
			}

			// перерисовать таймер, возможно работает без него, но как говорится:
			// работает - не трожь (с) Джейсон Стетхем
			InvalidateRect(hWnd, NULL, 1);
			UpdateWindow(hWnd);

			if (IsTimeOut()) // ну тут всё, проиграно, поздравляем:
			{
				MessageBox(hWnd, _T("Вы ёбаный неудачник"), _T("Конец игры"), MB_OK | MB_ICONQUESTION);
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

		sx = LOWORD(lParam) / RANK;			 //Ширина дочернего окна
		sy = HIWORD(lParam) / RANK;			 //Высота дочернего окна

		if (newFont) DeleteObject(newFont);	 //Залупа кентавра
		newFont = CreateFont(min(sx, sy), 0, 0, 0, FW_NORMAL, 0, 0, 0,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Arial"));

		for (int i = 0; i < ALL_COUNT; i++)
		{
			// рисуем много-много окошек, чтобы в них играть потом
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

	int my_num = 0;					// номер окошко(потом инициализируем, пока 0)
	auto it = links_m.find(hWnd);
	if (it != links_m.end())
		my_num = it->second;

	std::string str_num = std::to_string(my_num);			// Строковое представление числа внутри клетки
	int tmp = 0;											// Тут я уже перестала понимать, что происходит... код писался вне моего сознания
	switch (message)
	{
	case WM_CREATE:
		srand(lParam);									// Задачем генератор случайных чисел основанный на положении курсора
		tmp = rand() % init_v.size();
		my_num = init_v.at(tmp);						// Получаем случайное значение вектора

		init_v.erase(init_v.begin() + tmp);				// Удаляем это значение из вектора
		links_m.insert(std::make_pair(hWnd, my_num));	// Привязываем значение клетки к её окну

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
				MessageBox(hWnd, _T("Вы ёбаный красавчик"), _T("Конец игры"), MB_OK | MB_ICONQUESTION);
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

		} //Фоновая закраска окна
		else DefWindowProc(hWnd, message, wParam, lParam);
		break;
	default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  НАЗНАЧЕНИЕ:  обрабатывает сообщения в главном окне.
//
//  WM_COMMAND — обработать меню приложения
//  WM_PAINT — отрисовать главное окно
//  WM_DESTROY — отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i;
	static int sx, sy;
	switch (message)
	{
	case WM_CREATE:
		AddMenus(hWnd); // Добавление менюшек(начать заного, выход)

		// регаем классы дочерних окошек таймера и игрового поля
		MyRegisterFieldsClass();
		hWindow = CreateWindow(FieldsClassName, NULL, WS_CHILD |
			WS_DLGFRAME | WS_VISIBLE, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);

		MyRegisterTimerClass();
		hTimer = CreateWindow(TimerClassName, NULL, WS_CHILD |
			WS_DLGFRAME | WS_VISIBLE, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);

		break;
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED) break; //Кнопка свертывания окна

		sx = LOWORD(lParam);				 //Ширина окна
		sy = HIWORD(lParam);				 //Высота окна

		MoveWindow(hTimer, 0, 0, sx, int(0.2*sy), TRUE);
		UpdateWindow(hTimer);

		MoveWindow(hWindow, 0, int(0.2*sy), sx, int(sy*0.8), TRUE);
		UpdateWindow(hWindow);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDM_NEW:						// НАЧИНАЕМ ИГРУ СНАЧАЛА
				// Вначале уничтожим старые окна
				DestroyWindow( hWindow );		
				DestroyWindow( hTimer );

				// Обнуление всех переменных
				InitFields();

				// Построение внутренних окошек заного
				hWindow = CreateWindow(FieldsClassName, NULL, WS_CHILD |
					WS_DLGFRAME | WS_VISIBLE, 0, int(0.2*sy), sx, int(sy*0.8), hWnd, NULL, hInst, NULL);

				hTimer = CreateWindow(TimerClassName, NULL, WS_CHILD |
					WS_DLGFRAME | WS_VISIBLE, 0, 0, sx, int(0.2*sy), hWnd, NULL, hInst, NULL);

				// всегда помогает при обновлении координат, почему - хз!
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

// Мета-описание классов окошек (не интересно)
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
//  ФУНКЦИЯ: MyRegisterClass()
//
//  НАЗНАЧЕНИЕ: регистрирует класс окна.
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

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WIN32PROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32PROJECT1));

    MSG msg;

    // Цикл основного сообщения:
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
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   НАЗНАЧЕНИЕ: сохраняет обработку экземпляра и создает главное окно.
//
//   КОММЕНТАРИИ:
//
//        В данной функции дескриптор экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится на экран главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить дескриптор экземпляра в глобальной переменной

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

// Обработчик сообщений для окна "О программе".
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