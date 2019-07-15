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
  *  lime.cpp
  *  Program entry point.
  *
  */

#include <string>
#include <vector>
#include <algorithm>
#include "interface.h"
#include "dict.h"
#include "pack.h"

const char* const LIME_VERSION = "0.2.0";
const char* const LIME_COPYRIGHT_YEAR = "2019";
const char* const LIME_COPYRIGHT_AUTHOR = "Danijel Durakovic";

#if defined(_WIN32)
const char* const PATH_SEPARATOR = "\\";
#else
const char* const PATH_SEPARATOR = "/";
#endif

void printHeader(Lime::Interface& inf, std::string const& execName)
{
	inf
		<< "\n"
		<< Lime::Interface::Color::BRIGHTGREEN
		<< " -----| Lime "
		<< Lime::Interface::Color::BRIGHTWHITE
		<< LIME_VERSION
		<< Lime::Interface::Color::BRIGHTGREEN
		<< " |-----\n"
		<< Lime::Interface::Color::GRAY
		<< "   Game datafile packer\n"
		<< "(c) " << LIME_COPYRIGHT_YEAR << " " << LIME_COPYRIGHT_AUTHOR
		<< "\n\n"
		<< Lime::Interface::Color::DEFAULT
		<< "Usage:\n\n"
		<< Lime::Interface::Color::BRIGHTWHITE
		<< "  " << execName << " ["
		<< Lime::Interface::Color::BRIGHTGREEN
		<< "resource manifest file"
		<< Lime::Interface::Color::BRIGHTWHITE
		<< "] ["
		<< Lime::Interface::Color::BRIGHTGREEN
		<< "output file"
		<< Lime::Interface::Color::BRIGHTWHITE
		<< "]\n\n";
}

void printHelp(Lime::Interface& inf, std::string const& execName)
{
	inf
		<< Lime::Interface::Color::DEFAULT
		<< "Use this utility to pack your Lime datafiles.\n\n"
		<< "To pack a datafile, you will first need to create a resource manifest.\n"
		<< "The resource manifest is an INI-formatted file with the following syntax:\n\n"
		<< Lime::Interface::Color::GRAY
		<< "  ; comment\n"
		<< Lime::Interface::Color::BRIGHTRED
		<< "  ["
		<< Lime::Interface::Color::BRIGHTWHITE
		<< "category"
		<< Lime::Interface::Color::BRIGHTRED
		<< "]\n"
		<< Lime::Interface::Color::BRIGHTWHITE
		<< "  key "
		<< Lime::Interface::Color::BRIGHTGREEN
		<< "="
		<< Lime::Interface::Color::BRIGHTWHITE
		<< " value\n\n"
		<< Lime::Interface::Color::DEFAULT
		<< "An example resource manifest entry can look like this:\n\n"
		<< Lime::Interface::Color::GRAY
		<< "  ; graphics for my awesome game\n"
		<< Lime::Interface::Color::BRIGHTRED
		<< "  ["
		<< Lime::Interface::Color::BRIGHTWHITE
		<< "graphics"
		<< Lime::Interface::Color::BRIGHTRED
		<< "]\n"
		<< Lime::Interface::Color::BRIGHTWHITE
		<< "  sprite1 "
		<< Lime::Interface::Color::BRIGHTGREEN
		<< "="
		<< Lime::Interface::Color::BRIGHTWHITE
		<< " graphics" << PATH_SEPARATOR << "sprite1.png\n"
		<< "  sprite2 "
		<< Lime::Interface::Color::BRIGHTGREEN
		<< "="
		<< Lime::Interface::Color::BRIGHTWHITE
		<< " graphics" << PATH_SEPARATOR << "sprite2.png\n\n"
		<< Lime::Interface::Color::DEFAULT
		<< "Lime interprets every value as a file containing data to be packed. Note\n"
		<< "that filenames are lost in the process. You will be able to access data\n"
		<< "based on the category and key provided.\n\n"
		<< "Note also that a Lime datafile does not contain any information about the\n"
		<< "type of data stored in it.\n\n"
		<< "It is recommended that you create a structure where categories reflect\n"
		<< "the type of data contained in them - so for example everything in the\n"
		<< Lime::Interface::Color::BRIGHTWHITE
		<< "graphics"
		<< Lime::Interface::Color::DEFAULT
		<< " category will be an image of some kind. If you need further\n"
		<< "type information, you can use a category name like "
		<< Lime::Interface::Color::BRIGHTWHITE
		<< "graphics:png"
		<< Lime::Interface::Color::DEFAULT
		<< " to\n"
		<< "be even more specific or use a naming scheme that makes sense to you.\n\n"
		<< "You can also add meta-categories to the resource manifest by prefixing\n"
		<< "the category name with "
		<< Lime::Interface::Color::BRIGHTCYAN
		<< "@"
		<< Lime::Interface::Color::DEFAULT
		<< ". In this case, all values in the category will\n"
		<< "be stored directly:\n\n"
		<< Lime::Interface::Color::BRIGHTRED
		<< "  ["
		<< Lime::Interface::Color::BRIGHTCYAN
		<< "@"
		<< Lime::Interface::Color::BRIGHTWHITE
		<< "category"
		<< Lime::Interface::Color::BRIGHTRED
		<< "]\n"
		<< Lime::Interface::Color::BRIGHTWHITE
		<< "  important info "
		<< Lime::Interface::Color::BRIGHTGREEN
		<< "="
		<< Lime::Interface::Color::BRIGHTWHITE
		<< " Dinosaurs are awesome!\n\n"
		<< Lime::Interface::Color::DEFAULT
		<< "When you have the resource manifest file ready, you can use the utility:\n\n"
		<< Lime::Interface::Color::BRIGHTWHITE
		<< "  " << execName << " resources.manifest datafile.dat\n\n"
		<< Lime::Interface::Color::DEFAULT
		<< "This will generate a "
		<< Lime::Interface::Color::BRIGHTWHITE
		<< "datafile.dat"
		<< Lime::Interface::Color::DEFAULT
		<< " based on resources listed in the\n"
		<< Lime::Interface::Color::BRIGHTWHITE
		<< "resources.manifest"
		<< Lime::Interface::Color::DEFAULT
		<< " file. If the resource manifest changes often, it is\n"
		<< "recommended to create a batch script to simplify your development flow.	\n\n"
		<< "Typically, such a script would call this utility and then copy the\n"
		<< "datafile to where it is needed. When an error level 1 is raised, the\n"
		<< "utility was unable to produce a datafile and an error message will be\n"
		<< "displayed.\n";
}

std::string stripFilenamePathExt(const char* const fullPathFilename)
{
	// strips a filename path and extension and makes the output lowercase
	std::string filename(fullPathFilename);
	const std::size_t lastSlashPos = filename.find_last_of("\\/");
	if (lastSlashPos != std::string::npos)
	{
		filename.erase(0, lastSlashPos + 1);
	}
	const std::size_t periodPos = filename.rfind('.');
	if (periodPos != std::string::npos)
	{
		filename.erase(periodPos);
	}
	std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
	return filename;
}

int main(int argc, char* argv[])
{
	std::vector<std::string> args;
	if (argc > 1)
	{
		args.assign(argv + 1, argv + argc);
	}

	std::size_t n_args = args.size();
	const std::string execName = stripFilenamePathExt(argv[0]);

	Lime::Interface inf;

	printHeader(inf, execName);

	if (n_args >= 1 && (args[0] == "--help" || args[0] == "-h"))
	{
		if (n_args == 1)
		{
			printHelp(inf, execName);
		}
		else
		{
		}
	}
	else if (n_args == 2)
	{
		std::string const& resourceManifestFilename = args[0];
		std::string const& outputFilename = args[1];
	}
	else
	{
		inf
			<< Lime::Interface::Color::DEFAULT
			<< "Use "
			<< Lime::Interface::Color::BRIGHTWHITE
			<< execName << " --help"
			<< Lime::Interface::Color::DEFAULT
			<< " for more information.\n";
	}

	inf << Lime::Interface::Color::DEFAULT;

#if !defined(_WIN32)
	inf << "\n";
#endif

	return 0;
}
