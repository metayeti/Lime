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

	// demo textures
	sf::Texture texSprite1;
	sf::Texture texSprite2;

	// demo font
	Unlime::T_Bytes fontData; // SFML requires font data to remain in memory
	sf::Font font;

	// on-screen objects

	sf::Text text1;

	sf::Sprite sprite1;
	sf::Sprite sprite2;

	void CreateApplicationWindow();

	void PrepareDemo();

	static void LoadFont(sf::Font& font, Unlime::T_Bytes const& data);
	static void LoadTexture(sf::Texture& texture, Unlime::T_Bytes const& data);

	void ExtractData();

public:
	void Init();
	void Run();
};

#endif // LIME_DEMO_H