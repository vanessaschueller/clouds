#version 330

in vec2 TexCoords;
out vec4 pixelColor;

uniform bool  showGodrays;
uniform int   numSamples;
uniform float density, weight, decay, exposure;
uniform vec3 sunPositionWorld, cameraPosition;
uniform mat4 view, projection;
uniform sampler2D cloudOutputTexture, alphaTexture;

vec3 uncharted2Tonemap(const vec3 x)
{
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 tonemapUncharted2(const vec3 color)
{
    const float W            = 11.2;
    const float exposureBias = 2.0;
    vec3        curr         = uncharted2Tonemap(exposureBias * color);
    vec3        whiteScale   = 1.0 / uncharted2Tonemap(vec3(W));
    return curr * whiteScale;
}
void main()
{
    vec2 uv               = TexCoords;
    vec4 originalColor    = texture(cloudOutputTexture, uv);
    vec4 originalColorMod = vec4(tonemapUncharted2(originalColor.rgb), 1.0);

    if(max(originalColor.r, max(originalColor.g, originalColor.b)) < 0.2)
        originalColorMod = min(originalColor, originalColorMod);

    if(showGodrays)
    {
        vec3 cameraLookAt = -normalize(vec3(view[0][2], view[1][2], view[2][2]));
        vec3 sunToCamera  = normalize(sunPositionWorld);

        float blend = dot(sunToCamera, cameraLookAt);
        if(blend < 0.f)
            pixelColor = originalColorMod;
        else
        {
            vec4 sunposNDC     = projection * view * vec4(sunPositionWorld + cameraPosition,
                                                      1.0f); // sunposition in normalized device coordinates
            vec2 sunposSS      = clamp((sunposNDC.xy + 1.0f) / 2.f, 0.f,
                                  1.f); // sunposition in screenspace
            vec2 deltaTexCoord = ((uv - sunposSS) / float(numSamples)) * density;

            float illuminationDecay = 1.0f;
            vec3  accumulatedColor  = vec3(0.f);

            for(int i = 0; i < numSamples; i++)
            {
                vec4 sampleColor = originalColor;
                sampleColor.rgb *= 1 - texture(alphaTexture, uv).r; // only compute where cloudedges appear
                sampleColor *= illuminationDecay * weight;
                illuminationDecay *= decay;
                accumulatedColor += sampleColor.rgb;
                uv -= deltaTexCoord;
            }
            vec4 finalColor = originalColorMod + vec4(accumulatedColor * exposure, 1.0f) * blend;
            pixelColor      = finalColor;
        }
    }
    else
    {
        pixelColor = originalColorMod;
    }
}
