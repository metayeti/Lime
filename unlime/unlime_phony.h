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

#define UNLIME_PHONY

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <algorithm>
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
		};

		struct CorruptedFile : public std::exception
		{
		};

		struct VersionMismatch : public std::exception
		{
		};

		struct UnknownDatafile : public std::exception
		{
		};

		struct Decompress : public std::exception
		{
		};

		struct Unknown : public std::exception
		{
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

	struct INIParse
	{
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

		static void trim(std::string& str)
		{
			static const std::string whitespaceDelimiters = " \t\n\r\f\v";
			str.erase(str.find_last_not_of(whitespaceDelimiters) + 1);
			str.erase(0, str.find_first_not_of(whitespaceDelimiters));
		}

		static PData parseLine(std::string line)
		{
			trim(line);
			if (line.empty())
			{
				return { PDataType::PDATA_NONE };
			}
			const char firstCharacter = line.at(0);
			if (firstCharacter == ';')
			{
				return { PDataType::PDATA_COMMENT };
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
					return { PDataType::PDATA_SECTION, section };
				}
			}
			const size_t equalsAt = line.find_first_of('=');
			if (equalsAt != std::string::npos)
			{
				std::string key = line.substr(0, equalsAt);
				trim(key);
				std::string value = line.substr(equalsAt + 1);
				trim(value);
				return { PDataType::PDATA_KEYVALUE, key, value };
			}

			return { PDataType::PDATA_UNKNOWN };
		}
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

	std::string resourceDirectory;

	void readDict()
	{
		std::ifstream resourceManifestStream(resourceManifestFilename, std::ios::in | std::ifstream::binary);
		if (!resourceManifestStream.is_open())
		{
			throw Exception::UnableToOpen(resourceManifestFilename);
		}

		std::string resourceManifestContents;
		resourceManifestStream.seekg(0, resourceManifestStream.end);
		size_t fileSize = resourceManifestStream.tellg();
		resourceManifestStream.seekg(0, resourceManifestStream.beg);
		resourceManifestContents.resize(fileSize);
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

		bool inSection = false;
		bool sectionIsMeta = false;
		T_DictCategory* currentDictCategory = nullptr;
		std::string category;
		for (auto const& line : lineData)
		{
			auto [ptype, key, value] = INIParse::parseLine(line);
			if (ptype == INIParse::PDataType::PDATA_SECTION)
			{
				inSection = true;
				category = key;
				sectionIsMeta = category[0] == '@';
				if (sectionIsMeta)
				{
					category = category.substr(1);
				}
				currentDictCategory = &dictMap[category];
				currentDictCategory->isMeta = sectionIsMeta;
			}
			else if (inSection && ptype == INIParse::PDataType::PDATA_KEYVALUE)
			{
				// normalize filename path separators to apply to current system
				// assumes windows uses \ and everything else uses /
				if (category.size() && !currentDictCategory->isMeta) // skip meta categories
				{
#if defined(_WIN32)
					std::replace(value.begin(), value.end(), '/', '\\');
#else
					std::replace(value.begin(), value.end(), '\\', '/');
#endif
				}
				currentDictCategory->map[key] = value;
			}
		}

		const size_t lastSlashInPath = resourceManifestFilename.find_last_of("\\/");
		if (lastSlashInPath == std::string::npos)
		{
			resourceDirectory = "";
		}
		else
		{
			resourceDirectory = resourceManifestFilename.substr(0, lastSlashInPath);

#if defined(_WIN32)
			std::replace(resourceDirectory.begin(), resourceDirectory.end(), '/', '\\');
			resourceDirectory += '\\';
#else
			std::replace(resourceDirectory.begin(), resourceDirectory.end(), '\\', '/');
			resourceDirectory += '/';
#endif
		}

		// done reading dict
		dictWasRead = true;
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
			auto it = unlime->dictMap.find(category);
			if (it == unlime->dictMap.end())
			{
				return false;
			}
			auto& collection = it->second;
			auto it2 = collection.map.find(key);
			if (it2 == collection.map.end())
			{
				return false;
			}
			std::string& value = it2->second;
			if (collection.isMeta)
			{
				data.assign(value.begin(), value.end());
			}
			else
			{
				std::string const resourceFilename = unlime->resourceDirectory + value;
				std::ifstream resourceStream(resourceFilename, std::ios::in | std::ifstream::binary);
				if (!resourceStream.is_open())
				{
					throw Exception::UnableToOpen(resourceFilename);
				}

				static const size_t inBuffSize = 512u;
				Bytef* inputBuffer = new Bytef[inBuffSize];

				resourceStream.seekg(0, resourceStream.end);
				size_t resourceSize = resourceStream.tellg();
				resourceStream.seekg(0, resourceStream.beg);

				data.clear();
				data.reserve(resourceSize);

				bool isEof = false;
				size_t numRead = 0;

				do {

					resourceStream.read(reinterpret_cast<char*>(inputBuffer), inBuffSize);
					numRead = resourceStream.gcount();

					std::string blah(reinterpret_cast<const char*>(inputBuffer), inBuffSize);

					data.insert(data.end(), inputBuffer, inputBuffer + numRead);

				} while (!resourceStream.eof());

				delete[] inputBuffer;

				resourceStream.close();
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
		resourceDirectory = "";
	}
};

#endif // LIME_UNLIME_PHONY_H_