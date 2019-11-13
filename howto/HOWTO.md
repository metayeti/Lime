# Lime HowTo

## Contents

- [Introduction](#introduction)
- [Step 1: Prepare assets](#step-1-prepare-assets)
- [Step 2: Create a resource manifest](#step-2-create-a-resource-manifest)
- [Step 3: Pack the datafile](#step-3-pack-the-datafile)
- [Step 4: Extract data in-game](#step-4-extract-data-in-game)
- [Phony Unlime](#phony-unlime)

## Introduction

Suppose you are working on a game, and you don't really appreciate the idea of having your game assets exposed to the outside world as plain files. First, it litters your game's folder with files, and second - it allows anyone to just go and edit those files, ruining all your hard work in a pinch! Well - this is exactly the problem Lime is trying to solve! Lime takes a list of assets you need for your game, and packs them into a single file. It then gives you the Unlime API, which you can use to extract game data at will.

The following tutorial demonstrates the basic idea of Lime. For more in-depth information on how to use the API, see the Lime Demo code.

## Step 1: Prepare assets

Let's suppose we prepared a few asset files in a folder called resources, that we want to use in our game:

![Howto1](howto1.png?raw=true)

## Step 2: Create a resource manifest

Next, we will require a resource manifest file:

![Howto2](howto2.png?raw=true)

The filename does not matter, however as convention we use the `.manifest` extension.

Inside the file, we put the following:
```INI
; All our graphics
[graphics]
house = resources/house.png
stairs = resources/stairs.png

; A meta category. note the @ prefix.
; Values in meta categories will be stored as strings
[@window]
title = Hello world!
```

## Step 3: Pack the datafile


Now we can use the Lime utility to pack our datafile. The first argument is the resource manifest filename, and the second is our output datafile filename:
```
lime resources.manifest mydata.dat
```

The output should look something like this:

![Howto3](howto3.png?raw=true)

There are a number of options to use with Lime, use `lime --help` to get more information.

## Step 4: Extract data in-game

Now we can extract the data in our game's code using the Unlime API. The following example uses SFML to create and manage a game window:

```C++
#include <SFML/Graphics.hpp>

#include "unlime.h"
// NOTE: during development, it is recommended that we use unlime_phony.h instead
//#include "unlime_phony.h"

int main()
{
	// Create the Unlime object
#if defined(UNLIME_PHONY)
	Unlime unlime("resources.manifest");
#else
	Unlime unlime("mydata.dat");
#endif

	// Create the main window
	sf::RenderWindow window(sf::VideoMode(600, 400), "SFML window");
	window.setPosition({ 300, 300 });

	// Prepare some textures
	sf::Texture texHouse, texStairs;

	// Load some data from the datafile
	{
		// Create an Extractor; this opens the datafile
		Unlime::Extractor ex(unlime);

		// Load the textures
		Unlime::T_Bytes bytesHouse;
		if (ex.get(bytesHouse, "graphics", "house")) {
			texHouse.loadFromMemory(bytesHouse.data(), bytesHouse.size());
		}
		Unlime::T_Bytes bytesStairs;
		if (ex.get(bytesStairs, "graphics", "stairs")) {
			texStairs.loadFromMemory(bytesStairs.data(), bytesStairs.size());
		}

		// Fetch and set the window title
		Unlime::T_Bytes bytesTitle;
		if (ex.get(bytesTitle, "window", "title")) {
			std::string strTitle(bytesTitle.begin(), bytesTitle.end());
			window.setTitle(strTitle);
		}
		
		// When Extractor goes out of scope, the datafile is closed	
	}

	// Create sprites from texture data
	sf::Sprite sprHouse(texHouse);
	sprHouse.setPosition(280, 50);
	sf::Sprite sprStairs(texStairs);
	sprStairs.setPosition(70, 120);

	// Start the game loop
	while (window.isOpen())
	{
		// Process events
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
		// Clear screen
		window.clear(sf::Color(0xf6, 0xf6, 0xf1, 0xff));
		// Draw sprites
		window.draw(sprHouse);
		window.draw(sprStairs);
		// Update the window
		window.display();
	}

	return 0;
}
```

Running the above code shows a window showcasing our two sprites loaded from the datafile:

![Howto3](howto4.png?raw=true)

## Phony Unlime

During development, it is highly recommended that you use the unlime_phony.h header in place of unlime.h. The reason for this is that, presumably, during development of your game, the assets will get added, removed and changed over time. At the point when we start working with hundreds of assets, it gets tiring to have to repack the datafile over and over again for any kind of change, especially when all we want to do is - for example - quickly test how a new asset looks like. By including unlime_phony.h in place of unlime.h, we can continue using the Unlime API, but read the resource manifest and related asset files directly instead. The only thing you need to worry about is correctly setting the filename Unlime gets associated with - you use the datafile for unlime.h and the resource manifest for unlime_phony.h. To aid with this, `UNLIME_PHONY` is defined whenever the phony header is added.
