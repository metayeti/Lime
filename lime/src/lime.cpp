/*
 *  Lime
 *
 *  Utility for Lime datafile creation.
 *
 *  lime.cpp
 *  Program entry point. 
 *
 */

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

#include <vector>
#include <algorithm>
/*
#include <cstdio>
*/
#include <cstdint>
#include <cstring>
#include <sys/stat.h>
#include <zlib.h>
#include "interface.h"

const char* const LIME_VERSION = "0.2.0";
const char* const LIME_COPYRIGHT_YEARS = "2019";
const char* const LIME_COPYRIGHT_AUTHOR = "Danijel Durakovic";

const size_t LIME_HEAD_SIZE = 2;
const char* const LIME_HEAD = "LM";

// define our own printf to avoid compilation warnings on Windows
/*
#if defined(_WIN32)
	#define lm_printf(...) printf_s(__VA_ARGS__)	
#else
	#define lm_printf(...) printf(__VA_ARGS__)
#endif
*/

#if defined(_WIN32)
	const char* const PATH_SEPARATOR = "\\";
#else
	const char* const PATH_SEPARATOR = "/";
#endif

using ByteData = std::vector<Bytef>;

bool fileExists(const char* filename)
{
	struct stat buff;
	return !stat(filename, &buff);
}

unsigned int fileSize(const char* filename)
{
	struct stat buff;
	stat(filename, &buff);
	return buff.st_size;
}

template<typename T>
ByteData toBytes(T const& value)
{
	ByteData bytes;
	for (size_t i = sizeof(T); i --> 0; )
	{
		bytes.push_back(static_cast<Bytef>(value >> (i * 8)));
	}
	return bytes;
}

inline void appendBytes(ByteData& a, ByteData& b)
{
	a.insert(a.end(), b.begin(), b.end());
}

bool pack(const char* resourceManifestFilename, const char* outputFilename)
{
	/*
	
	Datafile structure:


	         Z0           Z1    ...   Zn
	        [~~~~~~~~~~] [~~~] [~~~] [~~~]        (zipped content)

	  head   dictionary   user resources   checksum
	|______|____________|________________|__________|
	             |
	             |
	             |
	             |
	         dictionary:

	         N   section 1   ...   section N
	       |___|___________|     |___________|
	                 |
	                 |
	                 |
	                 |
	              section:

	              section key   N   data 1   ...   data N
	            |_____________|___|________|     |________|
	                                  |
	                                  |
	                                  |
	                                data:

	                                data key   seek_id   size
	                              |__________|_________|______|

	*/

	return false;
}

void printHeader(const char* const executableName, Lime::Interface& inf)
{
	inf.print("\n");
	inf.setConsoleColor(Lime::Interface::Color::BRIGHTBLUE);
	inf.print(" -----| Lime");
	inf.setConsoleColor(Lime::Interface::Color::BRIGHTRED);
	inf.print(" %s", LIME_VERSION);
	inf.setConsoleColor(Lime::Interface::Color::BRIGHTBLUE);
	inf.print(" |-----");
	inf.setConsoleColor(Lime::Interface::Color::GRAY);
	inf.print("\n      Datafile packer\n");
	inf.print("(c) %s %s\n\n", LIME_COPYRIGHT_YEARS, LIME_COPYRIGHT_AUTHOR);
	inf.setConsoleColor(Lime::Interface::Color::WHITE);
	inf.print("Usage:\n");
	inf.setConsoleColor(Lime::Interface::Color::YELLOW);
	inf.print("  %s [resource manifest file] [output file]\n", executableName);
}

void printHelp(char** argv)
{
	/*
	lm_printf("Use this utility to prepare and pack datafiles.\n\n");
	lm_printf("To use this utility, you need to create a resource manifest first. The resource manifest\n");
	lm_printf("is an INI-formatted file that contains references to files or values that will be stored\n");
	lm_printf("in the datafile.\n\n");
	lm_printf("A resource manifest file consists of categories, keys and values:\n\n");
	lm_printf("  ; comment\n");
	lm_printf("  [category]\n");
	lm_printf("  key = value\n\n");
	lm_printf("An example resource manifest entry can look like this:\n\n");
	lm_printf("  [graphics]\n");
	lm_printf("  sprite1 = graphics%ssprite1.png\n", PATH_SEPARATOR);
	lm_printf("  sprite2 = graphics%ssprite2.png\n\n", PATH_SEPARATOR);
	lm_printf("Lime interprets the value of every entry as a filename containing data to be packed.\n");
	lm_printf("Note that filenames are not stored in the datafile, data is indexed based on key.\n\n");
	lm_printf("You can add meta categories by prefixing category names with @. This will store values\n");
	lm_printf("directly. Example use:\n\n");
	lm_printf("  [@meta]\n");
	lm_printf("  datafile version = 1.0.0\n");
	lm_printf("  important info = Dinosaurs are awesome!\n\n");
	lm_printf("When unpacking, you can use category and keys to reach data. Note that Lime datafiles do\n");
	lm_printf("not contain any information about the types of files stored in it.\n\n");
	lm_printf("When you're done creating the resource manifest, you can use the utility, for example:\n\n");
	lm_printf("  %s resources.manifest datafile.dat\n\n", argv[0]);
	lm_printf("If the manifest changes often and you need to compile the datafile many times over, it\n");
	lm_printf("is recommended that you create a batch script to automate the process.\n");
	*/
}

int main(int argc, char* argv[])
{
	const char* const executableName = argv[0];
	Lime::Interface inf;

	if (argc == 2 && !strcmp(argv[1], "--help"))
	{
		printHeader(argv[0], inf);
		//printHeader(argv);
		//printHelp(argv);
	}
	else if (argc != 3)
	{
		printHeader(executableName, inf);
		inf.setConsoleColor(Lime::Interface::Color::WHITE);
		inf.print("\nUse ", argv[0]);
		inf.setConsoleColor(Lime::Interface::Color::YELLOW);
		inf.print("%s --help", executableName);
		inf.setConsoleColor(Lime::Interface::Color::WHITE);
		inf.print(" for more information.\n");
	}
	else
	{
		pack(argv[1], argv[2]);
	}

#if !defined(_WIN32)
	inf.print("\n");
#endif

	inf.restoreConsoleColor();

	return 0;
}
