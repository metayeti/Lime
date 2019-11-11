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
  *  packer.cpp
  *  Implements the class for datafile packing.
  *
  */

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include <sys/stat.h>
#include <fstream>
#include <cstdint>
#include <stdexcept>
#include <zlib.h>
#include "pack.h"
#include "dict.h"
#include "interface.h"
#include "const.h"

namespace Lime
{
	inline bool systemIsLittleEndian()
	{
		static bool first = true;
		static bool isLittleEndian = false;
		if (first) {
			first = false;
			static const uint16_t n = 1;
			return (isLittleEndian = (*((char*)& n) == 1));
		}
		return isLittleEndian;
	}

	// from http://stackoverflow.com/a/4956493/238609
	template<typename T>
	T swapEndianness(T value)
	{
		union
		{
			T value;
			unsigned char u8[sizeof(T)];
		} source, dest;

		source.value = value;

		for (size_t k = 0; k < sizeof(T); k++)
			dest.u8[k] = source.u8[sizeof(T) - k - 1];

		return dest.value;
	}

	template<typename T>
	T toBigEndian(T value)
	{
		if (systemIsLittleEndian())
		{
			return swapEndianness<T>(value);
		}
		return value;
	}

	template<typename T>
	T toLittleEndian(T value)
	{
		if (!systemIsLittleEndian())
		{
			return swapEndianness<T>(value);
		}
		return value;
	}

	using T_Bytes = std::vector<Bytef>;

	template<typename T>
	T_Bytes toBytes(T const& value)
	{
		T_Bytes bytes;
		bytes.reserve(sizeof(T));
		for (size_t i = 0; i < sizeof(T); ++i)
		{
			bytes.push_back(static_cast<Bytef>(value >> (i * 8)));
		}
		return bytes;
	}

	template<class T>
	inline void appendBytes(T_Bytes& bytesA, T const& bytesB)
	{
		bytesA.insert(bytesA.end(), bytesB.begin(), bytesB.end());
	}

	bool fileExists(const char* filename)
	{
#if defined(_WIN32)
		struct __stat64 buff;
		return !__stat64(filename, &buff);
#else
		struct stat64 buff;
		return !stat64(filename, &buff);
#endif
	}

	size_t fileSize(const char* filename)
	{
#if defined(_WIN32)
		struct __stat64 buff;
		__stat64(filename, &buff);
#else
		struct stat64 buff;
		stat64(filename, &buff);
#endif
		return buff.st_size;
	}

	void verifyFiles(Interface& inf, Dict const& dict)
	{
		inf << "Verifying files ...\n";
		std::vector<std::string> passedFilenames;
		for (auto const& it : dict)
		{
			auto const& category = it.first;
			if (category.length() && category[0] == '@')
			{
				// skip meta sections
				continue;
			}
			auto const& collection = it.second;
			for (auto const& it2 : collection)
			{
				auto const& filename = it2.second;
#if defined(_WIN32)
				auto pFilename = it2.second;
				std::transform(pFilename.begin(), pFilename.end(), pFilename.begin(), ::tolower);
#else
				auto const& pFilename = filename;
#endif
				if (std::find(passedFilenames.begin(), passedFilenames.end(), pFilename) != passedFilenames.end())
				{
					// don't check the same file twice
					continue;
				}
				inf << filename << " ... ";
				if (!fileExists(filename.c_str()))
				{
					throw std::runtime_error("Missing file: " + filename);
				}
				passedFilenames.push_back(pFilename);
				inf.ok() << "\n";
			}
		}
		const size_t fileCount = passedFilenames.size();
		inf << "\nTotal: " << std::to_string(fileCount) << " file";
		if (fileCount != 1u) {
			inf << "s";
		}
		inf << ".\n\n";
	}

	void printOptionsInfo(Interface& inf, PackOptions const& options)
	{
		inf << "Using compression level: " << std::to_string(options.clevel);
		if (options.clevel == 0) {
			inf << " (no compression)";
		}
		else if (options.clevel == 9) {
			inf << " (maximum)";
		}
		inf << "\n";
		if (options.chksum == ChkSumOption::ADLER32)
		{
			inf << "Using checksum algorithm: Adler32\n";
		}
		else if (options.chksum == ChkSumOption::CRC32)
		{
			inf << "Using checksum algorithm: CRC32\n";
		}
		else if (options.chksum == ChkSumOption::NONE)
		{
			inf << "Using no checksum algorithm.\n";
		}
		if (options.headstr.size())
		{
			inf << "Using head string: " << options.headstr << "\n";
		}
	}

	inline void capStringSizeTo255(std::string& str)
	{
		// limit string size to 255
		if (str.size() > 255u)
		{
			str = str.substr(0, 255u);
		}
	}

	void pack(Interface& inf, Dict const& dict, std::string const& outputFilename, PackOptions& options)
	{
		/*

		Lime datafile structure:

		           Z1    ...   Zn    Zdict
		          [~~~] [~~~] [~~~] [~~~~~~~~~~]       (zipped content)

		   header   user resources   dictionary   end
		 |________|________________|____________|_____|


		   Header:

		   bgn   revision-  head*  dict size   dict checksum
		 |_____|__________|______|___________|...............|


		   Dictionary:

		   N   category 1   ...   category N
		 |___|____________|     |____________|
		            |
		            |
		            |
		         Category:

		         category key*  M   data 1   ...   data M
		       |______________|___|________|     |________|
		                              |
		                              |
		                              |
		                            Data:

		                            data key*  seek_id+  size+  checksum
		                          |__________|_________|______|..........|


		All non-resource strings* are stored in the following manner:

		   length-  string
		 |________|________|

		Numeric values are stored as 32-bit unsigned integers.
		Numeric values marked + are stored as 64-bit unsigned integers.
		Numeric values marked - are stored as 8-bit unsigned integers.

		*/

		// verify each file's existence
		verifyFiles(inf, dict);

		// sanitize options
		if (options.clevel > 9) {
			options.clevel = 9;
		}
		capStringSizeTo255(options.headstr);

		// print options info
		printOptionsInfo(inf, options);

		// prepare bgn and end endpoints
		std::string const* bgnEndpoint = nullptr;
		std::string const* endEndpoint = nullptr;

		switch (options.chksum)
		{
			case ChkSumOption::ADLER32:
				bgnEndpoint = &LM_BGN_ADLER32;
				endEndpoint = &LM_END_ADLER32;
				break;
			case ChkSumOption::CRC32:
				bgnEndpoint = &LM_BGN_CRC32;
				endEndpoint = &LM_END_CRC32;
				break;
			case ChkSumOption::NONE:
				bgnEndpoint = &LM_BGN_NOCHKSUM;
				endEndpoint = &LM_END_NOCHKSUM;
				break;
		}

		// prepare header data
		const uint8_t limeRevision = LIME_REVISION;
		const std::string* headString = &options.headstr;

		size_t dictPlaceholderOffset = 0u;

		//
		// pack data
		//

		inf << "\nWriting data file: " << outputFilename << " ... ";

		size_t totalRead = 0;

		std::ofstream datafileStream(outputFilename, std::ios::out | std::ofstream::binary);

		if (!datafileStream.is_open())
		{
			// abort packing
			throw std::runtime_error("Unable to open file for writing: " + outputFilename);
		}

		// bgn endpoint
		if (bgnEndpoint)
		{
			datafileStream.write(bgnEndpoint->data(), bgnEndpoint->size());
		}

		// lime revision
		{
			T_Bytes limeRevisionBytes = toBytes(toBigEndian(limeRevision));
			datafileStream.write(reinterpret_cast<const char*>(limeRevisionBytes.data()), limeRevisionBytes.size());
		}


		// head length
		{
			uint8_t headLength = static_cast<uint8_t>(headString->size());
			T_Bytes headLengthBytes = toBytes(toBigEndian(headLength));
			datafileStream.write(reinterpret_cast<const char*>(headLengthBytes.data()), headLengthBytes.size());
		}

		// head string
		datafileStream.write(reinterpret_cast<const char*>(headString->data()), headString->size());

		// write placeholders for header dict information
		// these will get overwritten in the last step when we have the full dict compiled
		{
			// memorize the offset
			dictPlaceholderOffset = datafileStream.tellp();
			// both values are 32-bit uint
			T_Bytes dictPlaceholderBytes = toBytes<uint32_t>(0u);
			// dict size placeholder
			datafileStream.write(reinterpret_cast<const char*>(dictPlaceholderBytes.data()), dictPlaceholderBytes.size());
			// dict checksum placeholder
			if (options.chksum != ChkSumOption::NONE)
			{
				datafileStream.write(reinterpret_cast<const char*>(dictPlaceholderBytes.data()), dictPlaceholderBytes.size());
			}
		}

		// pack user resources
		struct DictItemData
		{
			size_t offset = 0;
			uint32_t checksum = 0;
			size_t size = 0;
		};

		DMap<DMap<DictItemData>> dictDataMap;
		std::unordered_map<std::string, DictItemData const*> knownFilenameMap; // used for detecting duplicates

		static const size_t inBuffSize = 512u;
		static const size_t outBuffSize = 16348u;

		Bytef* inputBuffer = new Bytef[inBuffSize];
		Bytef* outputBuffer = new Bytef[outBuffSize];

		for (auto it = dict.begin(); it != dict.end(); ++it)
		{
			auto category = it->first;
			capStringSizeTo255(category);

			const bool isMeta = (category.length() && category[0] == '@');
			auto const& collection = it->second;

			for (auto it2 = collection.begin(); it2 != collection.end(); ++it2)
			{
				auto key = it2->first;
				capStringSizeTo255(key);

				auto const& value = it2->second;

				uint32_t checksum = 0u;

				if (isMeta)
				{
					// meta category, store value directly
					std::string const& data = value;

					const size_t offset = datafileStream.tellp();

					uLong dataBytesCompressedSize = compressBound(static_cast<uLong>(data.size()));
					Bytef* dataBytesCompressedData = new Bytef[dataBytesCompressedSize];
					if (compress2(dataBytesCompressedData, &dataBytesCompressedSize, (const Bytef*)data.data(), static_cast<uLong>(data.size()), options.clevel) != Z_OK)
					{
						// abort packing
						throw std::runtime_error("Unable to compress data.");
					}

					datafileStream.write(reinterpret_cast<const char*>(dataBytesCompressedData), dataBytesCompressedSize);

					totalRead += data.size();

					switch (options.chksum)
					{
						case ChkSumOption::ADLER32:
							checksum = adler32_z(0ul, reinterpret_cast<const Bytef*>(data.c_str()), data.size());
							break;
						case ChkSumOption::CRC32:
							checksum = crc32_z(0ul, reinterpret_cast<const Bytef*>(data.c_str()), data.size());
							break;
					}

					// store offset, checksum and size
					dictDataMap[category][key] = { offset, checksum, static_cast<size_t>(dataBytesCompressedSize) };
				}
				else
				{
#if defined(_WIN32)
					std::string resFilename = value;
					// on windows we can safely lowercase the filename so we don't end up with duplicate data
					// due to mismatched case
					std::transform(resFilename.begin(), resFilename.end(), resFilename.begin(), ::tolower);
#else
					std::string const& resFilename = value;
#endif
					DictItemData& itemData = dictDataMap[category][key];

					auto knownFilenameIt = knownFilenameMap.find(resFilename);
					if (knownFilenameIt != knownFilenameMap.end())
					{
						// we already packed this file
						// simply reference the same item and skip packing for this item
						itemData = *knownFilenameIt->second;
						continue;
					}

					knownFilenameMap[resFilename] = &itemData;

					size_t totalWritten = 0u;

					// get current offset
					const size_t offset = datafileStream.tellp();

					// pack data from resource file
					std::ifstream resourceStream(resFilename, std::ios::in | std::ifstream::binary);

					if (!resourceStream.is_open())
					{
						throw std::runtime_error("Unable to open file: " + resFilename);
					}

					z_stream cmpStream;
					cmpStream.zalloc = Z_NULL;
					cmpStream.zfree = Z_NULL;
					cmpStream.opaque = Z_NULL;

					if (deflateInit(&cmpStream, options.clevel) != Z_OK)
					{
						throw std::runtime_error("Unable to compress data.");
					}

					size_t numRead = 0;
					size_t numReadTotal = 0;

					bool isEof = false;

					do {
						resourceStream.read(reinterpret_cast<char*>(inputBuffer), inBuffSize);

						numRead = resourceStream.gcount();
						numReadTotal += numRead;

						isEof = resourceStream.eof();

						cmpStream.next_in = &inputBuffer[0];
						cmpStream.avail_in = static_cast<uInt>(numRead);

						const int flush = isEof ? Z_FINISH : Z_NO_FLUSH;

						do {
							cmpStream.next_out = &outputBuffer[0];
							cmpStream.avail_out = outBuffSize;

							int streamState = deflate(&cmpStream, flush);

							if (streamState == Z_STREAM_ERROR)
							{
								delete[] inputBuffer;
								delete[] outputBuffer;
								throw std::runtime_error("Unable to compress data.");
							}

							const size_t compressedChunkSize = outBuffSize - cmpStream.avail_out;

							datafileStream.write(reinterpret_cast<const char*>(outputBuffer), compressedChunkSize);

							totalWritten += compressedChunkSize;

						} while (cmpStream.avail_out == 0);

						switch (options.chksum)
						{
							case ChkSumOption::ADLER32:
								checksum = adler32_z(checksum, inputBuffer, numRead);
								break;
							case ChkSumOption::CRC32:
								checksum = crc32_z(checksum, inputBuffer, numRead);
								break;
						}

					} while (!isEof);

					resourceStream.close();

					if (deflateEnd(&cmpStream) != Z_OK)
					{
						delete[] inputBuffer;
						delete[] outputBuffer;
						throw std::runtime_error("Unable to compress data.");
					}

					totalRead += numReadTotal;

					// store offset, checksum and size
					itemData = { offset, checksum, totalWritten };
				}
			}
		}
		delete[] inputBuffer;
		delete[] outputBuffer;

		// create the dictionary binary
		T_Bytes dictBytes;

		uint32_t N_categories = static_cast<uint32_t>(dict.size());
		appendBytes(dictBytes, toBytes(toBigEndian(N_categories)));

		for (auto const& it : dictDataMap)
		{
			auto categoryKey = it.first;
			auto const& collection = it.second;

			const bool isMeta = (categoryKey.length() && categoryKey[0] == '@');

			if (isMeta)
			{
				categoryKey.erase(0, 1);
			}

			uint8_t categoryKeySize = static_cast<uint8_t>(categoryKey.size());
			appendBytes(dictBytes, toBytes(toBigEndian(categoryKeySize)));
			appendBytes(dictBytes, categoryKey);

			uint32_t M_keys = static_cast<uint32_t>(collection.size());
			appendBytes(dictBytes, toBytes(toBigEndian(M_keys)));

			for (auto const& it2 : collection)
			{
				auto const& collectionKey = it2.first;
				auto const& itemData = it2.second;

				uint8_t collectionKeySize = static_cast<uint8_t>(collectionKey.size());
				appendBytes(dictBytes, toBytes(toBigEndian(collectionKeySize)));
				appendBytes(dictBytes, collectionKey);
				
				uint64_t seek_id = static_cast<uint64_t>(itemData.offset);
				appendBytes(dictBytes, toBytes(toBigEndian(seek_id)));

				uint64_t resourceSize = static_cast<uint64_t>(itemData.size);
				appendBytes(dictBytes, toBytes(toBigEndian(resourceSize)));

				if (options.chksum != ChkSumOption::NONE)
				{
					uint32_t const& checksum = itemData.checksum;
					appendBytes(dictBytes, toBytes(toBigEndian(checksum)));
				}
			}
		}

		// calculate dict checksum
		uint32_t dictChecksum = 0;
		switch (options.chksum)
		{
			case ChkSumOption::ADLER32:
				dictChecksum = adler32_z(0ul, dictBytes.data(), dictBytes.size());
				break;
			case ChkSumOption::CRC32:
				dictChecksum = crc32_z(0ul, dictBytes.data(), dictBytes.size());
				break;
		}

		// compress dictionary binary

		uLong dictBytesCompressedSize = compressBound(static_cast<uLong>(dictBytes.size()));
		Bytef* dictBytesCompressedData = new Bytef[dictBytesCompressedSize];
		
		if (compress2(dictBytesCompressedData, &dictBytesCompressedSize, dictBytes.data(), static_cast<uLong>(dictBytes.size()), options.clevel) != Z_OK)
		{
			delete[] dictBytesCompressedData;
			throw std::runtime_error("Unable to compress dictionary.");
		}

		// we can now dispose of uncompressed dict bytes
		dictBytes.clear();

		// write dictionary
		datafileStream.write(reinterpret_cast<const char*>(dictBytesCompressedData), static_cast<size_t>(dictBytesCompressedSize));

		// end endpoint
		if (endEndpoint)
		{
			datafileStream.write(endEndpoint->data(), endEndpoint->size());
		}

		// write dictionary data to header
		datafileStream.seekp(dictPlaceholderOffset);
		{
			const uint32_t dictSize = static_cast<uint32_t>(dictBytesCompressedSize);
			T_Bytes dictSizeBytes = toBytes(toBigEndian(dictSize));
			datafileStream.write(reinterpret_cast<const char*>(dictSizeBytes.data()), dictSizeBytes.size());
		}
		if (options.chksum != ChkSumOption::NONE)
		{
			T_Bytes dictChecksumBytes = toBytes(toBigEndian(dictChecksum));
			datafileStream.write(reinterpret_cast<const char*>(dictChecksumBytes.data()), dictChecksumBytes.size());
		}

		// all done
		datafileStream.close();

		// cleanup
		delete[] dictBytesCompressedData;

		// writing successful, print out some statistics
		size_t totalDataSize = fileSize(outputFilename.c_str());
		const float compressionRatio = (1.f - totalDataSize * 1.f / totalRead) * 100.f;
		char compressionRatioStr[16];
#if defined(_WIN32)
		sprintf_s(compressionRatioStr, "%4.2f", compressionRatio);
#else
		sprintf(compressionRatioStr, "%4.2f", compressionRatio);
#endif

		inf.ok("done")
			<< "\n\nRead " << totalRead << " bytes, wrote " << totalDataSize << " bytes.\n"
			<< "Compression ratio: " << compressionRatioStr << "%\n";
	}
}
