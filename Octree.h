#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <string>

class OctreePoint {
	glm::vec3 position;
	unsigned char scalarValue;
public:
	OctreePoint() { }
	OctreePoint(const glm::vec3& position, unsigned char scalarValue) : position(position)
															, scalarValue(scalarValue){ }
	inline const glm::vec3& getPosition() const { return position; }
	inline void setPosition(const glm::vec3& p) { position = p; }
	inline unsigned char getValue() { return scalarValue; }
};

typedef struct Tile {
	int key;
	glm::vec3 origin;
	float halfDimension;
	int level;
	std::vector<OctreePoint*> values;
} Tile;

class Octree
{
public:
	// Physical position/size. This implicitly defines the bounding box of this node

	glm::vec3 origin;							/* The physical center of this node.				*/
	glm::vec3 halfDimension;					/* Half the width/height/depth of this node.		*/
	Octree *children[8];						/* Pointers to child octants.						*/
	Octree* parent;								/* Ponteiro para nó pai.							*/
	OctreePoint *data;							/* Data point to be stored at a node.				*/
	std::vector<OctreePoint*> values;			/* Vetor que guardar os voxels da Tile.				*/
	int level;									/* Diz em qual altura da árvore o nó se encontra.	*/
	int id;
	bool trueLeaf = false;						/* Marca os nós folhas.								*/			
	bool tileComputed = false;					/* Marca se para determinado nó foi calculado Tile. */

	/*
	Children follow a predictable pattern to make accesses simple.
	Here, - means less than 'origin' in that dimension, + means greater than.
	child:	0 1 2 3 4 5 6 7
	x:      - - - - + + + +
	y:      - - + + - - + +
	z:      - + - + - + - +
	*/

public:

	Octree(){}
	Octree(const glm::vec3& origin, const glm::vec3& halfDimension)
		: origin(origin), halfDimension(halfDimension), data(NULL) {
		// Initially, there are no children
		for (int i = 0; i<8; ++i)
			children[i] = NULL;

		parent = NULL;
		level = 0;
	}

	Octree(const Octree& copy)
		: origin(copy.origin), halfDimension(copy.halfDimension), data(copy.data) {

	}

	~Octree() {
		// Recursively destroy octants
		for (int i = 0; i<8; ++i)
			delete children[i];
	}

	// Determine which octant of the tree would contain 'point'
	int getOctantContainingPoint(const glm::vec3& point) const {
		int oct = 0;
		if (point.x >= origin.x) oct |= 4;
		if (point.y >= origin.y) oct |= 2;
		if (point.z >= origin.z) oct |= 1;
		return oct;
	}

	bool isLeafNode() const {
		// We are a leaf iff we have no children. Since we either have none, or 
		// all eight, it is sufficient to just check the first.
		return children[0] == NULL;
	}

	// Tão sendo salvas na variável global
	void getTiles(std::map<std::string, Tile>* tilesMap)
	{
		// Pega apenas até o lvl 3
		if(level <= 3)
			for (int i = 0; i < 8; i++) {
				Tile tile;
				tile.origin = origin;
				tile.halfDimension = halfDimension.x;
				tile.values = values;
				tile.level = level;
				std::string key = std::to_string((int)level) + std::to_string((int)origin.x) + std::to_string((int)origin.y)
					 + std::to_string((int)origin.z);
				tile.key = -1;
				tilesMap->insert(std::pair<std::string,Tile>(key,tile));
				// Level 3 não chama getTile!
				if(level <= 2)
					children[i]->getTiles(tilesMap);
			}
	}

	/* (INCOMPLETE) Método que só calcula para o nível superior ao marcado com TileComputed os tiles */
	void generateUpperTiles()
	{
		/* É uma folha, precisa preencher o pai com uma Tile nova */
		if (tileComputed) {
			// TODO: Interpolação dos valores dos filhos!!
			/* Se o pai já foi preenchido (no primeiro filho que acha já preenche, não
			   precisa fazer o cálculo novamente */
			if (parent->values.empty())
				for (int i = 0; i < 8; i++)
					for (int j = 0; j < parent->children[i]->values.size(); j++)
						parent->values.push_back(parent->children[i]->values[j]);

			parent->tileComputed = true;
			tileComputed = false;
		}
		else {
			for (int i = 0; i < 8; i++)
				children[i]->generateUpperTiles();
		}
	}

	/* Método que gera todos os tiles da octree*/
	void generateTiles()
	{
		while (!tileComputed) {
			generateUpperTiles();
		}
	}

	void insert(OctreePoint* point) {
		
		// If this node doesn't have a data point yet assigned 
		// and it is a leaf, then we're done!
		if (isLeafNode()) {
			if (data == NULL) {
				data = point;
				return;
			}
			else {
				// We're at a leaf, but there's already something here
				// We will split this node so that it has 8 child octants
				// and then insert the old data that was here, along with 
				// this new data point

				if(parent)
					level = parent->level + 1;

				// 32x32x32 Tiles (para o dado Base, precisa mudar esse nível se mudar o dado)
				// para continuar criando texturas deste tamanho !
				if (level == 3) {
					if (!trueLeaf)
					{
						trueLeaf = true;
						tileComputed = true;
					}
					// Para inserir apenas uma vez o ponto que seria jogado pra baixo na árvore
					if (values.empty())
						values.push_back(data);
					values.push_back(point);
					return;
				}
					
				// Save this data point that was here for a later re-insert
				OctreePoint *oldPoint = data;
				data = NULL;

				// Split the current node and create new empty trees for each
				// child octant.
				for (int i = 0; i<8; ++i) {
					// Compute new bounding box for this child
					glm::vec3 newOrigin = origin;
					newOrigin.x += halfDimension.x * (i & 4 ? .5f : -.5f);
					newOrigin.y += halfDimension.y * (i & 2 ? .5f : -.5f);
					newOrigin.z += halfDimension.z * (i & 1 ? .5f : -.5f);
					children[i] = new Octree(newOrigin, halfDimension*.5f);
					children[i]->parent = this;
				}

				// Re-insert the old point, and insert this new point
				// (We wouldn't need to insert from the root, because we already
				// know it's guaranteed to be in this section of the tree)
				children[getOctantContainingPoint(oldPoint->getPosition())]->insert(oldPoint);
				children[getOctantContainingPoint(point->getPosition())]->insert(point);
			}
		}
		else {
			// We are at an interior node. Insert recursively into the 
			// appropriate child octant
			int octant = getOctantContainingPoint(point->getPosition());
			children[octant]->insert(point);
		}
	}
};

