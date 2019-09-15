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
  *  unlime_phony.h
  *  Class for Lime resource manifest data extraction.
  *
  *  This header replicates the API of unlime.h and is meant for use during development
  *  to avoid packing data between changes to the manifest or resources. Use unlime.h
  *  for deployment.
  *
  */

#pragma once

#ifndef LIME_UNLIME_PHONY_H_
#define LIME_UNLIME_PHONY_H_

#define LIME_PHONY

#include <string>
#include <vector>
/*
#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <cstdint>
*/
#include <zlib.h>

class Unlime
{
public:
	using T_Bytes = std::vector<Bytef>;

	struct Options
	{
		bool integrityCheck = true;
		bool checkHeadString = false;
		std::string headString;
	};

private:
	/*
	const std::string ERROR_UNKNOWN_ERROR = "Unknown error!";
	const std::string ERROR_UNKNOWN_FORMAT = "Unknown file format!";
	const std::string ERROR_VERSION_MISMATCH = "Datafile version mismatch!";
	const std::string ERROR_UNKNOWN_DATAFILE = "Unknown datafile!";
	const std::string ERROR_CORRUPTED_FILE = "Corrupted datafile format.";
	const std::string ERROR_DECOMPRESS = "Unable to decompress data.";
	*/

	const std::string datafileFilename;

	size_t n_extractors = 0;

	void openDatafile()
	{

	}

	void closeDatafile()
	{

	}

	Unlime(Unlime const&) = delete;
	Unlime& operator=(Unlime const&) = delete;
	Unlime(Unlime&& other) = delete;
	const Unlime& operator=(Unlime&& other) = delete;

public:
	class Extractor
	{
	private:
		Unlime* unlime = nullptr;

		Extractor(Extractor const&) = delete;
		Extractor& operator=(Extractor const&) = delete;
		Extractor(Extractor&& other) = delete;
		const Extractor& operator=(Extractor&& other) = delete;

	public:
		Extractor(Unlime& context)
			: unlime(&context)
		{
			if (unlime->n_extractors++ == 0)
			{
				unlime->openDatafile();
			}
		}

		~Extractor()
		{
			if (--unlime->n_extractors == 0)
			{
				unlime->closeDatafile();
			}
		}

		bool get(T_Bytes& data, std::string const& category, std::string const& key) const
		{
			return true;
		}
	};

	Unlime(std::string filename)
		: datafileFilename(filename)
	{
	}

	Unlime(std::string filename, Options options)
		: datafileFilename(filename)
	{
	}

	void dropDict()
	{
		/*
		dictMap.clear();
		*/
	}
};

#endif // LIME_UNLIME_PHONY_H_