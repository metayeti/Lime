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
#include <sys/stat.h>
#include <cstdio>
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
		/*
		for (size_t i = sizeof(T); i-- > 0; )
		{
			bytes.push_back(static_cast<Bytef>(value >> (i * 8)));
		}
		*/
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
		struct stat buff;
		return !stat(filename, &buff);
	}

	size_t fileSize(const char* filename)
	{
		struct stat buff;
		stat(filename, &buff);
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
				auto const& key = it2.first;
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

	void pack(Interface& inf, Dict& dict, std::string const& outputFilename, PackOptions& options)
	{
		/*

		Lime datafile structure:

		            Z0           Z1    ...   Zn
		           [~~~~~~~~~~] [~~~] [~~~] [~~~]      (zipped content)

		   header   dictionary   user resources   end
		 |________|____________|________________|_____|
		                |
		                |
		                |
		            dictionary:

		            N   category 1   ...   category N
		          |___|____________|     |____________|
		                    |
		                    |
		                    |
		                 category:

		                 category key*  M   data 1   ...   data M
		               |______________|___|________|     |________|
		                                      |
		                                      |
		                                      |
		                                    data:

		                                    data key*  seek_id   checksum
		                                  |__________|_________|..........|

		Header:

		   bgn   version*  head*  dict size   dict checksum   data size
		 |_____|_________|______|___________|...............|___________|

		All non-resource strings* are stored in the following manner:

		   length   string
		 |________|________|

		*/

		// verify each file's existence
		verifyFiles(inf, dict);

		// sanitize options
		if (options.clevel > 9) {
			options.clevel = 9;
		}
		if (options.headstr.size() > 255u) {
			options.headstr = options.headstr.substr(0u, 255u);
		}

		// print options info
		printOptionsInfo(inf, options);

		//
		// pack data
		//

		inf << "\nWriting data file: " << outputFilename << " ... ";

		size_t totalRead = 0;

		struct DictItemData
		{
			size_t offset = 0;
			uint32_t checksum = 0;
		};

		//std::unordered_map<std::string, std::unordered_map<std::string, DictItemData>> dictDataMap;

		using T_DictData = DMap<DMap<DictItemData>>;
		T_DictData dictDataMap;

		std::unordered_map<std::string, size_t> filenameOffsetMap; // used for detecting duplicates

		// first pack a temporary user data file
		std::string tmpDataFilename = "~" + outputFilename;

		std::string outFileMode = std::string("wb" + std::to_string(options.clevel));
		gzFile outFile = gzopen(tmpDataFilename.c_str(), outFileMode.c_str());

		if (!outFile)
		{
			throw std::runtime_error("Unable to open temporary file: " + tmpDataFilename);
		}

		bool first = true;

		for (auto it = dict.begin(); it != dict.end(); ++it)
		{
			auto const& category = it->first;
			const bool isMeta = (category.length() && category[0] == '@');
			auto const& collection = it->second;

			for (auto it2 = collection.begin(); it2 != collection.end(); ++it2)
			{
				auto const& key = it2->first;
				auto const& value = it2->second;

				uint32_t checksum = 0;

				if (isMeta)
				{
					// meta category, store value directly
					auto const& data = value;

					// flush previous stream
					if (!first)
					{
						gzflush(outFile, Z_FINISH);
					}

					gzwrite(outFile, data.data(), static_cast<unsigned int>(data.size()));

					switch (options.chksum)
					{
					case ChkSumOption::ADLER32:
						checksum = adler32(0ul, reinterpret_cast<const Bytef*>(data.c_str()), static_cast<unsigned int>(data.size()));
						break;
					case ChkSumOption::CRC32:
						checksum = crc32(0ul, reinterpret_cast<const Bytef*>(data.c_str()), static_cast<unsigned int>(data.size()));
						break;
					}

					// store offset, size and checksum
					dictDataMap[category][key] = { static_cast<size_t>(gzoffset(outFile)), checksum };
				}
				else
				{
#if defined(_WIN32)
					auto resFilename = value;
					std::transform(resFilename.begin(), resFilename.end(), resFilename.begin(), ::tolower);
#else
					auto const& resFilename = value;
#endif
					auto filenameOffsetIt = filenameOffsetMap.find(resFilename);
					if (filenameOffsetIt != filenameOffsetMap.end())
					{
						// we already packed this file so simply reference the same item and skip packing again
						auto& dataItem = dictDataMap[category][key];
						dataItem = { filenameOffsetIt->second, dataItem.checksum };
						continue;
					}

					// flush previous stream
					if (!first)
					{
						gzflush(outFile, Z_FINISH);
					}

					// store current offset
					dictDataMap[category][key] = { filenameOffsetMap[resFilename] = static_cast<size_t>(gzoffset(outFile)) };

					// pack data from file
					const size_t resFileSize = fileSize(resFilename.c_str());
					FILE* resFile;
#if defined(_WIN32)
					if (fopen_s(&resFile, resFilename.c_str(), "rb") != 0)
#else
					if ((resFile = fopen(resFilename.c_str(), "rb")) == nullptr)
#endif
					{
						// abort packing
						gzclose(outFile);
						std::remove(tmpDataFilename.c_str());
						throw std::runtime_error("Unable to open file: " + resFilename);
					}

					if (resFile)
					{
						switch (options.chksum)
						{
						case ChkSumOption::ADLER32:
							checksum = adler32(0ul, Z_NULL, 0u);
							break;
						case ChkSumOption::CRC32:
							checksum = crc32(0ul, Z_NULL, 0u);
							break;
						}

						Bytef* buffer = new Bytef[16348];
						size_t numRead = 0;
#if defined(_WIN32)
						while ((numRead = fread_s(buffer, 16348u, 1, sizeof(buffer), resFile)) > 0)
#else
						while ((numRead = fread(buffer, 1, 16348u, resFile)) > 0)
#endif
						{
							totalRead += numRead;
							if (gzwrite(outFile, buffer, static_cast<unsigned int>(numRead)) != static_cast<int>(numRead))
							{
								// abort packing
								fclose(resFile);
								gzclose(outFile);
								std::remove(tmpDataFilename.c_str());
								throw std::runtime_error("Unable to write to temporary file: " + tmpDataFilename);
							}
							switch (options.chksum)
							{
								case ChkSumOption::ADLER32:
									checksum = adler32(checksum, buffer, static_cast<unsigned int>(numRead));
									break;
								case ChkSumOption::CRC32:
									checksum = crc32(checksum, buffer, static_cast<unsigned int>(numRead));
									break;
							}
						}
						dictDataMap[category][key].checksum = checksum;
						delete[] buffer;
						fclose(resFile);
					}
					else
					{
						// abort packing
						gzclose(outFile);
						std::remove(tmpDataFilename.c_str());
						throw std::runtime_error("Unable to read from file: " + resFilename);
					}
				}

				first = false;

				inf << "\nChecksum for " << category << ", " << key << ": " << checksum << "\n";
			}
		}

		// close the stream
		gzclose(outFile);

		// create the dictionary binary
		T_Bytes dictBytes;

		uint32_t N_categories = static_cast<uint32_t>(dict.size());
		appendBytes(dictBytes, toBytes(toBigEndian(N_categories)));

		for (auto const& it : dictDataMap)
		{
			auto const& categoryKey = it.first;
			auto const& collection = it.second;

			uint32_t categoryKeySize = static_cast<uint32_t>(categoryKey.size());
			appendBytes(dictBytes, toBytes(toBigEndian(categoryKeySize)));
			appendBytes(dictBytes, categoryKey);

			uint32_t M_keys = static_cast<uint32_t>(collection.size());
			appendBytes(dictBytes, toBytes(toBigEndian(M_keys)));

			inf << "\n?? " << categoryKey;

			for (auto const& it2 : collection)
			{
				auto const& collectionKey = it2.first;
				auto const& itemData = it2.second;

				inf << "\n   -> " << collectionKey << "\n";

				uint32_t collectionKeySize = static_cast<uint32_t>(collectionKey.size());
				appendBytes(dictBytes, toBytes(toBigEndian(collectionKeySize)));
				appendBytes(dictBytes, collectionKey);
				
				uint32_t seek_id = static_cast<uint32_t>(itemData.offset);
				appendBytes(dictBytes, toBytes(toBigEndian(seek_id)));

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
				dictChecksum = adler32(0ul, dictBytes.data(), static_cast<unsigned int>(dictBytes.size()));
				break;
			case ChkSumOption::CRC32:
				dictChecksum = crc32(0ul, dictBytes.data(), static_cast<unsigned int>(dictBytes.size()));
				break;
		}

		// compress dictionary binary

		uLong dictBytesCompressedSize = compressBound(static_cast<uLong>(dictBytes.size()));
		Bytef* dictBytesCompressedData = new Bytef[dictBytesCompressedSize];
		
		if (compress2(dictBytesCompressedData, &dictBytesCompressedSize, dictBytes.data(), static_cast<uLong>(dictBytes.size()), options.clevel) != Z_OK)
		{
			std::remove(tmpDataFilename.c_str());
			delete[] dictBytesCompressedData;
			throw std::runtime_error("Unable to compress dictionary.");
		}

		////T_Bytes dictBytesCompressed(dictBytesCompressedData, dictBytesCompressedData + dictBytesCompressedSize);

		// prepare bgn and end endpoints
		const std::string* bgnEndpoint = nullptr;
		const std::string* endEndpoint = nullptr;

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
		const std::string* limeVersion = &LIME_VERSION;
		const std::string* headString = &options.headstr;

		uint32_t dataSize = fileSize(tmpDataFilename.c_str());

		// write to datafile

		FILE* dataFile;
#if defined(_WIN32)
		if (fopen_s(&dataFile, outputFilename.c_str(), "wb") != 0)
#else
		if ((dataFile = fopen(outputFilename.c_str(), "wb")) == nullptr)
#endif
		{
			// abort packing
			std::remove(tmpDataFilename.c_str());
			delete[] dictBytesCompressedData;
			throw std::runtime_error("Unable to open file for writing: " + outputFilename);
		}

		if (dataFile)
		{
			// bgn endpoint
			if (fwrite(bgnEndpoint->data(), 1u, bgnEndpoint->size(), dataFile) != bgnEndpoint->size())
			{
				throw std::runtime_error("?");
			}

			// version length
			uint32_t versionLength = static_cast<uint32_t>(limeVersion->size());
			T_Bytes versionLengthBytes = toBytes(toBigEndian(versionLength));
			if (fwrite(versionLengthBytes.data(), 1u, versionLengthBytes.size(), dataFile) != versionLengthBytes.size())
			{
				throw std::runtime_error("?");
			}

			// version string
			if (fwrite(limeVersion->data(), 1u, limeVersion->size(), dataFile) != limeVersion->size())
			{
				throw std::runtime_error("?");
			}

			// head length
			uint32_t headLength = static_cast<uint32_t>(headString->size());
			T_Bytes headLengthBytes = toBytes(toBigEndian(headLength));
			if (fwrite(headLengthBytes.data(), 1u, headLengthBytes.size(), dataFile) != headLengthBytes.size())
			{
				throw std::runtime_error("?");
			}

			// head string
			if (fwrite(headString->data(), 1u, headString->size(), dataFile) != headString->size())
			{
				throw std::runtime_error("?");
			}

			// dict size
			uint32_t dictSize = static_cast<uint32_t>(dictBytesCompressedSize);
			T_Bytes dictSizeBytes = toBytes(toBigEndian(dictSize));
			if (fwrite(dictSizeBytes.data(), 1u, dictSizeBytes.size(), dataFile) != dictSizeBytes.size())
			{
				throw std::runtime_error("?");
			}

			// dict checksum
			if (options.chksum != ChkSumOption::NONE)
			{
				T_Bytes dictChecksumBytes = toBytes(toBigEndian(dictChecksum));
				if (fwrite(dictChecksumBytes.data(), 1u, dictChecksumBytes.size(), dataFile) != dictChecksumBytes.size())
				{
					throw std::runtime_error("?");
				}
			}

			// data size
			T_Bytes dataSizeBytes = toBytes(toBigEndian(dataSize));
			if (fwrite(dataSizeBytes.data(), 1u, dataSizeBytes.size(), dataFile) != dataSizeBytes.size())
			{
				throw std::runtime_error("?");
			}

			// write compressed dictionary
			if (fwrite(dictBytesCompressedData, 1u, static_cast<size_t>(dictBytesCompressedSize), dataFile) != static_cast<size_t>(dictBytesCompressedSize))
			{
				throw std::runtime_error("?");
				/*
				// abort packing
				fclose(dataFile);
				remove(tmpDataFilename.c_str());
				delete[] dictBytesCompressedData;
				throw std::runtime_error("Unable to write to file: " + outputFilename);
				*/
			}

			// write compressed data from the temporary data file
			FILE* cDataFile;
#if defined(_WIN32)
			if (fopen_s(&cDataFile, tmpDataFilename.c_str(), "rb") != 0)
#else
			if ((cDataFile = fopen(tmpDataFilename.c_str(), "rb")) == nullptr)
#endif
			{
				throw std::runtime_error("?");
			}

			if (cDataFile)
			{
				Bytef* buffer = new Bytef[16348];
				size_t numRead = 0;
				size_t totalDataRead = 0;
#if defined(_WIN32)
				while ((numRead = fread_s(buffer, 16348u, 1, sizeof(buffer), cDataFile)) > 0)
#else
				while ((numRead = fread(buffer, 1, 16348u, cDataFile)) > 0)
#endif
				{
					totalDataRead += numRead;
					if (fwrite(buffer, 1u, numRead, dataFile) != numRead)
					{
						throw std::runtime_error("?");
					}
				}

				delete[] buffer;
				fclose(cDataFile);

				if (totalDataRead != dataSize)
				{
					// something isn't right
					throw std::runtime_error("?");
				}
			}

			// end endpoint
			if (fwrite(endEndpoint->data(), 1u, endEndpoint->size(), dataFile) != endEndpoint->size())
			{
				throw std::runtime_error("?");
			}
			
			// all done, close the file handle
			fclose(dataFile);
		}

		// cleanup
		delete[] dictBytesCompressedData;
		std::remove(tmpDataFilename.c_str());
		

		// writing successful

		inf.ok("done")
			<< "\n\nWriting successful.\n\n"
			<< "Read 0 bytes, wrote 0 bytes.\n"
			<< "Compression ratio: 0%\n";
	}
}
