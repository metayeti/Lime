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
#include <utility>
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

void printHeader(Lime::Interface& inf)
{
	inf
		<< Lime::Interface::Color::BRIGHTGREEN
		<< " -----| Lime "
		<< Lime::Interface::Color::BRIGHTWHITE
		<< LIME_VERSION
		<< Lime::Interface::Color::BRIGHTGREEN
		<< " |-----\n"
		<< Lime::Interface::Color::GRAY
		<< "   Game datafile packer\n"
		<< "(c) " << LIME_COPYRIGHT_YEAR << " " << LIME_COPYRIGHT_AUTHOR
		<< Lime::Interface::Color::DEFAULT
		<< "\n\n";
}

void printUsage(Lime::Interface& inf, std::string const& execName)
{
	inf
		<< "Usage:\n\n"
		<< "  " << execName << " {options...} [resource manifest file] [output file]\n\n";
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
	const size_t lastSlashPos = filename.find_last_of("\\/");
	if (lastSlashPos != std::string::npos)
	{
		filename.erase(0, lastSlashPos + 1);
	}
	const size_t periodPos = filename.rfind('.');
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

	const size_t n_args = args.size();
	const std::string execName = stripFilenamePathExt(argv[0]);

	Lime::Interface inf;

	// parse arguments

	std::vector<std::string> freeParams; // filenames
	std::vector<std::pair<std::string, std::string>> optionParams;

	for (auto const& arg : args)
	{
		size_t argLength = arg.size();
		if (argLength >= 2 && arg[0] == '-')
		{
			// parameter is an option
			const std::string option = arg.substr(1);
			const size_t equalsPos = option.find_first_of('=');
			if (equalsPos != std::string::npos)
			{
				std::string optionKey = option.substr(0, equalsPos);
				std::transform(optionKey.begin(), optionKey.end(), optionKey.begin(), ::tolower);
				const std::string optionValue = option.substr(equalsPos + 1);
				optionParams.push_back({ optionKey, optionValue });
			}
		}
		else
		{
			// add to free parameters
			freeParams.push_back(arg);
		}
	}

	inf << "\n";

	if (n_args >= 1 && (args[0] == "--help" || args[0] == "-h"))
	{
		if (n_args == 1)
		{
			inf << "Use this utility to pack your Lime datafiles.\n\n";
			printUsage(inf, execName);
			inf
				<< "Options:\n\n"
				<< "  -clevel=[0..9] (default: 9)\n"
				<< "    Compression level. 0 is no compression, 9 is highest compression.\n\n"
				<< "  -head=[\"string\"] (default: none)\n"
				<< "    Head string used for datafile identification.\n\n"
				<< "  -padding=[number] (default: 0)\n"
				<< "    Amount of random bytes to add between data.\n\n"
				<< "  -h [topic]\n"
				<< "    Show help for given topic.\n\n"
				<< "Help topics: basic, examples, structure, manifest, clevel, head, padding\n";
		}
		else
		{
			//printUsage(inf, execName);
			const std::string helpTopic(args[1]);
			if (helpTopic == "basic") {
				inf
					<< "Use this utility to pack your Lime datafiles.\n\n"
					<< "Basic syntax (with all options set to default) is as follows:\n\n"
					<< "  " << execName << " [resource manifest file] [output file]\n\n"
					<< "The resource manifest file is an INI-formatted file that lists assets\n"
					<< "to be packed in the datafile.\n\n"
					<< "Use " << execName << " -h manifest to learn more about the resource manifest file.\n\n"
					<< "Use " << execName << " -h examples to see more usage examples.\n";
			}
			else if (helpTopic == "examples") {
				inf
					<< "Listed below are some common usage examples.\n\n"
					<< "Pack resources listed in resources.manifest into example.dat:\n\n"
					<< "  " << execName << " resources.manifest example.dat\n\n"
					<< "Pack a datafile without compressing data:\n\n"
					<< "  " << execName << " -clevel=0 resources.manifest example.dat\n\n"
					<< "Pack a datafile with a predefined header string:\n\n"
					<< "  " << execName << " -head=\"my project\" resources.manifest example.dat\n\n"
					<< "Use multiple options:\n\n"
					<< "  " << execName << " -clevel=5 -head=\"my project\" resources.manifest example.dat\n\n"
					<< "Note: when options are left unspecified, default values will be used.\n";
			}
			else if (helpTopic == "structure") {
				inf <<
					"Datafile structure:\n\n"
					"         Z0           Z1    ...   Zn\n"
					"        [~~~~~~~~~~] [~~~] [~~~] [~~~]      (zipped content)\n\n"
					"  head   dictionary   user resources   checksum\n"
					"|______|____________|________________|__________|\n"
					"             |\n"
					"             |\n"
					"             |\n"
					"             |\n"
					"         dictionary:\n\n"
					"         N   section 1   ...   section N\n"
					"       |___|___________|     |___________|\n"
					"                 |\n"
					"                 |\n"
					"                 |\n"
					"                 |\n"
					"              category:\n\n"
					"              category key   N   data 1   ...   data N\n"
					"            |______________|___|________|     |________|\n"
					"                                   |\n"
					"                                   |\n"
					"                                   |\n"
					"                                 data:\n\n"
					"                                 data key   seek_id   size\n"
					"                               |__________|_________|______|\n\n\n"
					"All non-resource strings are stored in the following manner:\n\n"
					"  length   string\n"
					"|________|________|\n";
			}
			else if (helpTopic == "manifest") {
				inf
					<< "To pack a datafile, you will first need to create a resource manifest.\n"
					<< "The resource manifest is an INI-formatted file with the following syntax:\n\n"
					<< "  ; comment\n"
					<< "  [category]\n"
					<< "  key = value\n\n"
					<< "An example resource manifest entry can look like this:\n\n"
					<< "  ; graphics assets for my project\n"
					<< "  [graphics]\n"
					<< "  sprite1 = graphics" << PATH_SEPARATOR << "sprite1.png\n"
					<< "  sprite2 = graphics" << PATH_SEPARATOR << "sprite2.png\n\n"
					<< "Lime interprets every value as a file containing data to be packed. Note\n"
					<< "that filenames are lost in the process. You will be able to access data\n"
					<< "using the category and key provided in the manifest.\n\n"
					<< "Note also that Lime does not contain any information about the\n"
					<< "type of data stored in it.\n\n"
					<< "It is recommended that you create a structure where categories reflect\n"
					<< "the type of data contained in them - so for example everything in the\n"
					<< "graphics category will be image data of some kind.\n\n"
					<< "Category and key names are stripped of leading and trailing whitespace\n"
					<< "and are case sensitive. They can contain spaces.\n\n"
					<< "You can also add meta-categories to the resource manifest by prefixing\n"
					<< "the category name with @. In this case, all values in the category will\n"
					<< "be stored directly:\n\n"
					<< "  [@metadata]\n"
					<< "  important info = Giraffes are awesome!\n";
			}
			else if (helpTopic == "clevel") {
			}
			else if (helpTopic == "head") {
			}
			else if (helpTopic == "padding") {
			}
			else {
				inf << "Unknown help topic: " << helpTopic << "\n";
			}
		}
	}
	else
	{
		printHeader(inf);
		printUsage(inf, execName);
		inf << "Use " << execName << " --help for more information.\n";
	}

	/*

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
	*/
	inf << Lime::Interface::Color::DEFAULT;

#if !defined(_WIN32)
	inf << "\n";
#endif

	return 0;
}
