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
  *  packer.h
  *  Defines the class for datafile packing.
  *
  */

#pragma once

#ifndef LIME_PACK_H_
#define LIME_PACK_H_

#include <string>
#include "dict.h"
#include "interface.h"

namespace Lime
{
	enum class ChkSumOption : unsigned char
	{
		ADLER32, CRC32, NONE
	};

	struct PackOptions
	{
		unsigned char clevel = 9;
		ChkSumOption chksum = ChkSumOption::ADLER32;
		std::string headstr;
	};

	void pack(Interface& inf, Dict& resourceDict, std::string const& outputFilename, PackOptions& options);
}

#endif // LIME_PACK_H_