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
#include <unordered_map>
#include <fstream>
#include <algorithm>
/*
#include <stdexcept>
#include <fstream>
#include <cstdint>
*/
#include <zlib.h>

class Unlime
{
public:
	struct Exception
	{
		struct UnableToOpen : public std::exception
		{
			const std::string filename;

			UnableToOpen(std::string filename)
			: filename(filename)
			{
			}

			const char* what() const throw()
			{
				return ("Unable to open file: " + filename).c_str();
			}
		};

		struct UnknownFormat : public std::exception
		{
			const char* what() const throw()
			{
				return "Unknown file format!";
			}
		};

		struct CorruptedFile : public std::exception
		{
			const char* what() const throw()
			{
				return "Corrupted datafile!";
			}
		};

		struct VersionMismatch : public std::exception
		{
			const char* what() const throw()
			{
				return "Datafile version mismatch!";
			}
		};

		struct UnknownDatafile : public std::exception
		{
			const char* what() const throw()
			{
				return "Unknown datafile!";
			}
		};

		struct Decompress : public std::exception
		{
			const char* what() const throw()
			{
				return "Unable to decompress data!";
			}
		};

		struct Unknown : public std::exception
		{
			const char* what() const throw()
			{
				return "Unknown error!";
			}
		};
	};

	using T_Bytes = std::vector<Bytef>;

	struct Options
	{
		bool integrityCheck = true;
		bool checkHeadString = false;
		std::string headString;
	};

private:

	class INIParse
	{
		const std::string whitespaceDelimiters = " \t\n\r\f\v";

		enum class PDataType : char
		{
			PDATA_NONE,
			PDATA_COMMENT,
			PDATA_SECTION,
			PDATA_KEYVALUE,
			PDATA_UNKNOWN
		};

		struct PData
		{
			PDataType type;
			std::string key;
			std::string value;
		};

		static void trim(std::string& str);

		static PData parseLine(std::string line);
	};

	const std::string resourceManifestFilename;

	struct T_DictCategory
	{
		std::unordered_map<std::string, std::string> map;
		bool isMeta = false;
	};
	using T_DictMap = std::unordered_map<std::string, T_DictCategory>;

	T_DictMap dictMap;
	bool dictWasRead = false;

	void readDict()
	{
		std::ifstream resourceManifestStream(resourceManifestFilename);
		if (!resourceManifestStream.is_open())
		{
			throw Exception::UnableToOpen(resourceManifestFilename);
		}

		std::string resourceManifestContents;
		resourceManifestStream.seekg(0, resourceManifestStream.end);
		size_t fileSize = resourceManifestStream.tellg();
		resourceManifestStream.seekg(0, resourceManifestStream.beg);
		resourceManifestStream.read(&resourceManifestContents[0], fileSize);
		resourceManifestStream.close();

		std::vector<std::string> lineData;
		{
			std::string buff;
			buff.reserve(50);
			for (std::size_t i = 0; i < fileSize; i++)
			{
				char const& c = resourceManifestContents[i];
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
		}

		~Extractor()
		{
		}

		bool get(T_Bytes& data, std::string const& category, std::string const& key) const
		{
			if (!unlime->dictWasRead)
			{
				unlime->readDict();
			}
			return true;
		}
	};

	Unlime(std::string filename)
		: resourceManifestFilename(filename)
	{
	}

	Unlime(std::string filename, Options options)
		: resourceManifestFilename(filename)
	{
	}

	void dropDict()
	{
		dictMap.clear();
		dictWasRead = false;
	}
};

#endif // LIME_UNLIME_PHONY_H_