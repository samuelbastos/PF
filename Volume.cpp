#include "Volume.h"

#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>

namespace vr
{
	Volume::Volume()
		: m_width(0), m_height(0), m_depth(0)
	{
		m_scalar_values = NULL;
		data_storage_size = DataStorageSize::_8_BITS;

		scalex = 1.0f;
		scaley = 1.0f;
		scalez = 1.0f;
	}

	Volume::Volume(unsigned int width, unsigned int height, unsigned int depth)
		: m_width(width), m_height(height), m_depth(depth)
	{
		m_scalar_values = NULL;
		data_storage_size = DataStorageSize::_8_BITS;
		m_scalar_values = new unsigned char[m_width*m_height*m_depth];

		for (int i = 0; i < width * height * depth; i++)
			m_scalar_values[i] = 0;

		scalex = 1.0f;
		scaley = 1.0f;
		scalez = 1.0f;
	}

	Volume::Volume(unsigned int width, unsigned int height, unsigned int depth, unsigned char* scalars, DataStorageSize storage_size)
		: m_width(width), m_height(height), m_depth(depth)
	{
		m_scalar_values = NULL;
		data_storage_size = storage_size;
		m_scalar_values = new unsigned char[m_width*m_height*m_depth];

		if (scalars != NULL)
			for (int i = 0; i < width * height * depth; i++)
				m_scalar_values[i] = (unsigned char)scalars[i];

		scalex = 1.0f;
		scaley = 1.0f;
		scalez = 1.0f;
	}

	Volume::~Volume()
	{
		delete[] m_scalar_values;
	}

	int Volume::GetWidth()
	{
		return m_width;
	}

	int Volume::GetHeight()
	{
		return m_height;
	}

	int Volume::GetDepth()
	{
		return m_depth;
	}

	void Volume::SetScale(float sx, float sy, float sz)
	{
		scalex = sx;
		scaley = sy;
		scalez = sz;
	}

	float Volume::GetScaleX()
	{
		return scalex;
	}

	float Volume::GetScaleY()
	{
		return scaley;
	}

	float Volume::GetScaleZ()
	{
		return scalez;
	}

	unsigned char Volume::SampleVolume(int x, int y, int z)
	{
		x = glm::clamp<int>(x, 0, m_width - 1);
		y = glm::clamp<int>(y, 0, m_height - 1);
		z = glm::clamp<int>(z, 0, m_depth - 1);

		return m_scalar_values[x + (y * m_width) + (z * m_width * m_height)];
	}

	bool Volume::IsOutOfBoundary(int x, int y, int z)
	{
		return !((x >= 0 && x < m_width)
			&& (y >= 0 && y < m_height)
			&& (z >= 0 && z < m_depth));
	}
}