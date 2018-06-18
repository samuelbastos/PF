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
	vector<glm::vec3> m_brickPosVec;
	std::map<int, int> m_mapPos;					// Map Chave do Brick <-> Posição onde se encontram seus dados nos arquivos ;
	std::map<int, bool> m_mapVis;
	std::map<int, glm::vec3> n_mapCoord;
	ifstream m_reader;
	int m_numberTotalTiles;							// Total de Bricks gerados ;
	int m_volumeOriginalSize;						// Dimensão original do Volume carregado ;
	static Model* s_instance;						// Instancia do Modelo ;
	int m_i, m_j, m_k;								// Registros do Storage (para sempre alocar uma origem diferente válida)
	std::vector<std::vector<int>> m_childs;

public:
	Model();
	~Model();
	static Model * getInstance()
	{
		if (!s_instance)
			s_instance = new Model;
		return s_instance;
	}
	
	void setBrickPosition(int brickID, glm::vec3 position);
	inline void setBrickVisible(int brickID) { m_mapVis[brickID] = true; }
	glm::vec3 genNewStoragePoint();
	inline int getNumberTotalTiles()   { return m_numberTotalTiles;   }
	inline int getVolumeOriginalSize() { return m_volumeOriginalSize; }
	inline float* getBricksPositions() { return m_brickPos; }
	inline int getPositionInFileById(int id) { return m_mapPos[id]; }
	inline bool getBrickVisibility(int id) { return m_mapVis[id]; }
	inline std::vector<glm::vec3> getBricksPositionsVec() { return m_brickPosVec;  }
	int fid(int level, glm::vec3 ijk);
	glm::vec3 fijk(int level, int id);
	int fparent(int level, int id);
	void loadChildsVector();
	std::vector<int> getChilds(int id);
};