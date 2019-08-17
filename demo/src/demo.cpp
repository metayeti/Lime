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

 #include "demo.h"

void Demo::CreateApplicationWindow()
{
	if (window)
	{
		return;
	}
	window = new sf::RenderWindow(sf::VideoMode(window_width, window_height), "LimePack Demo", sf::Style::Close);
	window->setFramerateLimit(60u);
	sf::VideoMode videoMode = sf::VideoMode::getDesktopMode();
	window->setPosition({
		static_cast<int>((videoMode.width - window_width) / 2),
		static_cast<int>((videoMode.height - window_height) / 2)
	});
}

void Demo::LoadTexture(sf::Texture& texture, Unlime::T_Bytes const& data)
{
	texture.loadFromMemory(data.data(), data.size());	
}

void Demo::Init()
{
	// setup unlime

	// there is a number of options we can set before we begin unliming data

	// integrityCheck performs the checksum test
	// set to false to skip (skips automatically for datafiles packed with -chksum=none)
	// default is true
	unlime.options.integrityCheck = true;

	// checkHeadString makes unlime throw an exception if options.headString does not match
	// the head string in the datafile
	// default is false
	unlime.options.checkHeadString = true;

	// headString sets the string to be compared against when checkHeadString is set to true
	// default is empty string
	unlime.options.headString = "Lime Demo";

	// we call readDict here to extract the dictionary from the datafile
	// this is not strictly necessary - if ommitted, the dictionary will be read on the first call to get()
	// throws an runtime_error if datafile can't be read
	unlime.readDict();

	// create the demo window
	CreateApplicationWindow();

	//LoadTexture(sprite1, unlime.get("a", "B"));
}

void Demo::Run()
{
	while (window->isOpen())
	{
		// process events
		sf::Event e;
		while (window->pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
			{
				window->close();
				break;
			}
		}
		// draw to screen
		window->display();
	}
}

void Demo::Unload()
{
	if (window)
	{
		delete window;
		window = nullptr;
	}
}