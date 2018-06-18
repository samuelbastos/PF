#include "Model.h"
#include <algorithm>
Model *Model::s_instance = 0;

Model::Model() 
{
	loadChildsVector();

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

int Model::fid(int level, glm::vec3 ijk)
{
	int N = (int)(floor((pow(8, level) - 1) / 7));
	int nn = (int)(pow(2, level));
	int t = N + (ijk.x*nn + ijk.y)*nn + ijk.z;
	return t;
}

glm::vec3 Model::fijk(int level, int id)
{
	int n = (int)(pow(2, level));
	int n2 = (int)(pow(n, 2));
	int delta = id - (int)(floor((pow(8, level) - 1) / 7));
	int i = (int)(floor(floor(delta / n) / n));
	int j = (int)(((int)(floor(delta / n)) % n));
	int k = (int)(((int)((delta % n)) % n));
	return  glm::vec3(i, j, k);
}

int Model::fparent(int level, int id)
{
	glm::vec3 ijk = fijk(level, id);
	int i = int(floor(ijk.x / 2));
	int j = int(floor(ijk.y / 2));
	int k = int(floor(ijk.z / 2));
	int p = fid((level - 1), glm::vec3(i, j, k));
	return p;
}

void Model::loadChildsVector()
{
	for (int i = 0; i < 585; i++)
	{
		std::vector<int> aux;
		m_childs.push_back(aux);
	}
	for (int i = 1; i < 585; i++)
	{	
		int level = 0;
		if (i > 0 && i < 9) level = 1;
		else if (i > 8 && i < 73) level = 2;
		else if (i > 72) level = 3;
		int parent = fparent(level, i);
		(m_childs[parent]).push_back(i);
	}
}

std::vector<int> Model::getChilds(int id)
{
	return m_childs[id];
}