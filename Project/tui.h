#pragma once
#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>  
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <conio.h>
#endif

namespace tui {
	// ===================== Color codes =====================
    enum : int {
        FG_OK = 10,
        FG_ALERT = 12,
        FG_HL = 14
    };
    // ===================== Key codes =====================
    enum : int {
        K_NONE = 0,
        K_LEFT = 1,
        K_RIGHT = 2,
        K_UP = 3,
        K_DOWN = 4,
        K_ENTER = 5,
        K_ESC = 6
    };
	// ===================== Key event =====================
    struct KeyEvent {
        int key;
        int ch;
        KeyEvent() : key(K_NONE), ch(0) {}
    };
    // ===================== Cursor & Clear =====================
#ifdef _WIN32
    inline void gotoxy(int x, int y) {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD c;
        c.X = (SHORT)((x > 0) ? (x - 1) : 0);
        c.Y = (SHORT)((y > 0) ? (y - 1) : 0);
        SetConsoleCursorPosition(h, c);
    }
	// ===================== Clear Screen =====================
    inline void clearScreen() {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        DWORD count;
        DWORD cellCount;
        COORD home = { 0, 0 };
        if (!GetConsoleScreenBufferInfo(h, &csbi)) {
            std::system("cls");
            return;
        }
        cellCount = (DWORD)csbi.dwSize.X * (DWORD)csbi.dwSize.Y;
        FillConsoleOutputCharacter(h, (TCHAR)' ', cellCount, home, &count);
        FillConsoleOutputAttribute(h, csbi.wAttributes, cellCount, home, &count);
        SetConsoleCursorPosition(h, home);
    }
	// ===================== Show Cursor =====================
    inline void showCursor() {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        if (GetConsoleCursorInfo(h, &info)) {
            info.bVisible = TRUE;
            SetConsoleCursorInfo(h, &info);
        }
    }
	// ===================== Hide Cursor =====================
    inline void hideCursor() {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        if (GetConsoleCursorInfo(h, &info)) {
            info.bVisible = FALSE;
            SetConsoleCursorInfo(h, &info);
        }
    }
	// ===================== Color =====================
    inline void setColor(int colorCode) {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        static WORD defaultAttr = 0;
        static bool cached = false;
        if (!cached) {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            if (GetConsoleScreenBufferInfo(h, &csbi)) {
                defaultAttr = csbi.wAttributes;
                cached = true;
            }
        }
        WORD attr = defaultAttr;
        switch (colorCode) {
        case FG_OK:    attr = FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
        case FG_ALERT: attr = FOREGROUND_RED | FOREGROUND_INTENSITY; break;
        case FG_HL:    attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
        default:       attr = defaultAttr; break;
        }
        SetConsoleTextAttribute(h, attr);
    }
	// ===================== Reset Color =====================
    inline void resetColor() {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(h, &csbi)) {
            SetConsoleTextAttribute(h, csbi.wAttributes);
        }
    }
	// ===================== Read Key =====================
    inline KeyEvent readKey() {
        KeyEvent e;
        int c = _getch();
        if (c == 0 || c == 224) {
            int c2 = _getch();
            switch (c2) {
            case 72: e.key = K_UP; break;    // Up
            case 80: e.key = K_DOWN; break;  // Down
            case 75: e.key = K_LEFT; break;  // Left
            case 77: e.key = K_RIGHT; break; // Right
            default: e.key = K_NONE; break;
            }
            e.ch = c2;
        }
        else if (c == 13) {
            e.key = K_ENTER; e.ch = c;
        }
        else if (c == 27) {
            e.key = K_ESC; e.ch = c;
        }
        else {            
            switch (c) {
            case 'w': case 'W': e.key = K_UP;    break;
            case 's': case 'S': e.key = K_DOWN;  break;
            case 'a': case 'A': e.key = K_LEFT;  break;
            case 'd': case 'D': e.key = K_RIGHT; break;
            default: e.key = K_NONE; break;
            }
            e.ch = c;
        }
        return e;
    }
#else
    inline void gotoxy(int x, int y) {
        if (x < 1) { x = 1; }
        if (y < 1) { y = 1; }
        std::printf("\x1b[%d;%dH", y, x);
        std::fflush(stdout);
    }
    inline void clearScreen() {
        std::printf("\x1b[2J\x1b[H");
        std::fflush(stdout);
    }
    inline void showCursor() {
        std::printf("\x1b[?25h");
        std::fflush(stdout);
    }
    inline void hideCursor() {
        std::printf("\x1b[?25l");
        std::fflush(stdout);
    }
    inline void setColor(int colorCode) {
        int ansi = 37;
        switch (colorCode) {
        case FG_OK:    ansi = 32; break; // green
        case FG_ALERT: ansi = 31; break; // red
        case FG_HL:    ansi = 33; break; // yellow
        default:       ansi = 37; break;
        }
        std::printf("\x1b[%dm", ansi);
        std::fflush(stdout);
    }
    inline void resetColor() {
        std::printf("\x1b[0m");
        std::fflush(stdout);
    }
    inline KeyEvent readKey() {
        KeyEvent e;
        int c = std::getchar();
        if (c == 27) {            
            int c1 = std::getchar();
            if (c1 == '[') {
                int c2 = std::getchar();
                switch (c2) {
                case 'A': e.key = K_UP; break;
                case 'B': e.key = K_DOWN; break;
                case 'C': e.key = K_RIGHT; break;
                case 'D': e.key = K_LEFT; break;
                default: e.key = K_ESC; break;
                }
                e.ch = c2;
            }
            else {
                e.key = K_ESC; e.ch = c1;
            }
        }
        else if (c == '\n' || c == '\r') {
            e.key = K_ENTER; e.ch = c;
        }
        else {
            switch (c) {
            case 'w': case 'W': e.key = K_UP;    break;
            case 's': case 'S': e.key = K_DOWN;  break;
            case 'a': case 'A': e.key = K_LEFT;  break;
            case 'd': case 'D': e.key = K_RIGHT; break;
            default: e.key = K_NONE; break;
            }
            e.ch = c;
        }
        return e;
    }
#endif
	// ===================== Drawing helpers =====================
    inline void drawHLine(int x, int y, int w) {
        if (w <= 0) { return; }
        gotoxy(x, y);
        for (int i = 0; i < w; ++i) {
            std::cout << "-";
        }
    }
    inline void drawVLine(int x, int y, int h) {
        for (int i = 0; i < h; ++i) {
            gotoxy(x, y + i);
            std::cout << "|";
        }
    }
    inline void drawBox(int x, int y, int w, int h, const std::string& title) {
        tui::setColor(tui::FG_HL);
        if (w < 4) { w = 4; }
        if (h < 3) { h = 3; }        
        gotoxy(x, y);             std::cout << "+";
        gotoxy(x + w - 1, y);     std::cout << "+";
        gotoxy(x, y + h - 1);     std::cout << "+";
        gotoxy(x + w - 1, y + h - 1); std::cout << "+";       
        drawHLine(x + 1, y, w - 2);
        drawHLine(x + 1, y + h - 1, w - 2);
        drawVLine(x, y + 1, h - 2);
        drawVLine(x + w - 1, y + 1, h - 2);     
        if (!title.empty() && w > 4) {
            std::string t = title;
            if ((int)t.size() > w - 4) {
                t = t.substr(0, w - 4);
            }
            gotoxy(x + 2, y);
            std::cout << " " << t << " ";
        }
    }
    inline void print_footer_hints(int x, int y, const std::string& text) {
        gotoxy(x, y);
        setColor(FG_HL);
        std::cout << text;
        resetColor();
    }
    // ===================== UX helpers =====================
    inline void press_any_key_to_back(int x, int y) {
        gotoxy(x, y);
        setColor(FG_HL);
        std::cout << "[Esc] Quay lai";
        resetColor();        
        while (true) {
            KeyEvent e = readKey();
            if (e.key == K_ESC) { break; }
        }
    }   
    inline void clear_rect(int x, int y, int w, int h) {
        for (int r = 0; r < h; ++r) {
            gotoxy(x, y + r);
            std::cout << std::string(w, ' ');
        }
    }    
    inline int _read_line_allow_esc_if_empty(int x, int y, int maxlen, std::string& out) {
        out.clear();
        tui::gotoxy(x, y);
        tui::showCursor();
        while (true) {
            tui::KeyEvent ev = tui::readKey();            
            if (ev.key == tui::K_ESC && out.empty()) {
                tui::hideCursor();
                return -1;
            }           
            if (ev.key == tui::K_ENTER) {
                tui::hideCursor();
                return 1;
            }            
            if (ev.ch == 8 || ev.ch == 127) {
                if (!out.empty()) {
                    out.pop_back();
                    int cx = x + (int)out.size();
                    tui::gotoxy(cx, y); std::cout << ' ';
                    tui::gotoxy(cx, y);
                }
                continue;
            }            
            char ch = 0;
            if (ev.ch != 0 && ev.key != tui::K_ESC && ev.key != tui::K_ENTER) {
                ch = (char)ev.ch;
            }
            if (ch >= 32 && ch <= 126) {
                if ((int)out.size() < maxlen) {
                    out.push_back(ch);
                    std::cout << ch;
                }
            }
        }
    }
}
