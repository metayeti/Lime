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

#undef UNLIME_PHONY

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <exception>
#include <fstream>
#include <cstdint>
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
	const uint8_t LIME_REVISION = 1;

	const std::string LM_BGN_ADLER32 = "L>";
	const std::string LM_END_ADLER32 = "<M";
	const std::string LM_BGN_CRC32 = "L]";
	const std::string LM_END_CRC32 = "[M";
	const std::string LM_BGN_NOCHKSUM = "L)";
	const std::string LM_END_NOCHKSUM = "(M";

	const int LM_ENDPOINT_LENGTH = 2;

	enum class DatafileChecksumFunc : unsigned char
	{
		ADLER32, CRC32, NONE
	};

	const uint64_t minimumDatafileSize = 36u;

	const std::string datafileFilename;

	Options options;

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
	uint32_t dictChecksum = 0;
	uint64_t dictOffset = 0;

	bool wasValidated = false;

	std::ifstream datafileStream;

	size_t n_extractors = 0;

	void openDatafile()
	{
		if (datafileStream.is_open())
		{
			return;
		}
		datafileStream.open(datafileFilename, std::ios::in | std::ifstream::binary);
		if (!datafileStream.is_open())
		{
			throw Exception::UnableToOpen(datafileFilename);
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
		if (size == 0)
		{
			throw Exception::Decompress();
		}

		static const size_t inBuffSize = 500u;
		static const size_t outBuffSize = 16348u;

		z_stream dcmpStream;
		dcmpStream.zalloc = Z_NULL;
		dcmpStream.zfree = Z_NULL;
		dcmpStream.opaque = Z_NULL;
		dcmpStream.avail_in = 0u;
		dcmpStream.next_in = Z_NULL;
		if (inflateInit(&dcmpStream) != Z_OK)
		{
			throw Exception::Decompress();
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
					throw Exception::Decompress();
				}

				destination.insert(destination.end(), outputBuffer, outputBuffer + (outBuffSize - dcmpStream.avail_out));

			} while (dcmpStream.avail_out == 0);

		} while (remainingBytesToRead != 0);

		delete[] outputBuffer;

		if (inflateEnd(&dcmpStream) != Z_OK)
		{
			throw Exception::Decompress();
		}
		
		if (options.integrityCheck)
		{
			uint32_t checksum = 0;
			switch (chksumFunc)
			{
				case DatafileChecksumFunc::ADLER32:
					checksum = adler32_z(0ul, destination.data(), destination.size());
					break;
				case DatafileChecksumFunc::CRC32:
					checksum = crc32_z(0ul, destination.data(), destination.size());
					break;
			}
			if (checksum != knownChecksum)
			{
				throw Exception::CorruptedFile();
			}
		}
	}

	void validateAndExtractHeader()
	{
		if (!datafileStream.is_open())
		{
			throw Exception::Unknown();
		}

		// filesize sanity check
		datafileStream.seekg(0, datafileStream.end);
		totalDatafileSize = datafileStream.tellg();
		if (totalDatafileSize < minimumDatafileSize)
		{
			throw Exception::UnknownFormat();
		}

		// retreive bgn and end endpoints
		datafileStream.seekg(0, datafileStream.beg);
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
			throw Exception::UnknownFormat();
		}

		// retreive revision number
		datafileStream.seekg(LM_ENDPOINT_LENGTH);
		uint8_t limeRevision = 0;
		readValueFromStream(limeRevision);

		if (limeRevision != LIME_REVISION)
		{
			// for now we assume that any version other than current version is unreadable;
			// future versions may allow backwards format compatibility
			throw Exception::VersionMismatch();
		}

		uint8_t headStrLength = 0;
		readValueFromStream(headStrLength);

		if (options.checkHeadString)
		{
			std::string headStr;
			readBytesFromStream(headStr, headStrLength);

			if (headStr != options.headString)
			{
				throw Exception::UnknownDatafile();
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
		dictOffset = totalDatafileSize - dictSize - LM_ENDPOINT_LENGTH;

		// validation complete
		wasValidated = true;
	}

	void readDict()
	{
		if (!datafileStream.is_open() || !wasValidated)
		{
			throw Exception::Unknown();
		}

		dictMap.clear();

		datafileStream.seekg(dictOffset, datafileStream.beg);

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
			if (!unlime->wasValidated)
			{
				unlime->validateAndExtractHeader();
			}
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
			auto it2 = collection.find(key);
			if (it2 == collection.end())
			{
				return false;
			}
			T_DictItem const& dictItem = it2->second;
			unlime->datafileStream.seekg(dictItem.seek_id);
			unlime->readCompressedStream(data, static_cast<size_t>(dictItem.size), dictItem.checksum);
			return true;
		}
	};

	Unlime(std::string filename)
	: datafileFilename(filename)
	{
	}

	Unlime(std::string filename, Options options)
	: datafileFilename(filename), options(options)
	{
	}

	void dropDict()
	{
		dictMap.clear();
		wasValidated = false;
		dictWasRead = false;
	}
};

#endif // LIME_UNLIME_H_