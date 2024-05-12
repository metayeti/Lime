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

#include <cmath>
#include "demo.h"

void Demo::CreateApplicationWindow()
{
	const sf::VideoMode videoMode = sf::VideoMode::getDesktopMode();
	window = std::make_unique<sf::RenderWindow>(sf::VideoMode(window_width, window_height), "LimePack Demo", sf::Style::Close);
	const int window_x = (videoMode.width - window_width) / 2;
	const int window_y = (videoMode.height - window_height) / 2;
	window->setPosition({ window_x, window_y });
	window->setFramerateLimit(60u);
	window->setIcon(40, 40, imgIcon.getPixelsPtr());
}

void Demo::PrepareDemo()
{
	// Prepare all on-screen objects.
	txtTitle.setFont(font);
	txtTitle.setString(metaName + " v" + metaVersion);
	txtTitle.setPosition(8, 5);
	txtTitle.setFillColor(sf::Color::Black);
	txtTitle.setCharacterSize(14);

	sprBackground.setTexture(texBackground);
	sprBackground.setScale(4.f, 4.f);

	sprFlag.setTexture(texFlag);
	sprFlag.setScale(4.f, 4.f);
	sprFlag.setPosition({ 140.f, 164.f });
	sprFlag.setTextureRect({ 0, 0, 18, 18 });

	sprCloud.setTexture(texCloud);
	sprCloud.setScale(4.f, 4.f);
	sprCloud.setPosition({ cloudX, 55.f });

	music.setLoop(true);
	music.play();
}

void Demo::ExtractData()
{
	// (This function is called by the Init function below)
	// To extract data, we create an Extractor using the unlime object for our context.

	// This opens the datafile.
	Unlime::Extractor ex(*unlime);

	// Now we can seamlessly extract any data we require with ex.get().
	// Note that ex.get() will throw in case the file can't be opened or
	// data corruption is detected. Return value is a bool which is true
	// if the item was found in the dictionary or false otherwise.

	// On the very first call to ex.get(), datafile will be verified and
	// the dictionary will be extracted. Every subsequent call to ex.get()
	// will use the dictionary that was extracted on the first get, even if
	// we create a new Extractor later in the future.

	// LoadResource and LoadString are helper functions to make extraction of
	// SFML and std::string objects from the datafile easier. These functions
	// call ex.get() with the provided resource category and key.

	// Fetch some strings from meta category.
	LoadString(metaName, ex, "meta", "name");
	LoadString(metaVersion, ex, "meta", "version");

	// Here we fetch our font.
	// We need font data to remain in memory the whole time the application remains open
	// so we extract fontData manually (as opposed to calling LoadResource which frees byte
	// data after it's done).
	if (ex.get(fontData, "fonts", "Lato"))
	{
		font.loadFromMemory(fontData.data(), fontData.size());
	}

	// Retreive the window icon.
	LoadResource(imgIcon, ex, "graphics", "icon");

	// Now let's acquire data for our textures.
	LoadResource(texBackground, ex, "graphics", "background");
	LoadResource(texFlag, ex, "graphics", "flag");
	LoadResource(texCloud, ex, "graphics", "cloud");

	// Retreive music data.
	// Same as with the font, we need this data to remain in memory.
	if (ex.get(musicData, "music", "demo"))
	{
		music.openFromMemory(musicData.data(), musicData.size());
	}

	// The datafile is closed when ex goes out of scope. In the odd case we have multiple
	// extractor objects, the datafile is closed when last extractor goes out of scope.
}

void Demo::Init()
{
	// Before we create the Unlime object, we may want to set some options.
	// This is an optional step, it is only required if you wish to perform datafile
	// identification via the head string or if you wish to skip integrity checking.
	Unlime::Options options;

	// integrityCheck performs the checksum test when reading data. An exception will
	// be thrown in case data corruption is detected. When set to true, the dictionary's
	// and each individual resource's checksum will be tested.
	// Set false to skip (skips automatically for datafiles packed with -chksum=none).
	// (The only reason to skip this would be to provide a very marginal speed increase
	// at the cost of reliability.)
	// Default is true.
	options.integrityCheck = true;

	// checkHeadString makes unlime throw an exception if options.headString does not match
	// the head string defined in the datafile. This is useful if you want to make sure you
	// are dealing with the correct datafile.
	// Default is false.
	options.checkHeadString = true;

	// headString sets the string to be compared against when checkHeadString is set to true.
	// This only comes into effect on the very first Extractor::get() call, when the datafile
	// format is being verified and the dictionary is extracted.
	// Default is an empty string.
	options.headString = "Lime Demo";

	// Create the Unlime object and associate it with the demo datafile filename.
	// The datafile is not open yet at this point, we are only setting up the object and
	// associating it with the filename.
	// The second parameter is optional (when omitted, default options will be used).
	// If using unlime_phony, we are associating the object with the resource manifest
	// filename instead and options are ignored.
	unlime = std::make_unique<Unlime>(datafileFilename, options);

	// We can now proceed to extract data from the datafile.
	ExtractData();

	// If we REALLY wanted to, we could drop the datafile dictionary from memory at this point.
	// Only do this if you don't want to query data from the datafile again, or at least don't
	// want to query it again for a very long time. An even better idea would be to let Unlime
	// go out of scope and let the destructor handle cleanup automatically.

	// In the vast majority of use cases, the memory footprint of the dict will be negligible.

	// If this function is called, the dictionary and all data related to the datafile is dropped.
	// If Extractor::get() is called, the datafile will be verified again and the the dictionary
	// will be read (again).

	// Commented out on purpose.
	//unlime->dropDict();

	// Create the sprites for our demo.
	PrepareDemo();

	// Create the demo window.
	CreateApplicationWindow();
}

void Demo::Run()
{
	while (window->isOpen())
	{
		// Process events.
		sf::Event e;
		while (window->pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
			{
				window->close();
				break;
			}
		}

		// Flap the flag.
		flagTimer += 0.1f;
		if (flagTimer >= 1.f)
		{
			flagTimer = 0.f;
			if (++flagTile >= 3)
			{
				flagTile = 0;
			}
			sprFlag.setTextureRect({ 18 * flagTile, 0, 18, 18 });
		}

		// Move the cloud.
		cloudX -= 0.1f;
		if (cloudX < -168.f)
		{
			cloudX = 640.f;
		}
		sprCloud.setPosition(cloudX, 55.f);

		// Draw everything to screen.
		window->draw(sprBackground);
		window->draw(sprFlag);
		window->draw(sprCloud);
		window->draw(txtTitle);
		window->display();
	}
}