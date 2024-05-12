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
  *  dict.cpp
  *  Implements the createDict function used for reading the resource manifest file.
  *
  */

#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>
#include "dict.h"
#include "iniparse.h"

namespace Lime
{
	Dict readDictFromFile(std::string const& resourceManifestFilename)
	{
		Dict outDict;
		// read file contents
		std::string fileContents;
		std::ifstream fin;
		fin.open(resourceManifestFilename, std::ios::in | std::ios::binary);
		if (!fin.is_open())
		{
			// can't open file
			throw std::runtime_error("Could not open \"" + resourceManifestFilename + "\" for reading.");
		}
		fin.seekg(0, std::ios::end);
		size_t fileSize = static_cast<size_t>(fin.tellg());
		fileContents.resize(fileSize);
		fin.seekg(0, std::ios::beg);
		fin.read(&fileContents[0], fileSize);
		fin.close();
		// translate content to lines
		std::vector<std::string> lineData;
		{
			std::string buff;
			buff.reserve(50);
			for (std::size_t i = 0; i < fileSize; i++)
			{
				char const& c = fileContents[i];
				if (c == '\n')
				{
					lineData.push_back(buff);
					buff.clear();
					continue;
				}
				if (c != '\0' && c != '\r')
				{
					buff += c;
				}
			}
			lineData.push_back(buff);
		}
		// parse INI lines and translate to dict
		bool inSection = false;
		std::string section;
		for (auto const& line : lineData)
		{
			auto [ ptype, key, value ] = INIParse::parseLine(line);
			if (ptype == INIParse::PDataType::PDATA_SECTION)
			{
				inSection = true;
				outDict[section = key];
			}
			else if (inSection && ptype == INIParse::PDataType::PDATA_KEYVALUE)
			{
				// normalize filename path separators to apply to current system
				// assumes windows uses \ and everything else uses /
				if (!section.size() || section[0] != '@') // skip meta sections
				{
#if defined(_WIN32)
					std::replace(value.begin(), value.end(), '/', '\\');
#else
					std::replace(value.begin(), value.end(), '\\', '/');
#endif
				}
				outDict[section][key] = value;
			}
		}
		
		// all done
		return outDict;
	}
}