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
#include <algorithm>
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
	const std::string ERROR_DECOMPRESS = "Unable to decompress data.";

	enum class DatafileChecksumFunc : unsigned char
	{
		ADLER32, CRC32, NONE
	};

	const uint64_t minimumDatafileSize = 10;

	const std::string datafileFilename;

	struct T_DictItem
	{
		uint64_t seek_id = 0;
		uint64_t size = 0;
		uint32_t checksum = 0;
	};
	using T_DictCategory = std::unordered_map<std::string, T_DictItem>;
	using T_DictMap = std::unordered_map<std::string, T_DictCategory>;

	T_DictMap dictMap;
	bool dictWasRead = false;

	uint64_t totalDatafileSize = 0;

	DatafileChecksumFunc chksumFunc = DatafileChecksumFunc::ADLER32;

	uint32_t dictSize = 0;
	uint32_t dictOffset = 0;
	uint32_t dictChecksum = 0;
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
	void readValueFromStream(T& value)
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
	void readValueFromBytes(T& value, T_Bytes const& bytes, size_t& at)
	{
		value = 0;
		size_t n_bytes = sizeof(T);
		for (size_t i = 0; i < n_bytes; ++i)
		{
			value = (value << 8) + bytes[at + i];
		}
		at += n_bytes;
	}

	template<class T>
	void readBytesFromStream(T& destination, size_t size)
	{
		destination.resize(size);
		datafileStream.read((char*)&destination[0], size);
	}

	void readStringFromBytes(std::string& destination, size_t size, T_Bytes const& buffer, size_t& at)
	{
		destination.resize(size);
		T_Bytes::const_iterator begin = buffer.begin() + at;
		T_Bytes::const_iterator end = buffer.begin() + at + size;
		destination.assign(begin, end);
		at += size;
	}

	void readCompressedStream(T_Bytes& destination, size_t size, uint32_t knownChecksum = 0)
	{
		static const size_t inBuffSize = 16348u;
		static const size_t outBuffSize = 16348u;

		z_stream dcmpStream;
		dcmpStream.zalloc = Z_NULL;
		dcmpStream.zfree = Z_NULL;
		dcmpStream.opaque = Z_NULL;
		dcmpStream.avail_in = 0u;
		dcmpStream.next_in = Z_NULL;
		if (inflateInit(&dcmpStream) != Z_OK)
		{
			throw std::runtime_error(ERROR_DECOMPRESS);
		}

		T_Bytes inputBuffer;
		inputBuffer.reserve(inBuffSize);
		size_t totalBytesRead = 0;
		size_t remainingBytesToRead = size;
	
		Bytef* outputBuffer = new Bytef[outBuffSize];

		do {
			size_t bytesToRead = std::min(inBuffSize, remainingBytesToRead);
			readBytesFromStream(inputBuffer, bytesToRead);
			remainingBytesToRead -= bytesToRead;

			dcmpStream.next_in = &inputBuffer[0];
			dcmpStream.avail_in = static_cast<unsigned int>(bytesToRead);

			do {
				dcmpStream.next_out = &outputBuffer[0];
				dcmpStream.avail_out = outBuffSize;

				int streamState = inflate(&dcmpStream, Z_NO_FLUSH);

				if (streamState != Z_OK && streamState != Z_STREAM_END)
				{
					delete[] outputBuffer;
					throw std::runtime_error(ERROR_DECOMPRESS);
				}

				destination.insert(destination.end(), outputBuffer, outputBuffer + (outBuffSize - dcmpStream.avail_out));

			} while (dcmpStream.avail_out == 0);

		} while (remainingBytesToRead != 0);

		delete[] outputBuffer;

		if (inflateEnd(&dcmpStream) != Z_OK)
		{
			throw std::runtime_error(ERROR_DECOMPRESS);
		}
		
		if (options.integrityCheck)
		{
			uint32_t checksum = 0;
			switch (chksumFunc)
			{
				case DatafileChecksumFunc::ADLER32:
					checksum = adler32(0ul, destination.data(), static_cast<unsigned int>(destination.size()));
					break;
				case DatafileChecksumFunc::CRC32:
					checksum = crc32(0ul, destination.data(), static_cast<unsigned int>(destination.size()));
					break;
			}
			if (checksum != knownChecksum)
			{
				throw std::runtime_error(ERROR_CORRUPTED_FILE);
			}
		}
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
		readBytesFromStream(bgnEndpointStr, LM_ENDPOINT_LENGTH);

		datafileStream.seekg(-LM_ENDPOINT_LENGTH, datafileStream.end);
		std::string endEndpointStr;
		readBytesFromStream(endEndpointStr, LM_ENDPOINT_LENGTH);

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
		readValueFromStream(versionStrLength);

		std::string versionStr;
		readBytesFromStream(versionStr, versionStrLength);

		if (versionStr != LIME_VERSION)
		{
			// for now we assume that any version other than current version is unreadable
			// future versions may allow backwards compatibility through datafile versioning
			// in case the format changes
			throw std::runtime_error(ERROR_VERSION_MISMATCH);
		}

		uint8_t headStrLength = 0;
		readValueFromStream(headStrLength);

		if (options.checkHeadString)
		{
			std::string headStr;
			readBytesFromStream(headStr, headStrLength);

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
		readValueFromStream(dictSize);
		if (chksumFunc != DatafileChecksumFunc::NONE)
		{
			readValueFromStream(dictChecksum);
		}

		// calculate offsets
		dictOffset = static_cast<uint32_t>(datafileStream.tellg());
		dataOffset = static_cast<uint64_t>(dictSize) + static_cast<uint64_t>(dictOffset);

		// validation complete
		wasValidated = true;
	}

	void readDict()
	{
		if (!datafileStream.is_open() || !wasValidated)
		{
			throw std::runtime_error(ERROR_UNKNOWN_ERROR);
		}

		dictMap.clear();

		datafileStream.seekg(dictOffset);
		T_Bytes dictBytes;
		readCompressedStream(dictBytes, dictSize, dictChecksum);

		size_t readAt = 0;

		uint32_t n_categories = 0;
		readValueFromBytes(n_categories, dictBytes, readAt);

		for (size_t i = 0; i < n_categories; ++i)
		{
			uint8_t categoryKeyLength = 0;
			readValueFromBytes(categoryKeyLength, dictBytes, readAt);

			std::string categoryKey;
			readStringFromBytes(categoryKey, categoryKeyLength, dictBytes, readAt);

			uint32_t n_dataNodes;
			readValueFromBytes(n_dataNodes, dictBytes, readAt);

			for (size_t j = 0; j < n_dataNodes; ++j)
			{
				uint8_t dataKeyLength = 0;
				readValueFromBytes(dataKeyLength, dictBytes, readAt);

				std::string dataKey;
				readStringFromBytes(dataKey, dataKeyLength, dictBytes, readAt);

				T_DictItem dictItem;
				readValueFromBytes(dictItem.seek_id, dictBytes, readAt);
				readValueFromBytes(dictItem.size, dictBytes, readAt);
				if (chksumFunc != DatafileChecksumFunc::NONE)
				{
					readValueFromBytes(dictItem.checksum, dictBytes, readAt);
				}

				dictMap[categoryKey][dataKey] = dictItem;
			}
		}
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
			T_Bytes data;
			
			T_DictItem const& dictItem = unlime->dictMap[category][key];
			unlime->datafileStream.seekg(unlime->dataOffset + dictItem.seek_id);
			unlime->readCompressedStream(data, static_cast<size_t>(dictItem.size), dictItem.checksum);
			
			return data;
		}
	};

	Unlime(std::string const& filename)
	: datafileFilename(filename)
	{
	}
};

#endif // LIME_UNLIME_H_