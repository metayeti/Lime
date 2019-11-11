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
#include <stdexcept>
#include <cstdlib>
#include "interface.h"
#include "dict.h"
#include "pack.h"
#include "const.h"

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
		<< Lime::LIME_VERSION
		<< Lime::Interface::Color::BRIGHTGREEN
		<< " |-----\n"
		<< Lime::Interface::Color::GRAY
		<< "   Game datafile packer\n"
		<< "(c) " << Lime::LIME_COPYRIGHT_YEAR << " " << Lime::LIME_COPYRIGHT_AUTHOR
		<< Lime::Interface::Color::DEFAULT
		<< "\n\n";
}

void printUsage(Lime::Interface& inf, std::string const& execName)
{
	inf
		<< "Usage:\n\n"
		<< "  " << execName << " {options...} [resource manifest file] [output file]\n\n";
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
				<< "  -chksum=[adler32|crc32|none] (default: adler32)\n"
				<< "    Selects the checksum algorithm to use for data integrity check.\n\n"
				<< "  -head=[\"string\"] (default: none)\n"
				<< "    Head string used for datafile identification.\n\n"
				<< "  -h [topic]\n"
				<< "    Show help for given topic.\n\n"
				<< "Help topics: basic, examples, structure, manifest, clevel, chksum, head\n";
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
					<< "Pack a datafile with a predefined head string:\n\n"
					<< "  " << execName << " -head=\"my project\" resources.manifest example.dat\n\n"
					<< "Use multiple options:\n\n"
					<< "  " << execName << " -clevel=5 -head=\"my project\" resources.manifest example.dat\n\n"
					<< "Note: when options are left unspecified, default values will be used.\n";
			}
			else if (helpTopic == "structure") {
				inf
					<< "Lime datafile structure:\n\n"
					<< "           Z1    ...   Zn    Zdict\n"
					<< "          [~~~] [~~~] [~~~] [~~~~~~~~~~]       (zipped content)\n\n"
					<< "   header   user resources   dictionary   end\n"
					<< " |________|________________|____________|_____|\n\n\n"
					<< "   Header:\n\n"
					<< "   bgn   revision-  head*  dict size   dict checksum\n"
					<< " |_____|__________|______|___________|...............|\n\n\n"
					<< "   Dictionary:\n\n"
					<< "   N   category 1   ...   category N\n"
					<< " |___|____________|     |____________|\n"
					<< "            |\n"
					<< "            |\n"
					<< "            |\n"
					<< "         Category:\n\n"
					<< "         category key*  M   data 1   ...   data M\n"
					<< "       |______________|___|________|     |________|\n"
					<< "                              |\n"
					<< "                              |\n"
					<< "                              |\n"
					<< "                            Data:\n\n"
					<< "                            data key*  seek_id+  size+  checksum\n"
					<< "                          |__________|_________|______|..........|\n\n\n"
					<< "All non-resource strings* are stored in the following manner:\n\n"
					<< "   length-  string\n"
					<< " |________|________|\n\n"
					<< "Numeric values are stored as 32-bit unsigned integers.\n"
					<< "Numeric values marked + are stored as 64-bit unsigned integers.\n"
					<< "Numeric values marked - are stored as 8-bit unsigned integers.\n";
			}
			else if (helpTopic == "manifest") {
				inf
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
					<< "Note also that a Lime datafile does not contain any information about the\n"
					<< "type of data stored inside.\n\n"
					<< "It is recommended that you create a structure where categories make the\n"
					<< "type of data contained in them implicit - for example, the \"graphics\"\n"
					<< "category will store only image data, and so on.\n\n"
					<< "Category and key names are stripped of leading and trailing whitespace\n"
					<< "and are case sensitive. They can contain spaces and other symbols.\n\n"
					<< "You can also add meta-categories to the resource manifest by prefixing\n"
					<< "the category name with @. In this case, all values in the category will\n"
					<< "be stored directly:\n\n"
					<< "  [@metadata]\n"
					<< "  important info = Giraffes are awesome!\n";
			}
			else if (helpTopic == "clevel") {
				inf
					<< "The clevel option is used to select the level of compression. Higher levels\n"
					<< "compress more but (de)compression takes more CPU time, so it is essentially\n"
					<< "a tradeoff between time and file size. To disable compression altogether,\n"
					<< "set clevel to 0. Default level is 9 which is the highest compression level.\n\n"
					<< "Usage: -clevel=[0..9]\n\n"
					<< "Examples:\n\n"
					<< "Pack a datafile without compressing data:\n\n"
					<< "  " << execName << " -clevel=0 resources.manifest example.dat\n\n"
					<< "Pack a datafile using compression level 5:\n\n"
					<< "  " << execName << " -clevel=5 resources.manifest example.dat\n";
			}
			else if (helpTopic == "chksum") {
				inf
					<< "The chksum option selects the checksum algorithm. Available options are:\n\n"
					<< "  Adler32 (default)\n"
					<< "  CRC32\n"
					<< "  None\n\n"
					<< "A checksum is attached to each user resource and is used for data integrity\n"
					<< "check. Adler32 is faster and slightly less reliable than CRC32.\n\n"
					<< "The type of the checksum function is implicitly defined by the bgn and end\n"
					<< "endpoints in the Lime datafile. Adler32 will use LM> and <LM, CRC32 will\n"
					<< "use LM] and [LM, and a file with no checksums will use LM) and (LM.\n\n"
					<< "Regardless of the checksum function used (or not used), you can skip data\n"
					<< "integrity check when unpacking the file if you so desire.\n\n"
					<< "Usage: -chksum=[adler32|crc32|none]\n\n"
					<< "Examples:\n\n"
					<< "Pack a datafile using the CRC32 algorithm for checksums:\n"
					<< "  " << execName << " -chksum=crc32 resources.manifest example.dat\n\n"
					<< "Pack a datafile without writing checksums:\n"
					<< "  " << execName << " -chksum=none resources.manifest example.dat\n";
			}
			else if (helpTopic == "head") {
				inf
					<< "Head is a custom string that can be used to identify the datafile.\n\n"
					<< "Usage: -head=[\"string\"]\n\n"
					<< "Examples:\n\n"
					<< "Pack a datafile with a simple head string:\n"
					<< "  " << execName << " -head=myproject resources.manifest example.dat\n\n"
					<< "Pack a datafile using a head string with several spaces:\n"
					<< "  " << execName << " -head=\"string of custom length\" resources.manifest example.dat\n";
			}
			else {
				inf << "Unknown help topic: " << helpTopic << "\n";
			}
		}
	}
	else if (freeParams.size() >= 2u) {

		std::string const& resourceManifestFilename = freeParams[0];
		std::string const& outputFilename = freeParams[1];

		try {

			printHeader(inf);

			// prepare options
			Lime::PackOptions options;

			for (auto const& prop : optionParams) {
				std::string const& propName = prop.first;
				std::string propValue = prop.second;
				if (propName == "clevel") {
					options.clevel = static_cast<unsigned char>(std::stoul(propValue));
				}
				else if (propName == "chksum") {
					std::transform(propValue.begin(), propValue.end(), propValue.begin(), ::tolower);
					if (propValue == "adler32") {
						options.chksum = Lime::ChkSumOption::ADLER32;
					}
					else if (propValue == "crc32") {
						options.chksum = Lime::ChkSumOption::CRC32;
					}
					else if (propValue == "none" || propValue == "no") {
						options.chksum = Lime::ChkSumOption::NONE;
					}
				}
				else if (propName == "head") {
					options.headstr = propValue;
				}
			}

			inf << "Reading resource manifest ... ";

			// read dictionary definitions from the resource manifest
			Lime::Dict dict = Lime::readDictFromFile(resourceManifestFilename);

			// successfully read resource manifest
			inf.ok() << "\n\n";

			// pack datafile
			Lime::pack(inf, dict, outputFilename, options);
		}
		catch (std::runtime_error& e) {
			inf.error(e.what()) << "\n";
		}
	}
	else
	{
		printHeader(inf);
		printUsage(inf, execName);
		inf << "Use " << execName << " --help (or " << execName << " -h) for more information.\n";
	}

	inf << Lime::Interface::Color::DEFAULT;

#if !defined(_WIN32)
	inf << "\n";
#endif

	return 0;
}
