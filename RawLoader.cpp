#include "RawLoader.h"

namespace lqc
{
	RawLoader::RawLoader(std::string filename, size_t bytes_per_pixel, size_t num_voxels, size_t type_size)
	{
		m_data = NULL;
		m_filename = filename;
		m_bytesperpixel = bytes_per_pixel;
		m_numvoxels = num_voxels;
		m_typesize = type_size;

		FILE *fp;
		if (!(fp = fopen(filename.c_str(), "rb")))
		{
			std::cout << "lqc: opening .raw file failed" << std::endl;
			exit(EXIT_FAILURE);
		}
		else
		{
			std::cout << "lqc: open .raw file successed" << std::endl;
		}

		m_data = (void*)malloc(m_numvoxels * type_size * sizeof(unsigned char));

		size_t tmp = fread(m_data, m_bytesperpixel, m_numvoxels, fp);
		if (tmp != m_numvoxels)
		{
			std::cout << "lqc: read .raw file failed. " << tmp << " bytes read != " << m_numvoxels << " bytes expected." << std::endl;
			fclose(fp);
			free(m_data);
			m_data = NULL;
			exit(EXIT_FAILURE);
		}
		else
		{
			std::cout << "lqc: read .raw file successed" << std::endl;
			fclose(fp);
		}
	}

	RawLoader::~RawLoader()
	{
		//if(m_data)
		//free(m_data);
	}

	void* RawLoader::GetData()
	{
		return m_data;
	}

	bool RawLoader::IsLoaded()
	{
		return (m_data != NULL);
	}
}