#include "Model.h"
#include <algorithm>
// Base:		32 1024 32768 256
// Sintetico:	8  64   512   64
Model *Model::s_instance = 0;
			
#define TTT 32768					// TEXTURE TOTAL TEXELS
#define VOS 256						// VOLUME ORIGINAL SIZE

Model::Model() 
{
	m_reader = ifstream("general.bin", ios::in | ios::binary);
	int* buffer = new int[2];
	m_reader.read((char*)buffer, 8);
	m_volumeOriginalSize = buffer[0];
	m_numberTotalTiles = buffer[1];
	m_reader.close();

	m_reader = ifstream("mapKeyCoord.bin", ios::in | ios::binary);
	for (int i = 0; i < m_numberTotalTiles; i++)
	{
		int* buffer = new int[5];
		m_reader.seekg(i * 20);
		m_reader.read((char*)buffer, 20);
		m_mapCoord.insert(std::pair<int, glm::vec4>(buffer[0],
			glm::vec4(buffer[1], buffer[2], buffer[3], buffer[4])));
	}
	m_reader.close();

	m_reader = ifstream("mapKeyPos.bin", ios::in | ios::binary);
	for (int i = 0; i < m_numberTotalTiles; i++)
	{
		int* buffer = new int[3];
		m_reader.seekg(i * 12);
		m_reader.read((char*)buffer, 12);
		m_mapPos.insert(std::pair<int, std::pair<int, int>>(buffer[0],
			std::make_pair(buffer[1], buffer[2])));
	}

	m_brickPos = new float[12 * m_numberTotalTiles];
	for (int i = 0; i < (12 * m_numberTotalTiles); i++)
		m_brickPos[i] = -1.0f;
	
	m_reader.close();
}
Model::~Model(){}

void Model::setBrickPosition()
{
	m_brickPos[0] = 0.0f;
	m_brickPos[1] = 0.0f;
	m_brickPos[2] = 0.0f;
}