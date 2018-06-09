#pragma once
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace lqc
{
	class RawLoader
	{
	public:
		RawLoader(std::string fileName, size_t bytes_per_pixel, size_t num_voxels, size_t type_size);
		~RawLoader();

		void* GetData();
		bool IsLoaded();
	private:
		std::string m_filename;
		size_t m_bytesperpixel;
		size_t m_numvoxels;
		size_t m_typesize;
		void* m_data;
	};
}

