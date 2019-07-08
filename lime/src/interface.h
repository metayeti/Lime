/*
 *  Lime
 *
 *  Utility for Lime datafile creation.
 *
 *  interface.h
 *  Defines the class for console output.
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

#pragma once

#ifndef LIME_INTERFACE_H_
#define LIME_INTERFACE_H_

#if defined(_WIN32)
	#include <Windows.h>
#endif

namespace Lime
{
	class Interface
	{
	private:
#if defined(_WIN32)
		HANDLE hConsole = nullptr;
		int colorsToRestore = -1;

		WORD GetConsoleTextAttribute();
#endif

	public:
#if defined(_WIN32)
		enum class Color : int
		{
			BLACK = 0,
			BLUE = 1,
			GREEN = 2,
			CYAN = 3,
			RED = 4,
			PURPLE = 5,
			YELLOW = 6,
			WHITE = 7,
			GRAY = 8,
			BRIGHTBLUE = 9,
			BRIGHTGREEN = 10,
			BRIGHTCYAN = 11,
			BRIGHTRED = 12,
			BRIGHTPURPLE = 13,
			BRIGHTYELLOW = 14,
			BRIGHTWHITE = 15
		};
#else
		enum class Color : int
		{
			BLACK = 30,
			BLUE = 34,
			GREEN = 32,
			CYAN = 36,
			RED = 31,
			PURPLE = 35,
			YELLOW = 33,
			WHITE = 37,
			GRAY = 90,
			BRIGHTBLUE = 94,
			BRIGHTGREEN = 92,
			BRIGHTCYAN = 96,
			BRIGHTRED = 91,
			BRIGHTPURPLE = 94,
			BRIGHTYELLOW = 93,
			BRIGHTWHITE = 97
		};
#endif

		void setConsoleColor(Color color);
		void restoreConsoleColor();

		void print(const char* format, ...);
	};

}

#endif // LIME_INTERFACE_H_