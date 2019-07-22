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

  /*
   *  Lime
   *
   *  Utility for Lime datafile creation.
   *
   *  interface.cpp
   *  Implements the class for console output.
   *
   */

#if defined(_WIN32)
	#include <Windows.h>
#else
	#include <unistd.h>
#endif
#include "interface.h"

namespace Lime
{
#if defined(_WIN32)
	WORD Interface::GetConsoleTextAttribute() const
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

	void Interface::setOutputColor(Interface::Color color)
	{
		bool restoreColor = color == Color::DEFAULT;

		if (!restoreColor)
		{
			int colorCode = static_cast<int>(color);
#if defined(_WIN32)
			if (colorsToRestore < 0)
			{
				colorsToRestore = GetConsoleTextAttribute();
			}
			SetConsoleTextAttribute(hConsole, colorCode);
#else
			if (enableColors)
			{
				std::cout << "\033[1;" << colorCode << "m";
			}
#endif
		}
		else
		{
#if defined(_WIN32)
			if (colorsToRestore >= 0 && hConsole)
			{
				SetConsoleTextAttribute(hConsole, colorsToRestore);
				colorsToRestore = -1;
			}
#else
			if (enableColors)
			{
				std::cout << "\033[1;0m";
			}
#endif
		}
	}

	Interface::Interface()
	{
#if defined(_WIN32)
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#else
		if (!isatty(fileno(stdout)))
		{
			// when stdout is not a terminal, prevent colors so output isn't littered by escape codes
			enableColors = false;
		}
#endif
	}

}