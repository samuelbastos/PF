#include "Preprocessor.h"
#include <math.h>       /* fmod */
#include <algorithm>    // std::sort

// Base:		32768 32
// Sintetico:	512	  8
#define TILE_SIZE 32768
#define TILE_DIMENSION 32

namespace ooc
{
	Preprocessor::Preprocessor(std::string path) 
	{
		Preprocessor::loadData(path);
	}


	Preprocessor::~Preprocessor()
	{
	}

	bool Preprocessor::preprocessData()
	{
		Preprocessor::genOctree();
		Preprocessor::genTilesData();
		Preprocessor::fillBricksInfo();
		Preprocessor::writeToDisc();
		return true;
	}

	/* Pega uma tile 32x32x32 do Volume (no caso 8x8x8 agora, mudar pra 32 quando trocar pro Base) */
	vr::Volume* Preprocessor::genTileVolume(int index)
	{
		auto tile = m_tiles[index];
		int size = tile.values.size();

		unsigned char* scalar_values = new unsigned char[size];
		for (int i = 0; i < size; i++)
			scalar_values[i] = tile.values[i]->getValue();
		tile.values.clear();

		return new vr::Volume(TILE_DIMENSION, TILE_DIMENSION, TILE_DIMENSION
			, scalar_values, vr::DataStorageSize::_8_BITS);
	}

	int Preprocessor::getPowerOfTwoResolution(int  w, int h, int d)
	{
		int min;
		int aux = 2;
		if (w < h && w < d) min = w;
		else if (h < w && h < d) min = h;
		else if (d < w && d < h) min = d;
		while (aux < min)
			aux *= 2;
		return aux / 2;
	}

	/* Carrega dado sísmico */
	/* Converte o SeismicData em um Volume */
	bool Preprocessor::loadData(std::string path)
	{
		ResSeismicData* seismicData = new ResSeismicData(path.c_str());
		if (!seismicData->IsValid())
			return false;

		int w, h, d;
		seismicData->GetTextureDimensions(&w, &h, &d);
		int p2 = getPowerOfTwoResolution(w, h, d);
		int MAX_RES = p2 * p2 * p2;
		unsigned char *b = new unsigned char[MAX_RES];
		memcpy(b, seismicData->getData(), sizeof(unsigned char)*MAX_RES);

		auto data = seismicData->getData();
		delete[] data;
		delete seismicData;

		m_data = new vr::Volume(p2, p2, p2, b, vr::DataStorageSize::_8_BITS);
		m_data->SetName("Seismic");

		printf("  - Volume Name     : %s\n", "Seismic");
		printf("  - Volume Size     : [%d, %d, %d]\n", p2, p2, p2);
		printf("  - Volume Byte Size: %d\n", vr::DataStorageSize::_8_BITS);

		return true;
	}

	/*
	* Gera a Octree, ao mesmo tempo que elimina as folhas (clearLeafNodes ())que possuem apenas um valor escalar,
	* e joga essas folhas pro nó intermediário imediatamente acima (saveTiles()), que passa a ser o novo nó folha.
	*/
	void Preprocessor::genOctree()
	{
		auto w = m_data->GetWidth();
		auto h = m_data->GetHeight();
		auto d = m_data->GetDepth();

		m_octree = Octree(glm::vec3(w / 2, h / 2, d / 2), glm::vec3(w / 2, h / 2, d / 2));

		for (int k = 0; k < d; k++)
			for (int j = 0; j < h; j++)
				for (int i = 0; i < w; i++)
					m_octree.insert(new OctreePoint(glm::vec3(i, j, k), m_data->SampleVolume(i, j, k)));

		std::cout << "Octree preenchida com todos os pontos;" << std::endl;
		m_octree.generateTiles();
		std::cout << "Octree preenchida com todos os Tiles;" << std::endl;

		m_octree. getTiles(&m_tilesMap);
	}

	void Preprocessor::genTilesData()
	{
		for (map<std::string, Tile>::iterator it = m_tilesMap.begin(); it != m_tilesMap.end(); ++it)
		{
			m_tiles.push_back(it->second);
		}

		// Arruma ordem dos tiles por id direito!
		for (auto& tile : m_tiles)
		{
			int i, j, k;
			int id, fn, levelMO, n;
			levelMO = tile.level;
			n = powf(2, tile.level);
			//tileLocation = tile.origin % (2 * tile.halfDimension);
			i = std::floorf(tile.origin.x / (2 * tile.halfDimension));
			j = std::floorf(tile.origin.y / (2 * tile.halfDimension));
			k = std::floorf(tile.origin.z / (2 * tile.halfDimension));
			fn = std::floorf((powf(8, levelMO) - 1.0)/7.0);
			id = fn + (i*n + j)*n + k;
			tile.key = id;
		}

		//Ordena no vetor por tile id;
		std::sort(m_tiles.begin(), m_tiles.end(), [=](Tile one, Tile two) { return one.key < two.key; });
	}


	void Preprocessor::writeToDisc()
	{
		std::vector<int> posTiles;

		// Inicializa writer
		m_writer = std::ofstream("tiles.bin", std::ios::out | std::ios::binary);
		int iter = 1;
		for (auto tile : m_tiles)
		{
			unsigned char* buffer = new unsigned char[TILE_SIZE];
			for (int i = 0; i < TILE_SIZE; i++)
				buffer[i] = tile.values[i]->getValue();
			tile.values.clear();
			m_writer.write((char*)buffer, TILE_SIZE);
			m_writer.seekp(iter*TILE_SIZE);
			posTiles.push_back((iter-1)*TILE_SIZE);
			iter++;
		}
		m_writer.close();

		m_writer = std::ofstream("mapKeyPos.bin", std::ios::out | std::ios::binary);
		iter = 1;
		for (auto tile : m_tiles)
		{
			int* buffer = new int[2];
			buffer[0] = tile.key;
			buffer[1] = posTiles[iter - 1];
			m_writer.write((char*)buffer, 8); // 16 = n bytes de float x 4
			m_writer.seekp(iter * 8);
			iter++;
		}
		m_writer.close();

		m_writer = std::ofstream("general.bin", std::ios::out | std::ios::binary);
		int* buffer = new int[2];
		buffer[0] = m_data->GetWidth();
		buffer[1] = m_tiles.size();
		m_writer.write((char*)buffer, 8); // 16 = n bytes de int x 4
		m_writer.close();
	}

	void Preprocessor::fillBricksInfo()
	{
		for (auto tile : m_tiles)
			m_tileInfos.push_back(glm::vec4(tile.origin, tile.halfDimension));
	}
}