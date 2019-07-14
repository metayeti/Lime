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
  *  packer.cpp
  *  Implements the class for datafile packing.
  *
  */

#include "pack.h"
#include "dict.h"

namespace Lime
{
	void pack(Dict& dict, const char* const outputFilename)
	{
		/*

		Datafile structure:


		         Z0           Z1    ...   Zn
		        [~~~~~~~~~~] [~~~] [~~~] [~~~]        (zipped content)

		  head   dictionary   user resources   checksum
		|______|____________|________________|__________|
		             |
		             |
		             |
		             |
		         dictionary:

		         N   section 1   ...   section N
		       |___|___________|     |___________|
		                 |
		                 |
		                 |
		                 |
		              section:

		              section key   N   data 1   ...   data N
		            |_____________|___|________|     |________|
		                                  |
		                                  |
		                                  |
		                                data:

		                                data key   seek_id   size
		                              |__________|_________|______|

		*/

		/*
		for (auto const& it : dict)
		{
			auto const& categoryName = it.first;
			auto const& collection = it.second;
			for (auto const& it2 : collection)
			{
				auto const& key = it2.first;
				auto const& value = it2.second;
			}
		}
		*/

	}
}