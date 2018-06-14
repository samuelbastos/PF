#include "Model.h"
#include <algorithm>
Model *Model::s_instance = 0;

Model::Model() 
{
	m_i = 0;
	m_j = 0;
	m_k = 0;

	m_reader = ifstream("general.bin", ios::in | ios::binary);
	int* buffer = new int[2];
	m_reader.read((char*)buffer, 8);
	m_volumeOriginalSize = buffer[0];
	m_numberTotalTiles = buffer[1];
	m_reader.close();

	m_reader = ifstream("mapKeyPos.bin", ios::in | ios::binary);
	for (int i = 0; i < m_numberTotalTiles; i++)
	{
		int* buffer = new int[2];
		m_reader.seekg(i * 8);
		m_reader.read((char*)buffer, 8);
		m_mapPos.insert(std::pair<int, int>(buffer[0], buffer[1]));
	}

	m_reader = ifstream("mapKeyCoord.bin", ios::in | ios::binary);
	for (int i = 0; i < m_numberTotalTiles; i++)
	{
		int* buffer = new int[5];
		m_reader.seekg(i * 20);
		m_reader.read((char*)buffer, 20);
		n_mapCoord.insert(std::pair<int, glm::vec3>(buffer[0],
			glm::vec3(buffer[1], buffer[2], buffer[3])));
	}
	m_reader.close();

	m_brickPos = new float[12 * m_numberTotalTiles];
	for (int i = 0; i < (12 * m_numberTotalTiles); i++)
		m_brickPos[i] = -1.0f;
	for (int i = 0; i < m_numberTotalTiles; i++)
		m_brickPosVec.push_back(glm::vec3(-1));

	for (int i = 0; i < m_numberTotalTiles; i++)
		m_mapVis.insert(std::pair<int, bool>(i, false));

	m_reader.close();
}
Model::~Model(){}

void Model::setBrickPosition(int brickID, glm::vec3 position)
{
	m_brickPosVec[brickID]  = position;
	m_brickPos[brickID*3]   = position.x;
	m_brickPos[brickID*3+1] = position.y;
	m_brickPos[brickID*3+2] = position.z;
}

glm::vec3 Model::genNewStoragePoint()
{
	auto newpoint = glm::vec3(m_i * 32, m_j * 32, m_k * 32);
	if (m_i < 7) m_i++; 
	else if (m_j < 7) m_j++;
	else if (m_k < 7) m_k++;
	return newpoint;
}