#include "StdAfx.h"
#include "Console.h"
#include "CharUtils.h"

#include < process.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <Windows.h>


//Console* Console::_instance = 0;

Console::Console(void)
{
	fstdin = fstdout = fstderr = 0;
	AllocConsole();
  //Delete close button for console,
  //otherwise, close console windows will close the whole program
  HWND hConsoleWnd = ::GetConsoleWindow();
  HMENU hConsoleMenu = ::GetSystemMenu(hConsoleWnd, FALSE);
  RemoveMenu(hConsoleMenu, SC_CLOSE, MF_BYCOMMAND);
	Attach(300,80);
	Console::SwitchReportType(Console::EInfo);
}

Console::~Console(void)
{
	FreeConsole();
}

void Console::Attach(SHORT ConsoleHeight, SHORT ConsoleWidth)
{
	HANDLE hStd;
  int hConHandle;
	FILE *fp;

	// redirect unbuffered STDOUT to the Console
	hStd = GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle((long)hStd, _O_TEXT);
	fstdout = fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );

	// redirect unbuffered STDIN to the Console
	hStd = GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle((long)hStd, _O_TEXT);
	fstdout = fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );

	// redirect unbuffered STDERR to the Console
	hStd = GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle((long)hStd, _O_TEXT);
	fstderr = fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );
}

void Console::ShowConsole(bool show)
{
	::ShowWindow(::GetConsoleWindow(), show);
}

void Console::OnSize(int cx, int cy)
{
  COORD size;
  size.X = cx;
  size.Y = cy;
  HANDLE hStd;
  hStd = GetStdHandle(STD_OUTPUT_HANDLE);
  ::SetConsoleScreenBufferSize(hStd, size);
}

HWND Console::GetWindow()
{
  return ::GetConsoleWindow();
}

Console* Console::Instance(bool noCreate)
{
  static Console s_console;
  return &s_console;
}

void Console::SetIcon(HICON hicon)
{
  ::SendMessage(Console::Instance()->GetWindow()
    , WM_SETICON, ICON_SMALL, (LPARAM)hicon);
  ::SendMessage(Console::Instance()->GetWindow()
    , WM_SETICON, ICON_BIG, (LPARAM)hicon);
}

COORD Console::GetOutCursorPosition()
{
	HANDLE hOut = GetStdHandle( STD_OUTPUT_HANDLE );
	CONSOLE_SCREEN_BUFFER_INFO info={0};
	GetConsoleScreenBufferInfo( hOut , &info );
	return info.dwCursorPosition;
}

void Console::SetOutCursorPosition( COORD c )
{
	HANDLE hOut = GetStdHandle( STD_OUTPUT_HANDLE );
	::SetConsoleCursorPosition(hOut, c);
}