#version 330 core

layout(location=0) in vec2 vertexPosition;

layout(std140) uniform terrainData
{
	ivec2 gridSize;
};

uniform vec2 position;
uniform vec2 size;

out vec2 textCoord;

void main()
{
	
	vec2 playerPos=vertexPosition*size+position;
	vec2 cellSize=2/vec2(gridSize);

	vec2 screenPos=vec2(playerPos.x*cellSize.x,playerPos.y*cellSize.y)-1;

	gl_Position=vec4(screenPos,0,1);

	textCoord=vertexPosition;
}