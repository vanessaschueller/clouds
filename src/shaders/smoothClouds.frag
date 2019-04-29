#version 430

in vec2 TexCoords;

uniform bool smoothClouds;
uniform vec2 resolution;
uniform sampler2D cloudColorTexture;
uniform sampler2D cloudDepthTexture;
uniform sampler2D currentDensityMap;

layout(location = 0) out vec4 pixelColor;

#define SIGMA 5.0
#define BSIGMA 0.1
#define MSIZE 15

float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);
float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);
float normpdf(in float x, in float sigma) { return 0.39894 * exp(-0.5 * x * x / (sigma * sigma)) / sigma; }

float normpdf3(in vec3 v, in float sigma) { return 0.39894 * exp(-0.5 * dot(v, v) / (sigma * sigma)) / sigma; }

void main(void)
{        
	vec4 gaussianColor;
    vec4 bilateralColor;
    if(smoothClouds && texture(currentDensityMap,TexCoords).r > 0)
    {
        vec3 c = texture(cloudColorTexture, vec2(0.0, 0.0) + (gl_FragCoord.xy / resolution)).rgb;

        // declare stuff
        const int kSize = (MSIZE - 1) / 2;
        float     kernel[MSIZE];
        vec3      final_colour = vec3(0.0);

        // create the 1-D kernel
        float Z = 0.0;
        for(int j = 0; j <= kSize; ++j)
        {
            kernel[kSize + j] = kernel[kSize - j] = normpdf(float(j), SIGMA);
        }

        vec3  cc;
        float factor;
        float bZ = 1.0 / normpdf(0.0, BSIGMA);
        // read out the texels
        for(int i = -kSize; i <= kSize; ++i)
        {
            for(int j = -kSize; j <= kSize; ++j)
            {
                cc     = texture(cloudColorTexture, vec2(0.0, 0.0) + (gl_FragCoord.xy + vec2(float(i), float(j))) / resolution).rgb;
                factor = normpdf3(cc - c, BSIGMA) * bZ * kernel[kSize + j] * kernel[kSize + i];
                Z += factor;
                final_colour += factor * cc;
            }
        }

        pixelColor = mix(texture(cloudColorTexture, TexCoords), vec4(final_colour / Z, 1.0), texture(currentDensityMap, TexCoords).r);
    }
    else
        pixelColor = texture(cloudColorTexture, TexCoords);
}
