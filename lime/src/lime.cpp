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

void printHeader(Lime::Interface& inf)
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
	inf.restoreConsoleColor();
}

void printUsage(const char* const executableName, Lime::Interface& inf)
{
	inf.setConsoleColor(Lime::Interface::Color::WHITE);
	inf.print("Usage:\n\n");
	inf.setConsoleColor(Lime::Interface::Color::YELLOW);
	inf.print("  %s [resource manifest file] [output file]\n", executableName);
	inf.restoreConsoleColor();
}

void printHelp(const char* const executableName, Lime::Interface& inf)
{
#if defined(_WIN32)
	static const char* const scriptEnv = "batch";
#else
	static const char* const scriptEnv = "bash";
#endif
	inf.setConsoleColor(Lime::Interface::Color::WHITE);
	inf.print("\nUse this utility to pack your Lime datafiles.\n\n");
	inf.print("To use this utility, you will need to create a resource manifest file.\n");
	inf.print("The resource manifest is an INI-formatted file that contains references\n");
	inf.print("to your asset files. It contains entries that consist of categories,\n");
	inf.print("keys and values:\n\n");
	inf.setConsoleColor(Lime::Interface::Color::GRAY);
	inf.print("  ; comments exist on a single line\n");
	inf.setConsoleColor(Lime::Interface::Color::BRIGHTRED);
	inf.print("  [");
	inf.setConsoleColor(Lime::Interface::Color::BRIGHTBLUE);
	inf.print("category");
	inf.setConsoleColor(Lime::Interface::Color::BRIGHTRED);
	inf.print("]\n");
	inf.setConsoleColor(Lime::Interface::Color::BRIGHTGREEN);
	inf.print("  key ");
	inf.setConsoleColor(Lime::Interface::Color::WHITE);
	inf.print("=");
	inf.setConsoleColor(Lime::Interface::Color::YELLOW);
	inf.print(" value\n\n");
	inf.setConsoleColor(Lime::Interface::Color::WHITE);
	inf.print("An example resource manifest entry can look like this:\n\n");
	inf.setConsoleColor(Lime::Interface::Color::GRAY);
	inf.print("  ; graphics for my awesome game\n");
	inf.setConsoleColor(Lime::Interface::Color::BRIGHTRED);
	inf.print("  [");
	inf.setConsoleColor(Lime::Interface::Color::BRIGHTBLUE);
	inf.print("graphics");
	inf.setConsoleColor(Lime::Interface::Color::BRIGHTRED);
	inf.print("]\n");
	inf.setConsoleColor(Lime::Interface::Color::BRIGHTGREEN);
	inf.print("  sprite1 ");
	inf.setConsoleColor(Lime::Interface::Color::WHITE);
	inf.print("=");
	inf.setConsoleColor(Lime::Interface::Color::YELLOW);
	inf.print(" graphics% ssprite1.png\n", PATH_SEPARATOR);
	inf.setConsoleColor(Lime::Interface::Color::BRIGHTGREEN);
	inf.print("  sprite2 ");
	inf.setConsoleColor(Lime::Interface::Color::WHITE);
	inf.print("=");
	inf.setConsoleColor(Lime::Interface::Color::YELLOW);
	inf.print(" graphics% ssprite2.png\n\n", PATH_SEPARATOR);
	inf.setConsoleColor(Lime::Interface::Color::WHITE);
	inf.print("Lime interprets the value of every entry as a file that contains data\n");
	inf.print("to be packed. Note that filenames are lost in the process and data is\n");
	inf.print("indexed by the provided category and key.\n\n");
	inf.print("You can add meta-categories by prefixing category names with ");
	inf.setConsoleColor(Lime::Interface::Color::BRIGHTCYAN);
	inf.print("@");
	inf.setConsoleColor(Lime::Interface::Color::WHITE);
	inf.print(". In\n");
	inf.print("this case all values in this category will be stored directly:\n\n");
	inf.setConsoleColor(Lime::Interface::Color::BRIGHTRED);
	inf.print("  [");
	inf.setConsoleColor(Lime::Interface::Color::BRIGHTCYAN);
	inf.print("@");
	inf.setConsoleColor(Lime::Interface::Color::BRIGHTBLUE);
	inf.print("metainfo");
	inf.setConsoleColor(Lime::Interface::Color::BRIGHTRED);
	inf.print("]\n");
	inf.setConsoleColor(Lime::Interface::Color::BRIGHTGREEN);
	inf.print("  important info ");
	inf.setConsoleColor(Lime::Interface::Color::WHITE);
	inf.print("=");
	inf.setConsoleColor(Lime::Interface::Color::YELLOW);
	inf.print(" Dinosaurs are awesome!\n\n");
	inf.setConsoleColor(Lime::Interface::Color::WHITE);
	inf.print("When unpacking. you can use category and keys to reach binary data.\n");
	inf.print("Note that Lime datafiles do not contain any information about the\n");
	inf.print("type of data stored inside - you will have to rely on some sort of\n");
	inf.print("structure to make data types clear.\n\n");
	inf.print("When you have the manifest file, you can use the utility like this:\n\n");
	inf.setConsoleColor(Lime::Interface::Color::YELLOW);
	inf.print("  %s resources.manifest datafile.dat\n\n", executableName);
	inf.setConsoleColor(Lime::Interface::Color::WHITE);
	inf.print("If the manifest file changes often, it is recommended to create a\n");
	inf.print("%s script to simplify your development flow.\n", scriptEnv);
}

int main(int argc, char* argv[])
{
	const char* const executableName = argv[0];
	Lime::Interface inf;

	if (argc == 2 && !strcmp(argv[1], "--help"))
	{
		printHeader(inf);
		printUsage(executableName, inf);
		printHelp(executableName, inf);
	}
	else if (argc != 3)
	{
		printHeader(inf);
		printUsage(executableName, inf);
		inf.setConsoleColor(Lime::Interface::Color::WHITE);
		inf.print("\nUse ");
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
