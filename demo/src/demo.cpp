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
	const sf::VideoMode videoMode = sf::VideoMode::getDesktopMode();
	const int window_x = (videoMode.width - window_width) / 2;
	const int window_y = (videoMode.height - window_height) / 2;
	window->setPosition({ window_x, window_y });
}

void Demo::LoadTexture(sf::Texture& texture, Unlime::T_Bytes const& data)
{
	texture.loadFromMemory(data.data(), data.size());	
}

void Demo::ExtractData()
{
	// To extract data, first we create an Extractor object.
	// This opens the datafile.
	Unlime::Extractor ex(*unlime);

	// Now we can seamlessly extract any data we require with ex.get().
	// Here we acquire some data for our textures.
	LoadTexture(texSprite1, ex.get("graphics", "sprite1"));
	LoadTexture(texSprite2, ex.get("graphics", "sprite2"));

	// The datafile is closed when ex goes out of scope.
}

void Demo::PrepareSprites()
{
	sprite1.setTexture(texSprite1);
	sprite1.setTextureRect({ 0, 0, 40, 40 });
	sprite1.setPosition({ 0.f, 0.f });

	sprite2.setTexture(texSprite2);
	sprite2.setTextureRect({ 0, 0, 80, 80 });
	sprite2.setPosition({ 150.f, 100.f });
}

void Demo::Init()
{
	// Create the unlime object on our demo datafile.
	// The datafile is not open yet, we are only setting up the object and
	// associating it with the filename.
	unlime = std::make_unique<Unlime>(datafileFilename);

	// Before we start extracting, we may want to set some options.
	// This is an optional step, it is only required if you wish to perform datafile
	// identification via the head string or if you wish to skip integrity checking.

	// integrityCheck performs the checksum test when reading data. An exception will
	// be thrown in case data corruption is detected.
	// Set false to skip (skips automatically for datafiles packed with -chksum=none).
	// Default is true.
	unlime->options.integrityCheck = true;

	// checkHeadString makes unlime throw an exception if options.headString does not match.
	// The head string defined in the datafile.
	// Default is false.
	unlime->options.checkHeadString = true;

	// headString sets the string to be compared against when checkHeadString is set to true.
	// This only comes into effect on the very first get() call, when the datafile format is
	// being verified and the dictionary is extracted.
	// Default is an empty string.
	unlime->options.headString = "Lime Demo";

	// We can now proceed to extract data from the datafile.
	ExtractData();

	// Create the sprites for our demo.


	// Create the demo window.
	CreateApplicationWindow();
}

void Demo::Run()
{
	while (window->isOpen())
	{
		// Process events
		sf::Event e;
		while (window->pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
			{
				window->close();
				break;
			}
		}
		// Draw to screen
		window->draw(sprite1);
		window->draw(sprite2);
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