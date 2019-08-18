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
  *  Unlime
  *
  *  unlime.h
  *  Class for Lime datafile extraction.
  *
  */

#pragma once

#ifndef LIME_UNLIME_H_
#define LIME_UNLIME_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <cstdio>
#include <zlib.h>

class Unlime
{
	/*
public:
	class Extractor;
*/
private:
	struct T_DictItem
	{
		uint64_t seek_id;
		uint64_t size;
		uint32_t checksum;
	};
	using T_DictCategory = std::unordered_map<std::string, T_DictItem>;
	using T_DictMap = std::unordered_map<std::string, T_DictCategory>;

/*
	using T_ExtractorList = std::vector<Extractor*>;
	T_ExtractorList extractorList;*/

	const std::string dataFilename;

	T_DictMap dictMap;
	bool dictWasRead = false;

	FILE* dataFileHandle = nullptr;
	bool dataFileOpen = false;

	size_t n_extractors = 0;

	void openDatafile()
	{
		if (dataFileOpen)
		{
			return;
		}
#if defined(_WIN32)
		if (fopen_s(&dataFileHandle, dataFilename.c_str(), "rb") != 0)
#else
		if ((dataFileHandle = fopen(dataFilename.c_str(), "rb")) == nullptr)
#endif
		{
			throw std::runtime_error("Unable to open file: " + dataFilename);
		}
		dataFileOpen = true;
	}

	void closeDatafile()
	{
		if (dataFileOpen)
		{
			fclose(dataFileHandle);
			dataFileHandle = nullptr;
			dataFileOpen = false;
		}
	}

/*
	void destroyExtractors()
	{
		for (Extractor* ex : extractorList)
		{
			delete ex;
		}
		extractorList.clear();
	}
	*/

	void readDict()
	{
		if (dictWasRead)
		{
			return;
		}

/*
#if defined(_WIN32)
				while ((numRead = fread_s(buffer, 16348u, 1, sizeof(buffer), cDataFile)) > 0)
#else
				while ((numRead = fread(buffer, 1, 16348u, cDataFile)) > 0)
#endif
*/

	//	fclose(dataFileHandle);
	//	dataFileHandle = nullptr;
	}

public:
	using T_Bytes = std::vector<Bytef>;

	struct Options
	{
		bool integrityCheck = true;
		bool checkHeadString = true;
		std::string headString;
	} options;

	class Extractor
	{
	private:
		Unlime* unlime = nullptr;

	public:
		Extractor(Unlime& unlime)
		: unlime(&unlime)
		{
			if (this->unlime->n_extractors++ == 0)
			{
				this->unlime->openDatafile();
			}
		}

		/*
		Extractor(Extractor const& other)
		: unlime(other.unlime)
		{
		}
		*/

		const Extractor operator=(Extractor const& other)
		{
			if (this != &other)
			{
				unlime = other.unlime;
			}
			return *this;
		}

		~Extractor()
		{
			if (--unlime->n_extractors == 0)
			{
				unlime->closeDatafile();
			}
		}

		/*
		Extractor(Extractor&& other)
		{
			unlime = other.unlime;
			other.unlime = nullptr;
		}

		const Extractor& operator=(Extractor&& other)
		{
			if (this != &other)
			{
				unlime = other.unlime;
				other.unlime = nullptr;
			}
			return *this;
		}
		*/

		Unlime::T_Bytes get(std::string const& category, std::string const& key) const
		{
			if (!unlime->dictWasRead)
			{
				unlime->readDict();
			}
			return T_Bytes();
		}
	};

	Unlime(std::string const& filename)
	: dataFilename(filename)
	{
	}
};

#endif // LIME_UNLIME_H_