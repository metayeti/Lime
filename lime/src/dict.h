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
  *  dict.h
  *  Dictionary class used for internal representation of the resource manifest list.
  *
  */

#pragma once

#ifndef LIME_DICT_H_
#define LIME_DICT_H_

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

namespace Lime
{
	template<typename T>
	class DMap
	{
	private:
		using T_DataIndexMap = std::unordered_map<std::string, std::size_t>;
		using T_DataItem = std::pair<std::string, T>;
		using T_DataContainer = std::vector<T_DataItem>;
		using T_MultiArgs = typename std::vector<std::pair<std::string, T>>;

		T_DataIndexMap dataIndexMap;
		T_DataContainer data;

		inline std::size_t setEmpty(std::string& key)
		{
			std::size_t index = data.size();
			dataIndexMap[key] = index;
			data.emplace_back(key, T());
			return index;
		}

	public:
		using const_iterator = typename T_DataContainer::const_iterator;

		DMap() { }

		DMap(DMap const& other)
		{
			std::size_t data_size = other.data.size();
			for (std::size_t i = 0; i < data_size; ++i)
			{
				auto const& key = other.data[i].first;
				auto const& obj = other.data[i].second;
				data.emplace_back(key, obj);
			}
			dataIndexMap = T_DataIndexMap(other.dataIndexMap);
		}

		T& operator[](std::string key)
		{
			auto it = dataIndexMap.find(key);
			bool hasIt = (it != dataIndexMap.end());
			std::size_t index = (hasIt) ? it->second : setEmpty(key);
			return data[index].second;
		}
		T get(std::string key) const
		{
			auto it = dataIndexMap.find(key);
			if (it == dataIndexMap.end())
			{
				return T();
			}
			return T(data[it->second].second);
		}
		bool has(std::string key) const
		{
			return (dataIndexMap.count(key) == 1);
		}
		void set(std::string key, T obj)
		{
			auto it = dataIndexMap.find(key);
			if (it != dataIndexMap.end())
			{
				data[it->second].second = obj;
			}
			else
			{
				dataIndexMap[key] = data.size();
				data.emplace_back(key, obj);
			}
		}
		void set(T_MultiArgs const& multiArgs)
		{
			for (auto const& it : multiArgs)
			{
				auto const& key = it.first;
				auto const& obj = it.second;
				set(key, obj);
			}
		}
		bool remove(std::string key)
		{
			auto it = dataIndexMap.find(key);
			if (it != dataIndexMap.end())
			{
				std::size_t index = it->second;
				data.erase(data.begin() + index);
				dataIndexMap.erase(it);
				for (auto& it2 : dataIndexMap)
				{
					auto& vi = it2.second;
					if (vi > index)
					{
						vi--;
					}
				}
				return true;
			}
			return false;
		}
		void clear()
		{
			data.clear();
			dataIndexMap.clear();
		}
		std::size_t size() const
		{
			return data.size();
		}
		const_iterator begin() const { return data.begin(); }
		const_iterator end() const { return data.end(); }
	};

	using Dict = DMap<DMap<std::string>>;

	Dict readDictFromFile(const char* const resourceManifestFilename);
}

#endif // LIME_DICT_H_
