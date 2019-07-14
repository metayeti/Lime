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
  *  iniparse.cpp
  *  Implements the INI parser.
  *
  */

#include "iniparse.h"
#include <algorithm>

namespace Lime
{
	void INIParse::trim(std::string& str)
	{
		str.erase(str.find_last_not_of(whitespaceDelimiters) + 1);
		str.erase(0, str.find_first_not_of(whitespaceDelimiters));
	}

	INIParse::PDataType INIParse::parseLine(std::string line, T_ParseValues& parseData)
	{
		parseData.first.clear();
		parseData.second.clear();

		trim(line);
		if (line.empty())
		{
			return PDataType::PDATA_NONE;
		}
		const char firstCharacter = line.at(0);
		if (firstCharacter == ';')
		{
			return PDataType::PDATA_COMMENT;
		}
		if (firstCharacter == '[')
		{
			const size_t commentAt = line.find_first_of(';');
			if (commentAt != std::string::npos)
			{
				line = line.substr(0, commentAt);
			}
			const size_t closingBracketAt = line.find_last_of(']');
			if (closingBracketAt != std::string::npos)
			{
				std::string section = line.substr(1, closingBracketAt - 1);
				trim(section);
				parseData.first = section;
				return PDataType::PDATA_SECTION;
			}
		}
		const size_t equalsAt = line.find_first_of('=');
		if (equalsAt != std::string::npos)
		{
			std::string key = line.substr(0, equalsAt);
			trim(key);
			std::string value = line.substr(equalsAt + 1);
			trim(value);
			parseData = { key, value };
			return PDataType::PDATA_KEYVALUE;
		}

		return PDataType::PDATA_UNKNOWN;
	}
}