#include "DiskReader.h"

#define TILE_SIZE 32768

DiskReader::DiskReader() : m_tileReader(ifstream("tiles.bin", ios::in | ios::binary))
						 , m_infoReader(ifstream("tilesinfo.bin", ios::in | ios::binary))
{
	Model::getInstance();
}

DiskReader::~DiskReader()
{
}

void DiskReader::readDataTEST(int tileLocation, int infoLocation)
{
	unsigned char* tilebuffer = new unsigned char[TILE_SIZE];
	//m_reader.seekg(location * TILE_SIZE);
	m_tileReader.read((char*)tilebuffer, TILE_SIZE);
	m_tileReader.close();

	float* infobuffer = new float[4];
	m_infoReader.read((char*)infobuffer, 16);
	m_infoReader.close();

	printf("1: %f\n", infobuffer[0]);
	printf("2: %f\n", infobuffer[1]);
	printf("3: %f\n", infobuffer[2]);
	printf("4: %f\n", infobuffer[3]);
}

unsigned char* DiskReader::readTileData(int tileLocation)
{
	unsigned char* tilebuffer = new unsigned char[TILE_SIZE];
	m_tileReader.seekg(tileLocation);
	m_tileReader.read((char*)tilebuffer, TILE_SIZE);
	//m_tileReader.close();
	return tilebuffer;
}