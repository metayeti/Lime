# LimePack

![Lime](/lime.png?raw=true)

## Info

LimePack is a set of tools for data and asset packing for C++ game development. It depends on [zlib](https://zlib.net/) for compression.

To create Lime datafiles, build the *Lime* utility in [`/lime/`](/lime/). You can find the build instructions in the build.txt file.

To unpack datafiles, copy the source files in [`/unlime/`](/unlime/) into your own project and use the provided classes to extract game data from a Lime datafile.

To build the demo, read the readme and the build instructions in [`/demo/`](/demo/).

## Thanks

- [zlib](https://zlib.net/) - used for compression
- [SFML](https://www.sfml-dev.org/) - used in the demo project

## License

Copyright (c) 2019 Danijel Durakovic

Licensed under the terms of the [MIT license](LICENSE)
