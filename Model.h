#pragma once
#include <vector>
#include "Volume.h"
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;
class Model
{
	float* m_brickPos;
	std::map<int, glm::vec4> m_mapCoord;			// Map Chave do Brick <-> Coordenadas do Brick ;
	std::map<int, std::pair<int, int>> m_mapPos;	// Map Chave do Brick <-> Posição onde se encontram seus dados nos arquivos ;
	ifstream m_reader;
	int m_numberTotalTiles;							// Total de Bricks gerados ;
	int m_volumeOriginalSize;						// Dimensão original do Volume carregado ;
	static Model* s_instance;						// Instancia do Modelo ;

public:
	Model();
	~Model();
	static Model * getInstance()
	{
		if (!s_instance)
			s_instance = new Model;
		return s_instance;
	}
	
	void setBrickPosition();
	inline int getNumberTotalTiles()   { return m_numberTotalTiles;   }
	inline int getVolumeOriginalSize() { return m_volumeOriginalSize; }
	inline float* getBricksPositions() { return m_brickPos; }
};

