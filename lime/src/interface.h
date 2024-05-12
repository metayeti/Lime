/*
 * Copyright (c) 2024 Danijel Durakovic
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
  *  interface.h
  *  Defines the class for console output.
  *
  */

#pragma once

#ifndef LIME_INTERFACE_H_
#define LIME_INTERFACE_H_

#if defined(_WIN32)
	#include <Windows.h>
#endif
#include <string>
#include <iostream>

namespace Lime
{
	class Interface
	{
	public:
#if defined(_WIN32)
		enum class Color : int
		{
			BLACK = 0, BLUE = 1, GREEN = 2, CYAN = 3,
			RED = 4, PURPLE = 5, YELLOW = 6, WHITE = 7,
			GRAY = 8, BRIGHTBLUE = 9, BRIGHTGREEN = 10, BRIGHTCYAN = 11,
			BRIGHTRED = 12, BRIGHTPURPLE = 13, BRIGHTYELLOW = 14, BRIGHTWHITE = 15,
			DEFAULT = 16
		};
#else
		enum class Color : int
		{
			BLACK = 30, BLUE = 34, GREEN = 32, CYAN = 36,
			RED = 31, PURPLE = 35, YELLOW = 33, WHITE = 37,
			GRAY = 90, BRIGHTBLUE = 94, BRIGHTGREEN = 92, BRIGHTCYAN = 96,
			BRIGHTRED = 91, BRIGHTPURPLE = 94, BRIGHTYELLOW = 93, BRIGHTWHITE = 97,
			DEFAULT = 9999
		};
#endif

	private:
#if defined(_WIN32)
		HANDLE hConsole = nullptr;
		int colorsToRestore = -1;

		WORD GetConsoleTextAttribute() const;
#else
		bool enableColors = true;
#endif
		void setOutputColor(Color color);

	public:
		Interface();

		template<typename T>
		Interface& operator<<(T output)
		{
			std::cout << output;
			return *this;
		}

		Interface& operator<<(Color color)
		{
			setOutputColor(color);
			return *this;
		}

		template<typename T>
		Interface& error(T message)
		{
			return *this
				<< "\n\n"
				<< Color::BRIGHTRED
				<< "Error: "
				<< Color::DEFAULT
				<< message;
		}
		
		Interface& ok(const std::string msg = "ok");
	};

}

#endif // LIME_INTERFACE_H_