// This file is based on the "Console.h" implementation by Jaded Hobo

#pragma once

#include <iostream>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#endif

namespace ConsoleColors
{
    struct State
    {
        State(unsigned short code) : code_(code) {}
        unsigned short code_;
    };
    struct FgColor
    {
        FgColor(unsigned short color) : color_(color) {}
        unsigned short color_;
    };
    struct BgColor
    {
        BgColor(unsigned short color) : color_(color) {}
        unsigned short color_;
    };

#ifdef _WIN32
    static const State stClear      ( 0 );
    static const State stBold       ( FOREGROUND_INTENSITY );
    static const FgColor fgBlack    ( 0 );
    static const FgColor fgLoRed    ( FOREGROUND_RED   );
    static const FgColor fgLoGreen  ( FOREGROUND_GREEN ); 
    static const FgColor fgLoBlue   ( FOREGROUND_BLUE  ); 
    static const FgColor fgLoCyan   ( FOREGROUND_GREEN | FOREGROUND_BLUE ); 
    static const FgColor fgLoMagenta( FOREGROUND_RED     | FOREGROUND_BLUE ); 
    static const FgColor fgLoYellow ( FOREGROUND_RED     | FOREGROUND_GREEN ); 
    static const FgColor fgLoWhite  ( FOREGROUND_RED     | FOREGROUND_GREEN | FOREGROUND_BLUE ); 
    static const FgColor fgGray     ( FOREGROUND_INTENSITY ); 
    static const FgColor fgHiWhite  ( FOREGROUND_RED     | FOREGROUND_GREEN | FOREGROUND_BLUE   | FOREGROUND_INTENSITY ); 
    static const FgColor fgHiBlue   ( FOREGROUND_BLUE    | FOREGROUND_INTENSITY ); 
    static const FgColor fgHiGreen  ( FOREGROUND_GREEN   | FOREGROUND_INTENSITY ); 
    static const FgColor fgHiRed    ( FOREGROUND_RED     | FOREGROUND_INTENSITY ); 
    static const FgColor fgHiCyan   ( FOREGROUND_GREEN | FOREGROUND_BLUE    | FOREGROUND_INTENSITY ); 
    static const FgColor fgHiMagenta( FOREGROUND_RED     | FOREGROUND_BLUE | FOREGROUND_INTENSITY ); 
    static const FgColor fgHiYellow ( FOREGROUND_RED     | FOREGROUND_GREEN  | FOREGROUND_INTENSITY );
    static const BgColor bgBlack    ( 0 ); 
    static const BgColor bgLoRed    ( BACKGROUND_RED   ); 
    static const BgColor bgLoGreen  ( BACKGROUND_GREEN ); 
    static const BgColor bgLoBlue   ( BACKGROUND_BLUE  ); 
    static const BgColor bgLoCyan   ( BACKGROUND_GREEN   | BACKGROUND_BLUE ); 
    static const BgColor bgLoMagenta( BACKGROUND_RED     | BACKGROUND_BLUE ); 
    static const BgColor bgLoYellow ( BACKGROUND_RED     | BACKGROUND_GREEN ); 
    static const BgColor bgLoWhite  ( BACKGROUND_RED     | BACKGROUND_GREEN | BACKGROUND_BLUE ); 
    static const BgColor bgGray     ( BACKGROUND_INTENSITY ); 
    static const BgColor bgHiWhite  ( BACKGROUND_RED     | BACKGROUND_GREEN | BACKGROUND_BLUE   | BACKGROUND_INTENSITY ); 
    static const BgColor bgHiBlue   ( BACKGROUND_BLUE    | BACKGROUND_INTENSITY ); 
    static const BgColor bgHiGreen  ( BACKGROUND_GREEN   | BACKGROUND_INTENSITY ); 
    static const BgColor bgHiRed    ( BACKGROUND_RED     | BACKGROUND_INTENSITY ); 
    static const BgColor bgHiCyan   ( BACKGROUND_GREEN   | BACKGROUND_BLUE    | BACKGROUND_INTENSITY ); 
    static const BgColor bgHiMagenta( BACKGROUND_RED     | BACKGROUND_BLUE | BACKGROUND_INTENSITY ); 
    static const BgColor bgHiYellow ( BACKGROUND_RED     | BACKGROUND_GREEN  | BACKGROUND_INTENSITY );
#else
    static const State stClear      ( 0 );
    static const State stBold       ( 1 );
    static const FgColor fgBlack    ( 0x0000 + 30 );
    static const FgColor fgLoRed    ( 0x0000 + 31 );
    static const FgColor fgLoGreen  ( 0x0000 + 32 );
    static const FgColor fgLoYellow ( 0x0000 + 33 );
    static const FgColor fgLoBlue   ( 0x0000 + 34 );
    static const FgColor fgLoMagenta( 0x0000 + 35 );
    static const FgColor fgLoCyan   ( 0x0000 + 36 );
    static const FgColor fgLoWhite  ( 0x0000 + 37 );
    static const FgColor fgGray     ( 0x0100 + 30 );
    static const FgColor fgHiRed    ( 0x0100 + 31 );
    static const FgColor fgHiGreen  ( 0x0100 + 32 );
    static const FgColor fgHiYellow ( 0x0100 + 33 );
    static const FgColor fgHiBlue   ( 0x0100 + 34 );
    static const FgColor fgHiMagenta( 0x0100 + 35 );
    static const FgColor fgHiCyan   ( 0x0100 + 36 );
    static const FgColor fgHiWhite  ( 0x0100 + 37 );
    static const BgColor bgBlack    ( 40 );
    static const BgColor bgLoRed    ( 41 );
    static const BgColor bgLoGreen  ( 42 );
    static const BgColor bgLoBlue   ( 44 );
    static const BgColor bgLoCyan   ( 46 );
    static const BgColor bgLoMagenta( 45 );
    static const BgColor bgLoYellow ( 43 );
    static const BgColor bgLoWhite  ( 47 );
    static const BgColor bgGray     ( 40 );
    static const BgColor bgHiRed    ( 41 );
    static const BgColor bgHiGreen  ( 42 );
    static const BgColor bgHiBlue   ( 44 );
    static const BgColor bgHiCyan   ( 46 );
    static const BgColor bgHiMagenta( 45 );
    static const BgColor bgHiYellow ( 43 );
    static const BgColor bgHiWhite  ( 47 );
#endif
    
#ifdef _WIN32
    static class con_dev
    {
    public:
        con_dev() 
        { 
            hCon = GetStdHandle( STD_OUTPUT_HANDLE );
        }

        void Clear()
        {
            COORD coordScreen = { 0, 0 };
            
            GetInfo(); 
            FillConsoleOutputCharacter( hCon, TEXT(' '),
                                        dwConSize, 
                                        coordScreen,
                                        &cCharsWritten ); 
            GetInfo(); 
            FillConsoleOutputAttribute( hCon,
                                        csbi.wAttributes,
                                        dwConSize,
                                        coordScreen,
                                        &cCharsWritten ); 
            SetConsoleCursorPosition( hCon, coordScreen ); 
        }
        
        void SetState(State s)
        {
            GetInfo();
            csbi.wAttributes &= bgHiWhite.color_;
            csbi.wAttributes |= s.code_;
            SetConsoleTextAttribute( hCon, csbi.wAttributes );
        }

        void SetFgColor(FgColor c)
        {
            GetInfo();
            csbi.wAttributes &= bgHiWhite.color_;
            csbi.wAttributes |= c.color_; 
            SetConsoleTextAttribute( hCon, csbi.wAttributes );
        }

        void SetBgColor(BgColor c)
        {
            GetInfo();
            csbi.wAttributes &= fgHiWhite.color_;
            csbi.wAttributes |= c.color_; 
            SetConsoleTextAttribute( hCon, csbi.wAttributes );
        }

    private:
        void GetInfo()
        {
            GetConsoleScreenBufferInfo( hCon, &csbi );
            dwConSize = csbi.dwSize.X * csbi.dwSize.Y; 
        }

    private:
        HANDLE hCon;
        DWORD cCharsWritten; 
        CONSOLE_SCREEN_BUFFER_INFO  csbi; 
        DWORD dwConSize;
    } console;

    inline ostream& clr( ostream& os )
    {
        os.flush();
        console.Clear();
        return os;
    };
    
    inline ostream& operator << ( ostream& os, State s )
    {
        os.flush();
        console.SetState(s);
        return os;
    }
    
    inline ostream& operator << ( ostream& os, FgColor c )
    {
        os.flush();
        console.SetFgColor(c);
        return os;
    }
    
    inline ostream& operator << ( ostream& os, BgColor c )
    {
        os.flush();
        console.SetBgColor(c);
        return os;
    }

#elif defined(__APPLE__) || defined(__linux__)
    inline ostream& clr( ostream& os )
    {
        os.flush();
        os << char(27) << "[0m";
        return os;
    };
    
    inline ostream& operator << ( ostream& os, State s )
    {
        os.flush();
        os << char(27) << "[" << s.code_ << "m";
        return os;
    }
    
    inline ostream& operator << ( ostream& os, FgColor c )
    {
        os.flush();
        int attr = c.color_ >> 8;
        int col = c.color_ & 0xff;
        os << char(27) << "[" << attr << ";" << col << "m";
        return os;
    }
    
    inline ostream& operator << ( ostream& os, BgColor c )
    {
        os.flush();
        int attr = c.color_ >> 8;
        int col = c.color_ & 0xff;
        os << "\0x1B[" << attr << ";" << col << "m";
        return os;
    }
#else
    inline ostream& clr( ostream& os )
    {
        os.flush();
        return os;
    };
    
    inline ostream& operator << ( ostream& os, State s )
    {
        os.flush();
        return os;
    }
    
    inline ostream& operator << ( ostream& os, FgColor c )
    {
        os.flush();
        return os;
    }
    
    inline ostream& operator << ( ostream& os, BgColor c )
    {
        os.flush();
        return os;
    }
#endif
}
