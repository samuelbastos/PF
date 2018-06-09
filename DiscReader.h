#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include "Model.h"

using namespace std;

class DiscReader
{
public:
	DiscReader();
	~DiscReader();
	void readDataTEST(int tileLocation, int infoLocation);
	unsigned char* readTileData(int infoLocation);

private:
	ifstream m_tileReader;
	ifstream m_infoReader;
};