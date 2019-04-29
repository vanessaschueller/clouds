#pragma once

#include <Includes/Defs.h>


class NoiseGenerator
{
public:
	NoiseGenerator();
	~NoiseGenerator();

	void generateNoises();

private:
	float hash(float n);
	float noise(const glm::vec3& x);
	float Cells(const glm::vec3& p, float cellCount);
	float WorleyNoise(const glm::vec3& p, float cellCount);
	float PerlinNoise(const glm::vec3& pIn, float frequency, int octaveCount);
	float remap(float origVal, float origMin, float origMax, float newMin, float newMax);
	float smoothstep(float edge0, float edge1, float x);


};