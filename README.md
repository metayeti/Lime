# LimePack

![Lime](/lime.png?raw=true)

(0.9.4, __not__ battle tested and most likely __not__ ready for production)

## Info

LimePack is a set of tools for data and asset packing intended for C++ game development. It depends on [zlib](https://zlib.net/) for compression.

To create Lime datafiles, build the *Lime* utility in [`/lime/`](/lime/). You can find the build instructions in the build.txt file.

To unpack datafiles, copy the source file in [`/unlime/`](/unlime/) into your own project and use the provided classes to extract game data from a Lime datafile. Don't forget to link [zlib](https://zlib.net/) to your project.

To build the demo, read the build instructions in [`/demo/`](/demo/).

## Notes

Ideally, Lime is used for small datafiles but large datafiles (2GB and beyond) are supported. Packing enormous (individual) resource files is not recommended as Unlime expands each queried resource fully into memory (streaming is not supported). Packing lots of small resources is fine but packing may take a while when using compression. If dealing with large datafiles, it is recommended to turn off compression until deployment.

## Thanks

- [zlib](https://zlib.net/) - used for compression
- [SFML](https://www.sfml-dev.org/) - used for the demo project

## License

Copyright (c) 2019 Danijel Durakovic

Licensed under the terms of the [MIT license](LICENSE)
