#pragma once
#include <iostream>
#include <glm/glm.hpp>

namespace vr
{
	class TransferFunction;

	enum DataStorageSize
	{
		_8_BITS = 1, // unsigned char  [0 -   255]
		_16_BITS = 2, // unsigned short [0 - 65535]
	};
	static DataStorageSize GetStorageSizeType(size_t bytesize)
	{
		if (bytesize == sizeof(unsigned char))
			return DataStorageSize::_8_BITS;
		else if (bytesize == sizeof(unsigned short))
			return DataStorageSize::_16_BITS;
	}

	enum VolumeType
	{
		STRUCTURED,
		UNSTRUCTURED,
	};

	//template<class T>
	class Volume
	{
	public:
		Volume();
		Volume(unsigned int width,
			unsigned int height,
			unsigned int depth);
		Volume(unsigned int width,
			unsigned int height,
			unsigned int depth,
			unsigned char* scalars,
			DataStorageSize storage_size = DataStorageSize::_8_BITS);
		~Volume();

		int GetWidth();
		int GetHeight();
		int GetDepth();

		void SetScale(float sx, float sy, float sz);
		float GetScaleX();
		float GetScaleY();
		float GetScaleZ();

		unsigned char SampleVolume(int x, int y, int z);
		bool IsOutOfBoundary(int x, int y, int z);

		std::string GetName()
		{
			return m_name;
		}

		void SetName(std::string name)
		{
			m_name = name;
		}

		bool Validate()
		{
			return m_scalar_values != NULL;
		}

		double GetMaxDensity()
		{
			if (data_storage_size == DataStorageSize::_8_BITS)
				return 255.0;
			else if (data_storage_size == DataStorageSize::_16_BITS)
				return 65535.0;
			return 0.0;
		}

		unsigned char* getBuffer()
		{
			return m_scalar_values;
		}

	protected:

	private:
		std::string m_name;

		glm::dvec3 m_pmin;
		glm::dvec3 m_pmax;

		float scalex, scaley, scalez;
		unsigned int m_width, m_height, m_depth;

		DataStorageSize data_storage_size;
		unsigned char* m_scalar_values;
	};

}