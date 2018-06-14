#pragma once

#include "seismicdata.h"
#include "Octree.h"
#include "Volume.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

namespace ooc
{
	class Preprocessor
	{
	public:
		Preprocessor(std::string path);
		~Preprocessor();

		bool preprocessData();
		vr::Volume* genTileVolume(int index);
		inline std::vector<glm::vec4> getTilesInfos() { return m_tileInfos; }

	private:

		int getPowerOfTwoResolution(int  w, int h, int d);
		bool loadData(std::string path);
		void genOctree();
		void genTilesData();
		void downscaleBricks();
		void writeToDisc();
		void fillBricksInfo();

		Octree m_octree;
		vr::Volume* m_data;
		std::vector<Tile> m_tiles;
		std::map<std::string, Tile> m_tilesMap;
		std::vector<glm::vec4> m_tileInfos;
		std::ofstream m_writer;
	};
}


