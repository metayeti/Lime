/*
 * Copyright (c) 2024 Danijel Durakovic
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
  *  LimePack Demo 
  *
  *  demo.h
  *  Demo application class.
  * 
  */

#pragma once

#ifndef LIME_DEMO_H_
#define LIME_DEMO_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <stdexcept>
#include <memory>

// Include the unlime header.
#include "unlime.h"

// Alternatively, we can include the unlime_phony header instead.
// unlime_phony replicates the API of unlime but it operates on resource manifests and
// associated resource files instead of datafiles. Using this essentially skips the datafile
// part and is highly recommended during development to avoid packing your data over and over
// again between changes. The idea is that the Unlime API stays the same in your code whether
// working with datafiles or directly with resource manifests. When deploying, simply use the
// real unlime header instead and pack your datafile. Both headers should work seamlessly, you
// only need to associate it with the correct datafile filename or the resource manifest
// filename. To aid with this, unlime_phony defines UNLIME_PHONY which the precompiler can check
// against (see below). Unlime constructor options are ignored when using unlime_phony.

// In case you are dealing with multiple datafiles, you can't combine phony and real unlime
// usage without considerable trouble. It is recommended you simply open all datafiles in phony
// mode while your project is being developed. This shouldn't be a problem as long as you setup
// your code correctly (see this demo for an example that you can build upon).

// Commented out on purpose.
//#include "unlime_phony.h"

class Demo
{
private:
#if defined(UNLIME_PHONY)
	// resource manifest filename (used when unlime_phony.h is included)
	#if defined(_WIN32)
		const std::string datafileFilename = "../../../datafile/resources.manifest";
	#else
		const std::string datafileFilename = "../datafile/resources.manifest";
	#endif
#else
	// datafile filename (used when unlime.h is included)
	#if defined(_WIN32)
		const std::string datafileFilename = "../../../datafile/demo.dat";
	#else
		const std::string datafileFilename = "../datafile/demo.dat";
	#endif
#endif

	const unsigned int window_width = 640;
	const unsigned int window_height = 480;

	// pointer to SFML window
	std::unique_ptr<sf::RenderWindow> window;

	// pointer to Unlime
	std::unique_ptr<Unlime> unlime;

	// images
	sf::Image imgIcon;

	// textures
	sf::Texture texBackground;
	sf::Texture texFlag;
	sf::Texture texCloud;

	// font
	Unlime::T_Bytes fontData; // SFML requires font data to remain in memory
	sf::Font font;

	// music loop
	Unlime::T_Bytes musicData; // SFML requires music data to remain in memory
	sf::Music music;

	// string data from "meta" section in the datafile
	std::string metaName;
	std::string metaVersion;

	// on-screen objects
	sf::Sprite sprBackground;
	sf::Sprite sprFlag;
	sf::Sprite sprCloud;
	sf::Text txtTitle;

	// animation variables
	float cloudX = 400.f;
	float flagTimer = 0.f;
	char flagTile = 0;

	void CreateApplicationWindow();

	void PrepareDemo();

	template<class T>
	static void LoadResource(T& sfmlObject, Unlime::Extractor const& ex, std::string const& resCategory, std::string const& resKey)
	{
		// load a SFML object using an Extractor to retreive data from the datafile
		Unlime::T_Bytes data;
		if (ex.get(data, resCategory, resKey))
		{
			sfmlObject.loadFromMemory(data.data(), data.size());
		}
	}

	static void LoadString(std::string& strObject, Unlime::Extractor const& ex, std::string const& resCategory, std::string const& resKey)
	{
		// same as above but for std::string
		Unlime::T_Bytes data;
		if (ex.get(data, resCategory, resKey))
		{
			strObject.assign(data.begin(), data.end());
		}
	}

	void ExtractData();

public:
	void Init();
	void Run();
};

#endif // LIME_DEMO_H
