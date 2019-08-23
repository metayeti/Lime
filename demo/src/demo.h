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
#include "unlime.h"

class Demo
{
private:
	const unsigned int window_width = 640;
	const unsigned int window_height = 480;
#if defined(_WIN32)
	const std::string datafileFilename = "../../../datafile/demo.dat";
#else
	const std::string datafileFilename = "../datafile/demo.dat";
#endif

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
	std::string metaImportantInfo;

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

	static void LoadResource(std::string& strObject, Unlime::Extractor const& ex, std::string const& resCategory, std::string const& resKey)
	{
		// same as above but for std::string
		Unlime::T_Bytes data;
		if (ex.get(data, resCategory, resKey))
		{
			strObject = std::string(reinterpret_cast<char*>(data.data()), data.size());
		}
	}

	void ExtractData();

public:
	void Init();
	void Run();
};

#endif // LIME_DEMO_H
