#include "TerrainManager.h"

#include <glad/glad.h>
#include <algorithm>

#include "../Engine/MathsFunctions.h"
#include "../Engine/Program.h"

TerrainManager::TerrainManager(glm::ivec2 arenaSize):arenaSize{arenaSize}
{
	//Arena size is the amount of squares wide/tall it is, since the scalarData doesn't include the edges that will default to -1, it is 1 unit smaller in both dimensions
	scalarDataSize = (arenaSize.x - 1) * (arenaSize.y - 1);	
	scalarData = new float[scalarDataSize];

	lineArray = new line[arenaSize.x * arenaSize.y * 2];

	glGenVertexArrays(1,&triangleVAO);	//Create the VAO and buffer for the triangles in the terrain, and allocate the memory needed for them
	glBindVertexArray(triangleVAO);

	glGenBuffers(1, &triangleVBO);
	glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle) * 4 * arenaSize.x * arenaSize.y, nullptr, GL_DYNAMIC_DRAW);	//size of buffer = size of triangle * 4 triangles per square * number of squares
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	glEnableVertexAttribArray(0);

	glGenVertexArrays(1, &lineVAO);	//Create the VAO and buffer for the lines in the terrain, and allocate the memory needed for them
	glBindVertexArray(lineVAO);

	glGenBuffers(1, &lineVBO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line) * 2 * arenaSize.x * arenaSize.y, nullptr, GL_DYNAMIC_DRAW);	//size of buffer = size of line * 2 lines per square * number of squares
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	glEnableVertexAttribArray(0);

	triangleProgram = new Program("src/Shaders/terrain/triangles/vertex.glsl","src/Shaders/terrain/triangles/fragment.glsl");
	lineProgram = new Program("src/Shaders/terrain/lines/vertex.glsl", "src/Shaders/terrain/lines/fragment.glsl");
}

TerrainManager::~TerrainManager()
{
	delete[] scalarData;	//Deleta arrays
	delete[] lineArray;

	glDeleteBuffers(1, &triangleVBO);	//Delete buffers
	glDeleteBuffers(1, &lineVBO);

	glDeleteVertexArrays(1, &triangleVAO);	//Delete VAOs
	glDeleteVertexArrays(1, &lineVAO);

	delete triangleProgram;
	delete lineProgram;
}

void TerrainManager::uploadStage(float* stage)
{
	std::copy(stage, stage + scalarDataSize, scalarData);

	generateTerrain();
}

void TerrainManager::generateTerrain()
{
	/*
	* Get the relavent values at each corner of the square
	* Use these to get the index of that configuration,
	* Use vertices and index tables to get which vertices should be calculated using lerp
	* Iterate through indices to construct triangles
	* Separate index table for which vertices should be assembled into lines
	*/

	/*	Corner order, and bit value
	*	3---2	8---4
	*	|	|	|	|
	*	|	|	|	|
	*	0---1	1---2
	*
	*	Edge bit value (7 MSB)
	*
	*	7-6-5
	*	|   |
	*	0	4
	*   |   |
	*	1-2-3
	*/

	constexpr char vertexTable[16]{	//Which vertices are used in the mesh
	0b00000000,	//0
	0b00000111,
	0b00011100,
	0b00011011,
	0b01110000,
	0b01110111,	//5
	0b01101100,
	0b01101011,
	0b11000001,
	0b11000110,
	0b11011101,	//10
	0b11011010,
	0b10110001,
	0b10110110,
	0b10101101,
	0b10101010	//15
	};

	//How the vertices are assembled in clockwise order, starting at the MSB, upto 4 triangles, 8 means no more triangles
	constexpr char triangleIndexTable[16][13]{
		{8,8,8,8,8,8,8,8,8,8,8,8,8},//0
		{2,1,0,8,8,8,8,8,8,8,8,8,8},
		{4,3,2,8,8,8,8,8,8,8,8,8,8},
		{4,3,0,3,1,0,8,8,8,8,8,8,8},
		{6,5,4,8,8,8,8,8,8,8,8,8,8},
		{6,5,4,6,4,2,6,2,0,2,1,0,8},//5
		{6,5,3,6,3,2,8,8,8,8,8,8,8},
		{6,5,3,6,3,0,3,1,0,8,8,8,8},
		{7,6,0,8,8,8,8,8,8,8,8,8,8},
		{7,6,2,7,2,1,8,8,8,8,8,8,8},
		{7,6,0,6,4,2,6,2,0,4,3,2,8},//10
		{7,6,1,6,4,1,4,3,1,8,8,8,8},
		{7,5,4,7,4,0,8,8,8,8,8,8,8},
		{7,5,4,7,4,2,7,2,1,8,8,8,8},
		{7,5,0,5,2,0,5,3,2,8,8,8,8},
		{7,5,3,7,3,1,8,8,8,8,8,8,8}	//15
	};

	//How the lines are assembled, in order so that for vector B-A, "left" of that direction is air and "right" is the solid terrain
	constexpr char lineIndexTable[16][5]{
		{8,8,8,8,8},//0
		{0,2,8,8,8},
		{2,4,8,8,8},
		{0,4,8,8,8},
		{4,6,8,8,8},
		{0,6,4,2,8},//5
		{2,6,8,8,8},
		{0,6,8,8,8},
		{6,0,8,8,8},
		{6,2,8,8,8},
		{6,4,2,0,8},//10
		{6,4,8,8,8},
		{4,0,8,8,8},
		{4,2,8,8,8},
		{2,0,8,8,8},
		{8,8,8,8,8}	//15
	};

	triangle* triangleArray = new triangle[4 * arenaSize.x * arenaSize.y];	//Create a temporary array for the triangles created
	
	int squareIndex = 0;	//Which square is currently being generated
	for (int y = -1; y < arenaSize.y-1; y++)
	{
		for (int x = -1; x < arenaSize.x-1; x++)
		{
			float cornerValues[4]	//Get the floating point values within each corner
			{
				getPoint({x  ,y+1}),	//(0,1) BL
				getPoint({x+1,y+1}),	//(1,1) BR
				getPoint({x+1,y  }),	//(1,0) TR
				getPoint({x  ,y  }),	//(0,0) TL
			};

			int index{};	//Get the index of the correct configuration of triangles and lines
			for (int i = 3; i >= 0; i--)
			{
				index = (index << 1) | cornerValues[i] > 0;
			}

			glm::vec2 corners[4]	//Get the co-ordinates of each corner (offset by 1 so the actual positions start at 0)
			{
				{x+1,y+2},
				{x+2,y+2},
				{x+2,y+1},
				{x+1,y+1},
			};

			const char vertexMask = vertexTable[index];	//Get which vertices are used in the final output
			const char* triangleIndices = triangleIndexTable[index];	//Get the assembly order of the vertices when making triangles and lines
			const char* lineIndices = lineIndexTable[index];

			glm::vec2 vertices[8]{};	//Get the vertices, only fills in the ones that are used
			
			if (vertexMask & 0b00000001) { vertices[0] = MathsFunctions::getMidPtY(corners[3], corners[0], cornerValues[3], cornerValues[0]); }
			if (vertexMask & 0b00000010) { vertices[1] = corners[0]; }
			if (vertexMask & 0b00000100) { vertices[2] = { MathsFunctions::getMidPtX(corners[0],corners[1],cornerValues[0],cornerValues[1]) }; }
			if (vertexMask & 0b00001000) { vertices[3] = corners[1]; }
			if (vertexMask & 0b00010000) { vertices[4] = { MathsFunctions::getMidPtY(corners[1],corners[2],cornerValues[1],cornerValues[2]) }; }
			if (vertexMask & 0b00100000) { vertices[5] = corners[2]; }
			if (vertexMask & 0b01000000) { vertices[6] = { MathsFunctions::getMidPtX(corners[2],corners[3],cornerValues[2],cornerValues[3]) }; }
			if (vertexMask & 0b10000000) { vertices[7] = corners[3]; }

			for (int i = 0; triangleIndices[i*3] != 8; i++)	//Assemble the triangles
			{
				triangle t{};
				t.A = vertices[triangleIndices[i*3 + 0]];	//Get each vertex of the triangle
				t.B = vertices[triangleIndices[i*3 + 1]];
				t.C = vertices[triangleIndices[i*3 + 2]];
				triangleArray[squareIndex * 4 + i] = t;		//Add the triangle to the array
			}

			for (int i = 0; i < 2; i++)	//Assemble the lines, lines need to be overwritten, so can't stop early
			{
				line l{};
				if (lineIndices[i * 2] == 8) { l = {}; }	//If there is no line, make it {0,0} to {0,0}
				else
				{
					l.A = vertices[lineIndices[i * 2 + 0]];	//If there is a line, assemble the line
					l.B = vertices[lineIndices[i * 2 + 1]];
				}
				lineArray[squareIndex * 2 + i] = l;
			}

			squareIndex++;
		}
	}
	//Put the triangle array in the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(triangle) * 4 * arenaSize.x * arenaSize.y, triangleArray);

	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line) * 2 * arenaSize.x * arenaSize.y, lineArray);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete[] triangleArray;
}

float TerrainManager::getPoint(glm::ivec2 pos)
{
	if (pos.x < 0 || pos.y < 0 || pos.x >= arenaSize.x-1 || pos.y >=arenaSize.y-1)	//If value is outside the bounds of the 
	{
		return -1;
	}
	int index = pos.y * (arenaSize.x - 1) + pos.x;
	return scalarData[index];
}

void TerrainManager::render()
{
	triangleProgram->use();
	triangleProgram->setVec2("gridSize", arenaSize);
	triangleProgram->setVec3("colour", glm::vec3{ 0,0,0 });

	glBindVertexArray(triangleVAO);
	glDrawArrays(GL_TRIANGLES, 0, 12 * arenaSize.x * arenaSize.y);
	

	lineProgram->use();
	lineProgram->setVec2("gridSize", arenaSize);
	lineProgram->setVec3("colour", glm::vec3{ 1,0,0 });

	glBindVertexArray(lineVAO);
	glDrawArrays(GL_LINES, 0, 4 * arenaSize.x * arenaSize.y);

	glBindVertexArray(0);
}