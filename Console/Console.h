#pragma once
#include "csExport.h"
#include <iostream>

class CONSOLE_API Console
{
public:
	virtual ~Console(void);
	//static void Destroy();
	static Console* Instance(bool noCreate=false);

	FILE *fstdout, *fstdin, *fstderr;

	void ShowConsole(bool show);

  void OnSize(int cx, int cy);

  HWND GetWindow();

  void SetIcon(HICON hicon);

	COORD GetOutCursorPosition();

	void SetOutCursorPosition(COORD c);

public: //static
	enum ReportType {
    EInfo=0,  //white
    ESucess,  //green
    EWarn,    //yellow
    EError,   //red
  };

  static inline void SwitchReportType(int type)
  {
    switch(type)
    {
    case ESucess://green
      SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|
        FOREGROUND_GREEN);
      break;
    case EWarn://yellow
      SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|
        FOREGROUND_RED|FOREGROUND_GREEN);
      break;
    case EError://red
      SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|
        FOREGROUND_RED);
      break;
    case EInfo: default://white
      SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|
        FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
    }
  }

  static inline std::ostream& me(const char* func_name=0, bool popup=false)
  {
    if(popup)
      ::SetForegroundWindow( Console::Instance()->GetWindow() );
		if(func_name!=0)
			std::cout << "[" << func_name << "] ";
    return std::cout;
  }

	static inline std::ostream& info()
	{
		SwitchReportType(EInfo);
		return std::cout;
	}

	static inline std::ostream& sucess()
	{
		SwitchReportType(ESucess);
		return std::cout;
	}

	static inline std::ostream& warn()
	{
		SwitchReportType(EWarn);
		return std::cout;
	}

	static inline std::ostream& error()
	{
		SwitchReportType(EError);
		return std::cout;
	}

	static inline std::ostream& endl(std::ostream& o)
	{
		o<<std::endl;
		SwitchReportType(EInfo);
		return o;
	}

private:
	//static Console* _instance;
	Console(void);
	void Attach(SHORT ConsoleHeight, SHORT ConsoleWidth);
};
