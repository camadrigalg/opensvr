#pragma  once

#include "csExport.h"

namespace Updator {
	CONSOLE_API void InitializeWaitbar(char* str);
	CONSOLE_API void UpdateWaitbar(float nValue);
	CONSOLE_API void CloseWaitbar();
}