#include <glm.hpp>

class Program;

struct line
{
	glm::vec2 A;
	glm::vec2 B;
};

struct triangle
{
	glm::vec2 A;
	glm::vec2 B;
	glm::vec2 C;
};

class TerrainManager
{
public:
	TerrainManager(glm::ivec2 arenaSize);	//Take in the size of the arena
	~TerrainManager();

	void uploadStage(float* stage);		//Upload a stage, to overwrite old one

	//Terrain destruction
	void modifyTerrainCircle(glm::vec2 centre, float radius, float value);	//Adds value to each point within the circle, scaled based on distance

	void render();	//Draw the terrain to the screen

	glm::ivec2 getArenaSize() { return arenaSize; }
protected:
	glm::ivec2 arenaSize;	//Size of the arena (number of squares)

	float* scalarData;
	int scalarDataSize;

	line* lineArray;	//2 lines max per square in arena

	unsigned int triangleVAO;	//Buffers for triangle mesh
	unsigned int triangleVBO;

	unsigned int lineVAO;		//Buffers for line mesh
	unsigned int lineVBO;

	Program* triangleProgram;
	Program* lineProgram;

	float getPoint(glm::ivec2 pos);				//Interact with specific points, used internally
	void setPoint(glm::ivec2 pos, float val);	
	void addPoint(glm::ivec2 pos, float val);

	void generateTerrain(int x, int y, int width, int height);	//Generate terrain of an area, width and height are in squares
};