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
///#include <cstdio>
#include <fstream>
#include <cstdint>
#include <sys/stat.h>
#include <zlib.h>

class Unlime
{
public:
	using T_Bytes = std::vector<Bytef>;

private:
	const std::string LIME_VERSION = "0.9.0";

	const std::string LM_BGN_ADLER32 = "LM>";
	const std::string LM_END_ADLER32 = "<LM";
	const std::string LM_BGN_CRC32 = "LM]";
	const std::string LM_END_CRC32 = "[LM";
	const std::string LM_BGN_NOCHKSUM = "LM)";
	const std::string LM_END_NOCHKSUM = "(LM";

	const int LM_ENDPOINT_LENGTH = 3;

	const std::string ERROR_UNKNOWN_ERROR = "Unknown error!";
	const std::string ERROR_UNKNOWN_FORMAT = "Unknown file format!";
	const std::string ERROR_VERSION_MISMATCH = "Datafile version mismatch!";
	const std::string ERROR_UNKNOWN_DATAFILE = "Unknown datafile!";
	const std::string ERROR_CORRUPTED_FILE = "Corrupted datafile format.";

	enum class DatafileChecksumFunc : unsigned char
	{
		ADLER32, CRC32, NONE
	};

	const uint64_t minimumDatafileSize = 10;

	struct T_DictItem
	{
		uint64_t seek_id;
		uint64_t size;
		uint32_t checksum;
	};
	using T_DictCategory = std::unordered_map<std::string, T_DictItem>;
	using T_DictMap = std::unordered_map<std::string, T_DictCategory>;

	const std::string datafileFilename;

	T_DictMap dictMap;
	bool dictWasRead = false;

	uint64_t totalDatafileSize = 0;

	DatafileChecksumFunc chksumFunc = DatafileChecksumFunc::ADLER32;

	uint32_t dictSize = 0;
	uint32_t dictOffset = 0;
	uint32_t dictChecksum = 0;

	uint64_t dataSize = 0;
	uint64_t dataOffset = 0;

	bool wasValidated = false;

	std::ifstream datafileStream;

	size_t n_extractors = 0;

	bool fileExists(const char* filename)
	{
		struct stat buff;
		return !stat(filename, &buff);
	}

	size_t fileSize(const char* filename)
	{
		struct stat buff;
		stat(filename, &buff);
		return buff.st_size;
	}

	void openDatafile()
	{
		if (datafileStream.is_open())
		{
			return;
		}
		if (!fileExists(datafileFilename.c_str()))
		{
			throw std::runtime_error("Unable to find file: " + datafileFilename);
		}
		datafileStream.open(datafileFilename, std::ios::in | std::ifstream::binary);
		if (!datafileStream.is_open())
		{
			throw std::runtime_error("Unable to open file: " + datafileFilename);
		}
	}

	void closeDatafile()
	{
		if (datafileStream.is_open())
		{
			datafileStream.close();
		}
	}

	template<class T>
	void readValue(T& value)
	{
		value = 0;
		size_t n_bytes = sizeof(T);
		for (size_t i = 0; i < n_bytes; ++i)
		{
			Bytef buffer;
			datafileStream.read((char*)&buffer, 1);
			value = (value << 8) + buffer;
		}
	}

	template<class T>
	void readBytes(T& buffer, size_t size)
	{
		buffer.resize(size);
		datafileStream.read((char*)&buffer[0], size);
	}

	void validateFormat()
	{
		if (!datafileStream.is_open())
		{
			throw std::runtime_error(ERROR_UNKNOWN_ERROR);
		}

		// filesize sanity check
		totalDatafileSize = fileSize(datafileFilename.c_str());
		if (totalDatafileSize < minimumDatafileSize)
		{
			throw std::runtime_error(ERROR_UNKNOWN_FORMAT);
		}

		// retreive bgn and end endpoints
		datafileStream.seekg(0);
		std::string bgnEndpointStr;
		readBytes(bgnEndpointStr, LM_ENDPOINT_LENGTH);

		datafileStream.seekg(-LM_ENDPOINT_LENGTH, datafileStream.end);
		std::string endEndpointStr;
		readBytes(endEndpointStr, LM_ENDPOINT_LENGTH);

		// validate endpoints and extract checksum function used
		if (bgnEndpointStr == LM_BGN_ADLER32 && endEndpointStr == LM_END_ADLER32)
		{
			chksumFunc = DatafileChecksumFunc::ADLER32;
		}
		else if (bgnEndpointStr == LM_BGN_CRC32 && endEndpointStr == LM_END_CRC32)
		{
			chksumFunc = DatafileChecksumFunc::CRC32;
		}
		else if (bgnEndpointStr == LM_BGN_NOCHKSUM && endEndpointStr == LM_END_NOCHKSUM)
		{
			chksumFunc = DatafileChecksumFunc::NONE;
		}
		else
		{
			throw std::runtime_error(ERROR_UNKNOWN_FORMAT);
		}

		// retreive version string
		datafileStream.seekg(LM_ENDPOINT_LENGTH);
		uint8_t versionStrLength = 0;
		readValue(versionStrLength);

		std::string versionStr;
		readBytes(versionStr, versionStrLength);

		if (versionStr != LIME_VERSION)
		{
			// for now we assume that any version other than current version is unreadable
			// future versions may allow backwards compatibility through datafile versioning
			// in case the format changes
			throw std::runtime_error(ERROR_VERSION_MISMATCH);
		}

		uint8_t headStrLength = 0;
		readValue(headStrLength);

		if (options.checkHeadString)
		{
			std::string headStr;
			readBytes(headStr, headStrLength);

			if (headStr != options.headString)
			{
				throw std::runtime_error(ERROR_UNKNOWN_DATAFILE);
			}
		}
		else if (headStrLength > 0)
		{
			datafileStream.seekg(headStrLength, datafileStream.cur);
		}

		// extract header data
		readValue(dictSize);
		if (chksumFunc != DatafileChecksumFunc::NONE)
		{
			readValue(dictChecksum);
		}
		readValue(dataSize);

		// calculate offsets
		dictOffset = static_cast<uint32_t>(datafileStream.tellg());
		dataOffset = static_cast<uint64_t>(dictSize) + static_cast<uint64_t>(dictOffset);

		// validation done
		wasValidated = true;
	}

	void readDict()
	{
		if (!datafileStream.is_open() || !wasValidated)
		{
			throw std::runtime_error(ERROR_UNKNOWN_ERROR);
		}

		/*
		datafileStream.seekg(dictOffset);
		T_Bytes dictBytesCompressed;
		readBytes(dictBytesCompressed, dictSize);

		T_Bytes dictBytesDecompressed;
		uLongf destLen = 100000u;
		dictBytesDecompressed.resize(destLen);
		uncompress(&dictBytesDecompressed[0], &destLen, dictBytesCompressed.data(), dictBytesCompressed.size());
		*/
	}

public:
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
		Extractor(Unlime& context)
		: unlime(&context)
		{
			if (unlime->n_extractors++ == 0)
			{
				unlime->openDatafile();
			}
		}

		/*
		Extractor(Extractor const& other)
		: unlime(other.unlime)
		{
		}
		*/

		/*
		const Extractor operator=(Extractor const& other)
		{
			if (this != &other)
			{
				unlime = other.unlime;
			}
			return *this;
		}*/

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
			if (!unlime->wasValidated)
			{
				unlime->validateFormat();
			}
			if (!unlime->dictWasRead)
			{
				unlime->readDict();
			}
			return T_Bytes();
		}
	};

	Unlime(std::string const& filename)
	: datafileFilename(filename)
	{
	}
};

#endif // LIME_UNLIME_H_