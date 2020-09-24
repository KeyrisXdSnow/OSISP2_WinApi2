#include <iostream>

#include <windows.h>
#include <tchar.h>
#include <gdiplus.h>
#include <fstream>
#include "vector"

using namespace std;

vector<string> text;

const int columnAmount = 6;
const int penWidth = 4;

HFONT hfnt;
HPEN hpen1;

void drawVerticalBorders(HDC hdc, int windowWidth, int windowHeight) {

    int columnWidth = windowWidth / columnAmount;

    for (int i = 0; i < columnAmount; i++) {
        MoveToEx(hdc, i * columnWidth, 0, NULL);
        LineTo(hdc, i * columnWidth, windowHeight);
    }
    MoveToEx(hdc, windowWidth-1, 0, NULL);
    LineTo(hdc,windowWidth-1, windowHeight);
}

void drawHorizontalBorder(HDC hdc, int windowWidth, int lineHeight) {

    MoveToEx(hdc, 0, lineHeight, NULL);
    LineTo(hdc, windowWidth, lineHeight);

}

void drawText(HDC hdc, int windowWidth, int windowHeight) {

    int columnWidth = windowWidth / columnAmount;
    int textHeightTop;

    for (int i = 0; i < text.size();) {

        RECT rect;
        rect.top = 5 * 0.5 + textHeightTop;

        int nextColumnHeight;

        for (int j = 0; j < columnAmount & i < text.size(); j++) {

            const CHAR *str = text[i++].c_str();

            rect.left = j * columnWidth + 10 * 0.5;
            rect.right = (j + 1) * columnWidth - 10 * 0.5;

            int textHeight = DrawText(hdc, (LPCSTR) str, strlen(str), &rect,DT_VCENTER | DT_EDITCONTROL | DT_WORDBREAK | DT_NOCLIP);

            if (textHeight > nextColumnHeight)
                nextColumnHeight = textHeight;

        }

        textHeightTop += nextColumnHeight + 5;

        drawHorizontalBorder(hdc, windowWidth, textHeightTop); // —
        drawVerticalBorders(hdc, windowWidth, textHeightTop); // |
    }
    drawHorizontalBorder(hdc, windowWidth, 1);
}

void initCompatibleDC (HWND hWnd, LPPAINTSTRUCT oldps,int windowWidth, int windowHeight) {

    RECT rc;
    HDC compatibleDC;
    HBITMAP compatibleHBitmap, oldHBitmap;

    // создаём новое окно
    GetClientRect(hWnd, &rc); // получаем размер нашего окна
    compatibleDC = CreateCompatibleDC(oldps->hdc); // // создаем memory DC.
    compatibleHBitmap = CreateCompatibleBitmap(oldps->hdc, rc.right - rc.left, rc.bottom - rc.top); // создаем bitmap перекрывающий наше старое окно
    oldHBitmap = (HBITMAP) SelectObject(compatibleDC, compatibleHBitmap); // отображаем DC

    // закрашиваем compatibleDC тем же фоном
    HBRUSH hbrBkGnd = CreateSolidBrush(RGB(0,0,0));
    FillRect(compatibleDC, &rc, hbrBkGnd);
    DeleteObject(hbrBkGnd);

    // настраиваем, цвет фона, шрифты и т.д.
    SetBkMode(compatibleDC, TRANSPARENT);

    if (hpen1)
        SelectObject(compatibleDC, hpen1);

    //SetTextColor(compatibleDC, GetSysColor(COLOR_WINDOWTEXT)); //Render the image into the offscreen DC
    SetTextColor(compatibleDC,RGB(104, 204, 126));
    if (hfnt)
        SelectObject(compatibleDC, hfnt);

    drawText(compatibleDC, windowWidth, windowHeight);
    BitBlt(oldps->hdc,rc.left, rc.top,rc.right-rc.left, rc.bottom-rc.top,compatibleDC,0, 0,SRCCOPY);

    SelectObject(compatibleDC, oldHBitmap); // Done with off-screen bitmap and DC

    DeleteObject(compatibleHBitmap);
    DeleteDC(compatibleDC);

}

void drawTable(HDC hdc, int windowWidth, int windowHeight) {
    drawText(hdc, windowWidth, windowHeight);
}


int readFile() {


    ifstream fs(R"(C:\Users\Keyris\CLionProjects\Laba2\text.txt)", ios::in | ios::binary);
    string out_s;

    if (!fs)
        return 1;

    while (!fs.eof()) {
        getline(fs, out_s);
        //out_s = out_s.substr(0, out_s.size() - 1); приколы с кодировками :{
        if (!out_s.empty())
            text.push_back(out_s);
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) { // Обработчик сообщений

    PAINTSTRUCT ps;

    static int width, height;

    switch (message) {

        case WM_ERASEBKGND: {
            return (LRESULT)1;
        }

        case WM_CREATE : {
            hpen1 = CreatePen(PS_SOLID, penWidth, RGB(106, 230, 132));
            hfnt = CreateFont(20,10,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,VARIABLE_PITCH,"Bahnschrift");
            break;
        }

        case WM_SIZE : {
            width = LOWORD(lParam);
            height = HIWORD(lParam);
            break;
        }

        case WM_SETFONT:
            hfnt = (HFONT) wParam;
            break;

        case WM_PAINT : {
            BeginPaint(hWnd, &ps);
            SetTextColor(GetDC(hWnd), RGB(255, 0, 0));
            initCompatibleDC(hWnd, &ps,width,height);
            EndPaint(hWnd, &ps);
            break;
        }

        case WM_DESTROY : {
            PostQuitMessage(0);
            DeleteObject(hfnt);
            DeleteObject(hpen1);
            break;
        }

        default : {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    return 0;
}

TCHAR WinName[] = _T("MainFrame");

int WINAPI _tWinMain(HINSTANCE This, HINSTANCE Prev, LPTSTR cmd, int mode) {

    readFile();

    HWND hWnd; // Дескриптор главного окна программы
    MSG msg; // Структура для хранения сообщения
    WNDCLASS wc; // Класс окна

    //wc — структура, содержащая информацию по настройке окна. Требуется заполнить следующие поля:

    wc.hInstance = This; //Дескриптор текущего приложения
    wc.lpszClassName = WinName; // Имя класса окна
    wc.lpfnWndProc = WndProc; // Имя функции для обработки сообщений.
    wc.style = CS_HREDRAW | CS_VREDRAW; // Стиль окна
    //Такой стиль определяет автоматическую перерисовку окна при изменении
    //его ширины или высоты.
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); // Стандартная иконка
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); // Стандартный курсор
    wc.lpszMenuName = NULL; // Нет меню
    wc.cbClsExtra = 0; // Нет дополнительных данных класса
    wc.cbWndExtra = 0; // Нет дополнительных данных окна

    // Заполнение окна белым цветом
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 3);
    //Дескриптор кисти, которая используется для заполнения окна.
    //Стандартная конструкция, создает системную кисть белого цвета
    //WHITE_BRUSH. Требуется явное преобразование типа — HBRUSH.

    //2. Регистрация класса окна

    if (!RegisterClass(&wc))
        return 0;

    // Создание окна
    hWnd = CreateWindow(WinName, // Имя класса окна
                        _T("Каркас Windows-приложения"), // Заголовок окна
                        WS_OVERLAPPEDWINDOW, // Стиль окна
    //WS_OVERLAPPEDWINDOW — макрос, определяющий стиль отображения стандартного окна, имеющего системное меню, заголовок, рамку для изменения размеров
    //а также кнопки минимизации, развертывания и закрытия. Можно выбрать и другой стиль. WS_OVERLAPPED — стандартное окно с рамкой;
                        CW_USEDEFAULT, // x левого верхнего угла
                        CW_USEDEFAULT, // y девого верхнего угла
                        CW_USEDEFAULT, // width
                        CW_USEDEFAULT, // Height
    // CW_USEDEFAULT означает, что система сама выберет
    // для отображения окна наиболее (с ее точки зрения) удобное место и размер.
                        HWND_DESKTOP, // Дескриптор родительского окна
                        NULL, // Нет меню
                        This, // Дескриптор приложения
                        NULL); // Дополнительной информации нет

    ShowWindow(hWnd, mode); // Показать окно
    UpdateWindow(hWnd);

    // Цикл обработки сообщений
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}