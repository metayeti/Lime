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

namespace Lime
{
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

		                                    data key*  seek_id   size   checksum
		                                  |__________|_________|______|..........|

		Header:

		   bgn   version*  head*  dict size   data size
		 |_____|_________|______|___________|___________|

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

		std::unordered_map<std::string, std::unordered_map<std::string, DictItemData>> dictDataMap;
		std::unordered_map<std::string, size_t> filenameOffsetMap;

		// first pack a temporary user data file
		std::string tmpDataFilename = "~" + outputFilename;

		std::string outFileMode = std::string("wb" + std::to_string(options.clevel));
		gzFile outFile = gzopen(tmpDataFilename.c_str(), outFileMode.c_str());

		if (!outFile)
		{
			throw std::runtime_error("Unable to open temporary file: " + tmpDataFilename);
		}

		bool first = true;
		uint32_t checksum = 0;

		for (auto it = dict.begin(); it != dict.end(); ++it)
		{
			auto const& category = it->first;
			const bool isMeta = (category.length() && category[0] == '@');
			auto const& collection = it->second;

			for (auto it2 = collection.begin(); it2 != collection.end(); ++it2)
			{
				auto const& key = it2->first;
				auto const& value = it2->second;

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

					// store offset and checksum
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
					if (fopen_s(&resFile, resFilename.c_str(), "rb") != 0)
					{
						// abort packing
						gzclose(outFile);
						remove(tmpDataFilename.c_str());
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
						while ((numRead = fread_s(buffer, 16348u, 1, sizeof(buffer), resFile)) > 0)
						{
							totalRead += numRead;
							if (gzwrite(outFile, buffer, static_cast<unsigned int>(numRead)) != static_cast<int>(numRead))
							{
								// abort packing
								fclose(resFile);
								gzclose(outFile);
								remove(tmpDataFilename.c_str());
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
						remove(tmpDataFilename.c_str());
						throw std::runtime_error("Unable to read from file: " + resFilename);
					}
				}

				first = false;

				inf << "\nChecksum for " << category << ", " << key << ": " << checksum << "\n";
			}
		}

		// close the stream
		gzclose(outFile);

		// writing successful

		inf.ok("done")
			<< "\n\nWriting successful.\n\n"
			<< "Read 0 bytes, wrote 0 bytes.\n"
			<< "Compression ratio: 0%\n";
	}
}