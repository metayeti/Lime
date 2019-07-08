/*
 *  Lime
 *
 *  Utility for Lime datafile creation.
 *
 *  interface.cpp
 *  Implements the class for console output.
 *
 */

 /*
  * Copyright (c) 2019 Danijel Durakovic
  *
  * Permission is hereby granted, free of charge, to any person obtaining a copy of
  * this software and associated documentation files (the "Software"), to deal in
  * the Software without restriction, including without limitation the rights to
  * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
  * of the Software, and to permit persons to whom the Software is furnished to do
  * so, subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be included in all
  * copies or substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  *
  */

#include "interface.h"
#include <cstdio>
#include <cstdarg>
#if defined(_WIN32)
	#include <Windows.h>
#endif

#if defined(_WIN32)
WORD Lime::Interface::GetConsoleTextAttribute()
{
	if (!hConsole)
	{
		return 0;
	}
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
	return consoleInfo.wAttributes;
}
#endif

void Lime::Interface::setConsoleColor(Color color)
{
	int colorCode = static_cast<int>(color);
#if defined(_WIN32)
	if (!hConsole)
	{
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	}
	if (colorsToRestore < 0)
	{
		colorsToRestore = GetConsoleTextAttribute();
	}
	SetConsoleTextAttribute(hConsole, colorCode);
#else
	print("\033[1;%dm", colorCode);
#endif
}

void Lime::Interface::restoreConsoleColor()
{
#if defined(_WIN32)
	if (colorsToRestore < 0 || !hConsole)
	{
		return;
	}
	SetConsoleTextAttribute(hConsole, colorsToRestore);
	colorsToRestore = -1;
#else
	print("\033[1;0m");
#endif
}

void Lime::Interface::print(const char* format, ...)
{
	va_list argptr;
	va_start(argptr, format);
#if defined(_WIN32)
	vfprintf_s(stdout, format, argptr);
#else
	vfprintf(stdout, format, argptr);
#endif
	va_end(argptr);
}