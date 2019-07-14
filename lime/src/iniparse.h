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
  *  iniparse.h
  *  Defines the namespace for the INI parser.
  *
  */

#include <string>
#include <utility>

namespace Lime
{
	namespace INIParse
	{
		const std::string whitespaceDelimiters = " \t\n\r\f\v";

		using T_ParseValues = std::pair<std::string, std::string>;

		enum class PDataType : char
		{
			PDATA_NONE,
			PDATA_COMMENT,
			PDATA_SECTION,
			PDATA_KEYVALUE,
			PDATA_UNKNOWN
		};

		void trim(std::string& str);

		PDataType parseLine(std::string line, T_ParseValues& parseData);
	};
}

#pragma once
