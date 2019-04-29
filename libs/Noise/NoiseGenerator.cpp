#include <Noise/NoiseGenerator.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
NoiseGenerator::NoiseGenerator() {}
NoiseGenerator::~NoiseGenerator() {}
float NoiseGenerator::remap(float originalValue, float originalMin,
                            float originalMax, float newMin, float newMax)
{
  return newMin +
         (((originalValue - originalMin) / (originalMax - originalMin)) *
          (newMax - newMin));
}
float NoiseGenerator::smoothstep(float edge0, float edge1, float x) {
  float t = std::min(std::max((x - edge0) / (edge1 - edge0), 0.0f), 1.0f);
  return t * t * (3.0 - 2.0 * t);
}
float NoiseGenerator::PerlinNoise(const glm::vec3 &pIn, float frequency,
                                  int octaveCount) {
  const float octaveFrenquencyFactor =
      2; // noise frequency factor between octave, forced to 2
  // Compute the sum for each octave
  float sum = 0.0f;
  float weightSum = 0.0f;
  float weight = 0.5f;
  for (int oct = 0; oct < octaveCount; oct++) {
    // Perlin vec3 is bugged in GLM on the Z axis :(, black stripes are visible
    // So instead we use 4d Perlin and only use xyz...
    // glm::vec3 p(x * freq, y * freq, z * freq);
    // float val = glm::perlin(p, glm::vec3(freq)) *0.5 + 0.5;
    glm::vec4 p = glm::vec4(pIn.x, pIn.y, pIn.z, 0.0f) * glm::vec4(frequency);
    float val = glm::perlin(p, glm::vec4(frequency));
    sum += val * weight;
    weightSum += weight;
    weight *= weight;
    frequency *= octaveFrenquencyFactor;
  }
  float noise = (sum / weightSum) * 0.5f + 0.5f;
  noise = std::fminf(noise, 1.0f);
  noise = std::fmaxf(noise, 0.0f);
  return noise;
}
float NoiseGenerator::hash(float n)
{
	return glm::fract(sin(n + 1.951f) * 43758.5453f);
}
// hash based 3d value noise
float NoiseGenerator::noise(const glm::vec3& x)
{
	glm::vec3 p = glm::floor(x);
	glm::vec3 f = glm::fract(x);
	f = f * f*(glm::vec3(3.0f) - glm::vec3(2.0f) * f);
	float n = p.x + p.y*57.0f + 113.0f*p.z;
	return glm::mix(
		glm::mix(
			glm::mix(hash(n + 0.0f), hash(n + 1.0f), f.x),
			glm::mix(hash(n + 57.0f), hash(n + 58.0f), f.x),
			f.y),
		glm::mix(
			glm::mix(hash(n + 113.0f), hash(n + 114.0f), f.x),
			glm::mix(hash(n + 170.0f), hash(n + 171.0f), f.x),
			f.y),
		f.z);
}
float NoiseGenerator::Cells(const glm::vec3& p, float cellCount)
{
	const glm::vec3 pCell = p * cellCount;
	float d = 1.0e10;
	for (int xo = -1; xo <= 1; xo++)
	{
		for (int yo = -1; yo <= 1; yo++)
		{
			for (int zo = -1; zo <= 1; zo++)
			{
				glm::vec3 tp = glm::floor(pCell) + glm::vec3(xo, yo, zo);
				tp = pCell - tp - noise(glm::mod(tp, cellCount / 1));
				d = glm::min(d, dot(tp, tp));
			}
		}
	}
	d = std::fminf(d, 1.0f);
	d = std::fmaxf(d, 0.0f);
	return d;
}
float NoiseGenerator::WorleyNoise(const glm::vec3& p, float cellCount)
{
	return Cells(p, cellCount);
}
void NoiseGenerator::generateNoises() {
  //std::cout << "Generating weather Noise 1024x1024 RGB" << std::endl;
  //char *NoiseArray = new char[1024 * 1024 * 3];
  //for (int i = 0; i < 1024 * 1024 * 3; i += 3) {
  //  glm::vec3 pos = glm::vec3(float((i / 3) % 1024) / 1024,
  //                            float((i / 3) / 1024) / 1024, 0.051);
  //  glm::vec3 offset1 = glm::vec3(0.0, 0.0, 581.163);
  //  glm::vec3 offset2 = glm::vec3(0.0, 0.0, 1245.463);
  //  glm::vec3 offset3 = glm::vec3(0.0, 0.0, 2245.863);
  //  float perlinNoise = PerlinNoise(pos, 8, 3);
  //  float perlinNoise2 = PerlinNoise(pos + offset1, 8, 3);
  //  float perlinNoise3 = PerlinNoise(pos + offset2, 2, 3);
  //  // float perlinNoise4 = Tileable3dNoise::PerlinNoise(pos+offset3, 4, 3);
  //  perlinNoise3 = std::min(1.0, (smoothstep(0.45, 0.8, perlinNoise3) +
  //                                smoothstep(0.25, 0.45, perlinNoise3) * 0.5));
  //  NoiseArray[i] = char(perlinNoise * 128.0 + 127.0);
  //  NoiseArray[i + 1] = char(smoothstep(0.5, 0.7, perlinNoise2) * 255.0);
  //  NoiseArray[i + 2] = char(perlinNoise3 * 255.0);
  //}
  //stbi_write_bmp(CLOUDS_RESOURCES_PATH "/weather.bmp", 1024, 1024, 3, NoiseArray);
  //delete NoiseArray;


  //// worley and perlin-worley are from github/sebh/TileableVolumeNoise
  //// which is in turn based on noise described in 'real time rendering of
  //// volumetric cloudscapes for horizon zero dawn'
  //std::cout << "Generating Worley Noise 32x32x32 RGB" << std::endl;
  //char *worlNoiseArray = new char[32 * 32 * 32 * 3];
  //for (int i = 0; i < 32 * 32 * 32 * 3; i += 3) {
  //  glm::vec3 pos =
  //      glm::vec3(float((i / 3) % 32) / 32.0, float(((i / 3) / 32) % 32) / 32.0,
  //                float((i / 3) / (32 * 32)) / 32.0);
  //  float cell0 = 1.0f - WorleyNoise(pos, 2);
  //  float cell1 = 1.0f - WorleyNoise(pos, 4);
  //  float cell2 = 1.0f - WorleyNoise(pos, 8);
  //  float cell3 = 1.0f - WorleyNoise(pos, 16);
  //  float cellFBM0 = cell0 * 0.5f + cell1 * 0.35f + cell2 * 0.15f;
  //  float cellFBM1 = cell1 * 0.5f + cell2 * 0.35f + cell3 * 0.15f;
  //  float cellFBM2 = cell2 * 0.75f +
  //                   cell3 * 0.25f; // cellCount=4 -> worleyNoise4 is just noise
  //                                  // due to sampling frequency=texel freque.
  //                                  // So only take into account 2 frequenciM
  //  worlNoiseArray[i] = char(cellFBM0 * 255);
  //  worlNoiseArray[i + 1] = char(cellFBM1 * 255);
  //  worlNoiseArray[i + 2] = char(cellFBM2 * 255);
  //}
  //stbi_write_bmp(CLOUDS_RESOURCES_PATH "/worlnoise.bmp", 32 * 32, 32, 3, worlNoiseArray);
  //delete worlNoiseArray;

  std::cout << "Generating weather Noise 1024x1024 RGB" << std::endl;
  char* testArray = new char[512 * 512 * 3];
  for(int i = 0; i < 512 * 512 * 3; i += 3)
  {
      glm::vec3 pos          = glm::vec3(float((i / 3) % 512) / 512, float((i / 3) / 512) / 512, 0.051);
      float cell0 = 1.0f - WorleyNoise(pos, 2);
      float cell1 = 1.0f - WorleyNoise(pos, 4);
      float cell2 = 1.0f - WorleyNoise(pos, 8);
      float cell3 = 1.0f - WorleyNoise(pos, 16);
        float cellFBM0 = cell0 * 0.5f + cell1 * 0.35f + cell2 * 0.15f;
        float cellFBM1 = cell1 * 0.5f + cell2 * 0.35f + cell3 * 0.15f;
        float cellFBM2 = cell2 * 0.75f +
                         cell3 * 0.25f;
        testArray[i]         = char(cellFBM2 * 255);
        testArray[i + 1] = char(cellFBM2 * 255);
        testArray[i + 2]     = char(cellFBM2 * 255);
  }
  stbi_write_bmp(CLOUDS_RESOURCES_PATH "/test.bmp", 512, 512, 3, testArray);
  delete testArray;
  /*
  std::cout << "Generating Perlin-Worley Noise 128x128x128 RGBA" << std::endl;
  char *perlWorlNoiseArray = new char[128 * 128 * 128 * 4];
  for (int i = 0; i < 128 * 128 * 128 * 4; i += 4) {
    glm::vec3 pos = glm::vec3(float((i / 4) % 128) / 128.0,
                              float(((i / 4) / 128) % 128) / 128.0,
                              float((i / 4) / (128 * 128)) / 128.0);
    // Perlin FBM noise
    float perlinNoise = PerlinNoise(pos, 8, 3);
    const float worleyNoise00 = (1.0f - WorleyNoise(pos, 8));
    const float worleyNoise01 = (1.0f - WorleyNoise(pos, 32));
    const float worleyNoise02 = (1.0f - WorleyNoise(pos, 56));
    // const float worleyNoise3 = (1.0f - Tileable3dNoise::WorleyNoise(coord,
    // 80));
    // const float worleyNoise4 = (1.0f - Tileable3dNoise::WorleyNoise(coord,
    // 104));
    // const float worleyNoise5 = (1.0f - Tileable3dNoise::WorleyNoise(coord,
    // 128));	// half the frequency of texel, we should not go further (with
    // cellCount = 32 and texture size = 64)
    // PerlinWorley noise as described p.101 of GPU Pro 7
    float worleyFBM =
        worleyNoise00 * 0.625f + worleyNoise01 * 0.25f + worleyNoise02 * 0.125f;
    float PerlWorlNoise = remap(perlinNoise, 0.0, 1.0, worleyFBM, 1.0);
    // float worleyNoise0 = (1.0f - Tileable3dNoise::WorleyNoise(coord, 4));
    // float worleyNoise1 = (1.0f - Tileable3dNoise::WorleyNoise(coord, 8));
    float worleyNoise12 = (1.0f - WorleyNoise(pos, 16));
    // float worleyNoise3 = (1.0f - Tileable3dNoise::WorleyNoise(coord, 32));
    float worleyNoise14 = (1.0f - WorleyNoise(pos, 64));
    // Three frequency of Worley FBM noise
    float worleyFBM0 =
        worleyNoise00 * 0.625f + worleyNoise12 * 0.25f + worleyNoise01 * 0.125f;
    float worleyFBM1 =
        worleyNoise12 * 0.625f + worleyNoise01 * 0.25f + worleyNoise14 * 0.125f;
    float worleyFBM2 =
        worleyNoise01 * 0.75f +
        worleyNoise14 * 0.25f; // cellCount=4 -> worleyNoise5 is just noise due
                               // to sampling frequency=texel frequency. So only
                               // take into account 2 frequencies for FBM
    perlWorlNoiseArray[i] = char(PerlWorlNoise * 255);
    perlWorlNoiseArray[i + 1] = char(worleyFBM0 * 255);
    perlWorlNoiseArray[i + 2] = char(worleyFBM1 * 255);
    perlWorlNoiseArray[i + 3] = char(worleyFBM2 * 255);
  }
  stbi_write_tga(CLOUDS_RESOURCES_PATH "/perlworlnoise.tga", 128 * 128, 128, 4,
                 perlWorlNoiseArray);
  delete perlWorlNoiseArray;*/
}
