#version 330

/*
 * IN & OUTS
 */
in vec2 TexCoords;

layout(location = 0) out vec4 fragmentColor;
layout(location = 1) out vec4 depthBuffer;
layout(location = 2) out vec4 alphaness;
layout(location = 3) out vec4 densityMap;

/*
 * CAM & SCENE
 */
uniform int   frameCounter;
uniform float fogAmount;
uniform float iTime;
uniform vec2 iResolution;
uniform vec3 camPos;
uniform mat4 invProj, invView, viewMatrix, projMatrix;
uniform bool onlyFirstPass;

/*
 * SAMPLING
 */
uniform int maxSample, minSample;
uniform int stepCount;

/*
 * CLOUD PARAMS
 */
uniform float cloudSpeed;
uniform float coverageMultiplier, coverageMultiplierHigh, erosionMultiplier;
uniform float strat, stratcum, cum, absorption;
uniform bool  useWind, usePowder, useBeerLaw, useScattering, highRes, nocutscene;
uniform bool  userCloudType, densHeight, weather;
uniform vec3 windDirection;

/*
 * CLOUD TEXTURES
 */
uniform sampler2D weatherMap, depthTexture, colorTexture, highClouds, highClouds2, highClouds3, turbulence, blueNoise;
uniform sampler3D cloudBase, erodeCloud, flowTexture, sdfTexture;

/*
 * LIGHT
 */
uniform bool  lensFlare;
uniform bool  addSunGlare;
uniform float silverIntensity, silverSpread, eccentricity;
uniform float betaM, sunpowerMie, sunpowerRayleigh;
uniform vec3 lightDir;
uniform vec3 sunColor;

/*
 * MESH
 */
uniform vec3 bbmin;
uniform vec3 bbmax;
uniform vec3 voxelRes;
uniform vec3 moveDirection;

/*
 * RADII
 */
#define EARTH_RADIUS (700000.)
#define SPHERE_INNER_RADIUS (EARTH_RADIUS + 10000.0)
#define SPHERE_OUTER_RADIUS (SPHERE_INNER_RADIUS + 17000.0)
#define SKYDOME_RADIUS (SPHERE_OUTER_RADIUS + 5000.0)
#define SPHERE_DELTA float(SPHERE_OUTER_RADIUS - SPHERE_INNER_RADIUS)

/*
 * GRADIENT
 */
#define STRATUS_GRADIENT vec4(0.0, 0.1, 0.2, 0.3)
#define STRATOCUMULUS_GRADIENT vec4(0.02, 0.2, 0.48, 0.625)
#define CUMULUS_GRADIENT vec4(0.00, 0.1625, 0.88, 0.98)

/*
 * CLOUDS
 */
#define CLOUDS_MIN_TRANSMITTANCE 1e-1
#define CLOUD_SCALE 40.0
#define CLOUD_TOP_OFFSET 500.0

/*
 * GLOBAL
 */
#define M_PI 3.1415926535897932384626433832795
float e             = 2.7182818284590452353602874713527f;
float surfaceHeight = 0.99f;
float intensity     = 1.8f;
float numSamples;
vec3  kr           = vec3(0.18867780436772762f, 0.4978442963618773f, 0.6616065586417131f); // air
vec3  sphereCenter = vec3(0.0f, -EARTH_RADIUS, 0.0f);
float dotMoveWind  = dot(normalize(moveDirection), -windDirection);

/*
 * CONE SAMPLING OFFSETS
 */
uniform vec3 noiseKernel[6u] =
        vec3[](vec3(0.38051305f, 0.92453449f, -0.02111345f), vec3(-0.50625799f, -0.03590792f, -0.86163418f), vec3(-0.32509218f, -0.94557439f, 0.01428793f),
               vec3(0.09026238f, -0.27376545f, 0.95755165f), vec3(0.28128598f, 0.42443639f, -0.86065785f), vec3(-0.16852403f, 0.14748697f, 0.97460106f));
#define BAYER_FACTOR 1.0 / 16.0
uniform float bayerFilter[16u] =
        float[](0.0f * BAYER_FACTOR, 8.0f * BAYER_FACTOR, 2.0f * BAYER_FACTOR, 10.0f * BAYER_FACTOR, 12.0f * BAYER_FACTOR, 4.0f * BAYER_FACTOR,
                14.0f * BAYER_FACTOR, 6.0f * BAYER_FACTOR, 3.0f * BAYER_FACTOR, 11.0f * BAYER_FACTOR, 1.0f * BAYER_FACTOR, 9.0f * BAYER_FACTOR,
                15.0f * BAYER_FACTOR, 7.0f * BAYER_FACTOR, 13.0f * BAYER_FACTOR, 5.0f * BAYER_FACTOR);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// HELP
// FUNCTIONS////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * SIMPLE THRESHOLD FUNCTION
 */
float threshold(float v, float t) { return v > t ? v : 0.0f; }

/*
 * MAPS OLD VALUE FROM OLD RANGE TO NEW RANGE
 */
float remap(float origValue, float origMin, float origMax, float newMin, float newMax)
{
    return newMin + (((origValue - origMin) / (origMax - origMin)) * (newMax - newMin));
}

/*
 * COMPUTE ROTATION MATRIX AROUND Y
 */
mat3 rotateY(float rad)
{
    float c = cos(rad);
    float s = sin(rad);
    return mat3(c, 0.0, s, 0.0, 1.0, 0.0, -s, 0.0, c);
}

// SAMPLING HELP
// FUNCTIONS////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * GET HEIGHT VALUE
 */
float getHeightFractionForPoint(vec3 inPosition)
{
    return clamp((length(inPosition - sphereCenter) - SPHERE_INNER_RADIUS) / (SPHERE_DELTA), 0.0f, 1.0f);
}

/*
 * COMPUTE COORDINATES IN IMG SPACE
 */
vec3 computeImgSpaceCoord()
{
    vec2 ray = 2.0f * gl_FragCoord.xy / iResolution.xy - 1.0f;
    return vec3(ray, 1.0f);
}

/*
 * GET UV COORDINATE FROM 3D VECTOR
 */
vec2 getUVProjection(vec3 p) { return p.xz / SPHERE_INNER_RADIUS + 0.5f; }

/*
 * DETERMINES DENSITY DEPENDING ON HEIGHT AND CLOUD TYPE
 */
float getDensityHeightGradient(float heightFraction, float cloudType)
{
    vec4 mixed;
    if(userCloudType)
    {
        mixed = STRATUS_GRADIENT * strat + STRATOCUMULUS_GRADIENT * stratcum + CUMULUS_GRADIENT * cum;
    }
    else
    {
        float stratus       = 1.0f - clamp(cloudType * 2.0f, 0.f, 1.f);
        float stratocumulus = 1.0f - abs(cloudType - 0.5f) * 2.0f;
        float cumulus       = clamp(cloudType - 0.5f, 0.0f, 1.f) * 2.0f;
        mixed               = STRATUS_GRADIENT * stratus + STRATOCUMULUS_GRADIENT * stratocumulus + CUMULUS_GRADIENT * cumulus;
    }
    return smoothstep(mixed.x, mixed.y, heightFraction) - smoothstep(mixed.z, mixed.w, heightFraction);
}

/*
 * COMPUTE START AND END SAMPLING POINT
 */
bool raySphereintersection(vec3 ro, vec3 rd, float radius, out vec3 startPos)
{
    float t;
    sphereCenter.xz = camPos.xz;
    float radius2   = radius * radius;
    vec3  L         = ro - sphereCenter;
    float a         = dot(rd, rd);
    float b         = 2.0f * dot(rd, L);
    float c         = dot(L, L) - radius2;
    float discr     = b * b - 4.0f * a * c;
    if(discr < 0.0f)
        return false;
    t = max(0.0f, (-b + sqrt(discr)) / 2.f);
    if(t == 0.0f)
    {
        return false;
    }
    startPos = ro + rd * t;
    return true;
}

// LIGHT
// FUNCTIONS//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vec3 cc(vec3 color, float factor, float factor2) // color modifier
{
    float w = color.x + color.y + color.z;
    return mix(color, vec3(w) * factor, w * factor2);
}
/*
 * ADD LENSFLARE
 */
vec3 lensflare(vec2 uv)
{
    uv.x *= iResolution.x / iResolution.y; // fix aspect ratio

    vec2 pos = (projMatrix * viewMatrix * vec4(lightDir + camPos, 1.0f)).xy;
    pos.x *= iResolution.x / iResolution.y;
    vec2 uvd = uv * (length(uv));

    float f0 = 1.0f / (length(uv - pos) * 32.0f + 1.0f);

    // LOWEST GHOST
    float f2  = max(1.0f / (1.0f + 32.0f * pow(length(uvd + 0.8f * pos), 2.0f)), .0f) * 0.25f;
    float f22 = max(1.0f / (1.0f + 32.0f * pow(length(uvd + 0.85f * pos), 2.0f)), .0f) * 0.23f;
    float f23 = max(1.0f / (1.0f + 32.0f * pow(length(uvd + 0.9f * pos), 2.0f)), .0f) * 0.21f;

    vec2 uvx = mix(uv, uvd, -0.5f);
    // SURROUNDING MID GHOST
    float f4  = max(0.01f - pow(length(uvx + 0.4f * pos), 2.4f), .0f) * 6.0f;
    float f42 = max(0.01f - pow(length(uvx + 0.45f * pos), 2.4f), .0f) * 5.0f;
    float f43 = max(0.01f - pow(length(uvx + 0.5f * pos), 2.4f), .0f) * 3.0f;

    uvx = mix(uv, uvd, -.4f);
    // MID GHOST
    float f5  = max(0.01f - pow(length(uvx + 0.2f * pos), 5.5f), .0f) * 2.0f;
    float f52 = max(0.01f - pow(length(uvx + 0.4f * pos), 5.5f), .0f) * 2.0f;
    float f53 = max(0.01f - pow(length(uvx + 0.6f * pos), 5.5f), .0f) * 2.0f;

    uvx = mix(uv, uvd, -0.5f);
    // HIGHEST GHOST
    float f6  = max(0.01f - pow(length(uvx - 0.3f * pos), 1.6f), .0f) * 6.0f;
    float f62 = max(0.01f - pow(length(uvx - 0.325f * pos), 1.6f), .0f) * 3.0f;
    float f63 = max(0.01f - pow(length(uvx - 0.35f * pos), 1.6f), .0f) * 5.0f;

    vec3 c = vec3(.0f);

    c.r += f2 + f4 + f5 + f6;
    c.g += f22 + f42 + f52 + f62;
    c.b += f23 + f43 + f53 + f63;
    c = c * 1.3f - vec3(length(uvd) * .05f);
    c += vec3(f0);

    return c;
}

/*
 * COMPUTE AMOUNT OF FOG
 */
float computeFogAmount(in vec3 startPos, in float factor)
{
    float dist   = length(startPos - camPos);
    float radius = (EARTH_RADIUS)*0.3f;
    float alpha  = (dist / radius);
    return (1.f - exp(-dist * alpha * factor));
}

/*
 * RAYLEIGH FUNCTION
 */
float rayleigh(float cosAngle) { return 3.f / (16.f * M_PI) * (1.f + cosAngle * cosAngle); }

/*
 * HENYEY GREENSTEIN FUNCTION
 */
float HenyeyGreenstein(float cosAngle, float inG)
{
    return ((1.0f - inG * inG) / pow((1.0f + inG * inG - 2.0f * inG * cosAngle), 3.0f / 2.0f)) / 4.0f * M_PI;
}

/*
 * ATMOSPHERIC SCATTERING
 */
vec4 computeSkyColor(vec3 ray, float dotLightRay)
{
    vec3  betaR          = vec3(5.8f, 13.5f, 33.1f) * pow(10.f, -6.f);
    vec3  betaMV         = vec3(betaM) * pow(10.f, -5.f);
    float s              = length(ray);
    float theta          = clamp(dotLightRay, 0.0f, 1.0f);
    float rayleighCoeff  = rayleigh(theta);
    float mieCoeff       = HenyeyGreenstein(theta, 0.82f);
    vec3  L_InScattering = (rayleighCoeff * sunpowerRayleigh + mieCoeff * sunpowerMie) + sunColor * (1.f - exp(-(betaR + betaMV) * s));
    return vec4(L_InScattering, 1.0f);
}

// WIND
// DIRECTION/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * CHECK IF SAMPLE POINT IS INSIDE BB
 */
bool isInsideBox(vec3 pos)
{
    if(bbmin.x <= pos.x && pos.x <= bbmax.x && bbmin.y <= pos.y && pos.y <= bbmax.y && bbmin.z <= pos.z && pos.z <= bbmax.z)
    {
        return true;
    }
    return false;
}

/*
 * COMPUTE FLOW TAKING MOVEMENT INTO ACCOUNT
 */
float computeFlow(vec3 pos, float dotMoveWind)
{
    float xLookUp         = remap(pos.x, bbmin.x, bbmax.x, 2.f, voxelRes.x - 2.f) / voxelRes.x;
    float yLookUp         = remap(pos.y, bbmin.y, bbmax.y, 2.f, voxelRes.y - 2.f) / voxelRes.y;
    float zLookUp         = remap(pos.z, bbmin.z, bbmax.z, 2.f, voxelRes.z - 2.f) / voxelRes.z;
    float flowStrength    = texture(flowTexture, vec3(xLookUp, yLookUp, zLookUp)).r;
    vec3  gradient        = texture(sdfTexture, vec3(xLookUp, yLookUp, zLookUp)).rgb;
    float dotGradWind     = dot(gradient, -windDirection);
    float dotGradWindPlus = dot(gradient, windDirection);
    float lengthMove      = length(moveDirection);

    if(dotMoveWind < 0)
    {
        if(dotGradWind < 0)
        {
            flowStrength = mix(flowStrength, mix(flowStrength, 1.f, abs(dotGradWind)), lengthMove);
        }
    }
    else if(dotMoveWind > 0)
    {
        if(dotGradWind > 0 && length(moveDirection) * 1000.f >= cloudSpeed)
        {
            flowStrength = mix(flowStrength, mix(flowStrength, 1.f, abs(dotGradWindPlus)), lengthMove);
        }
        else if(length(moveDirection) * 1000 < cloudSpeed)
        {
            flowStrength = mix(flowStrength, mix(flowStrength, 1.f, abs(dotGradWindPlus)), lengthMove * 0.3f);
        }
    }
    else if(dotMoveWind == 0 && moveDirection != vec3(0.f) &&
            max(max(abs(moveDirection.x), abs(moveDirection.y)), abs(moveDirection.z)) != abs(moveDirection.y))
    {
        if(moveDirection.z > 0)
        {
            if(dot(gradient, normalize(-vec3(1.f, 0.f, 1.f))) < 0)
            {
                flowStrength = mix(flowStrength, mix(flowStrength, 1.f, abs(dot(gradient, normalize(-vec3(1.f, 0.f, 1.f))))), lengthMove);
            }
            if(dot(gradient, normalize(-vec3(0.f, 0.f, 1.f))) < 0)
            {
                flowStrength = mix(flowStrength, mix(flowStrength, 1.f, abs(dot(gradient, normalize(-vec3(0.f, 0.f, 1.f))))), lengthMove);
            }
        }
        if(moveDirection.z < 0)
        {
            if(dot(gradient, normalize(-vec3(1.f, 0.f, -1.f))) < 0)
            {
                flowStrength = mix(flowStrength, mix(flowStrength, 1.f, abs(dot(gradient, normalize(-vec3(1.f, 0.f, -1.f))))), lengthMove);
            }
            if(dot(gradient, normalize(-vec3(0.f, 0.f, -1.f))) < 0)
            {
                flowStrength = mix(flowStrength, mix(flowStrength, 1.f, abs(dot(gradient, normalize(-vec3(0.f, 0.f, -1.f))))), lengthMove);
            }
        }
        if(dotGradWind < 0)
        {
            flowStrength = mix(flowStrength, mix(flowStrength, 1.f, abs(dotGradWind)), length(moveDirection));
        }
    }
    return flowStrength;
}

// CLOUDS/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * SAMPLE DENSITY OF CLOUD
 */
float sampleCloudDensity(vec3 startPos, vec3 pos, vec4 weatherData, bool expensive, int mipmap)
{
    float height = getHeightFractionForPoint(pos);

    if(height <= 0.0f || height >= 1.0f)
    {
        return 0.f;
    }

    vec2  baseUV       = getUVProjection(pos);
    vec2  erodeUV      = getUVProjection(pos);
    float flowStrength = 1;
    vec3  basePos, erodePos;
    if(useWind)
    {
        if(isInsideBox(pos) && !nocutscene)
        {
            flowStrength = computeFlow(pos, dotMoveWind);
        }
        basePos  = pos + height * windDirection * 500.f;
        erodePos = basePos;
        basePos += (windDirection + vec3(0.0f, 0.1f, 0.0f)) * iTime * cloudSpeed;
        erodePos += (windDirection + vec3(0.0f, 0.1f, 0.0f)) * iTime * (cloudSpeed + 500.f);
        baseUV  = getUVProjection(basePos);
        erodeUV = getUVProjection(erodePos);
    }

    vec4  lowFrequencyNoises = texture(cloudBase, vec3(baseUV * CLOUD_SCALE, height), mipmap);
    float lowFrequFBM        = (lowFrequencyNoises.g * 0.625f) + (lowFrequencyNoises.b * 0.25f) + (lowFrequencyNoises.a * 0.125f);

    float baseCloud             = remap(lowFrequencyNoises.r, -(1.0f - lowFrequFBM), 1.0f, 0.0f, 1.0f);
    float densityHeightGradient = getDensityHeightGradient(height, weatherData.b);
    if(densHeight)
        baseCloud *= densityHeightGradient;

    float cloudCoverage = weatherData.r * coverageMultiplier * flowStrength; // use flowStrength to handle the cloud collision

    float baseCloudWithCoverage = baseCloud;
    if(weather)
    {
        baseCloudWithCoverage = remap(baseCloud, cloudCoverage, 1.0f, 0.0f, 1.0f);
        baseCloudWithCoverage *= cloudCoverage;
    }
    float finalCloud = baseCloudWithCoverage;

    if(finalCloud > 0 && expensive)
    {
        if(useWind)
            erodeUV += texture(turbulence, erodeUV, mipmap).xy * iTime / 8000.f;
        vec3  highFrequencyNoises    = texture(erodeCloud, vec3(erodeUV * CLOUD_SCALE, height) * 0.1f, mipmap).rgb;
        float highFrequFBM           = (highFrequencyNoises.r * 0.625f) + (highFrequencyNoises.g * 0.25f) + (highFrequencyNoises.b * 0.125f);
        float highFrequNoiseModifier = mix(highFrequFBM, 1.0f - highFrequFBM, clamp(height * 5.f, 0.0f, 1.0f));
        baseCloudWithCoverage -= erosionMultiplier * highFrequNoiseModifier * (1.0f - baseCloudWithCoverage);
        finalCloud = remap(baseCloudWithCoverage, highFrequNoiseModifier * 0.2f, 1.0f, 0.0f, 1.0f);
    }
    return clamp(finalCloud, 0.0f, 1.0f);
}

/*
 * POWDER EFFECT
 */
float powder(float lightSamples) { return (1. - exp(-lightSamples * 2.f)); }

// RAYMARCHING
// FUNCTIONS//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * RAYMARCH TO LIGHT SOURCE
 */
float raymarchToLight(vec3 o, float stepSize)
{
    vec3        startPos   = o;
    float       ds         = stepSize * 6.0f;
    vec3        rayStep    = lightDir * ds;
    const float CONE_STEP  = 1.0f / 6.0f;
    float       coneRadius = 1.0f;
    float       density    = 0.0f;
    vec3        pos;

    for(int i = 0; i < 6; i++)
    {
        pos          = startPos + coneRadius * noiseKernel[i] * float(i);
        float height = getHeightFractionForPoint(pos);
        if(height >= 0)
        {
            vec4  weatherData  = texture(weatherMap, getUVProjection(pos));
            float cloudDensity = sampleCloudDensity(startPos, pos, weatherData, density > 0.3f && highRes, i);
            if(cloudDensity == 0 || cloudDensity == -1)
                return density;
            else
            {
                if(density > 0.9f)
                    break;
                density += cloudDensity;
            }
        }
        startPos += rayStep;
        coneRadius += CONE_STEP;
    }
    return density;
}
vec3 sampleColor(vec3 pos, float cloudDensity, vec4 weatherData, float Ti, float dotLightRay, float stepSize)
{
    float height = getHeightFractionForPoint(pos);
    /*
     * RAYMARCH TO LIGHT SOURCE
     */
    float lightDensity = raymarchToLight(pos, stepSize * 0.1f);

    float absorptionFactor = weatherData.g + absorption;
    float beerLaw          = 1.f;
    if(useBeerLaw)
        beerLaw = max(exp(-lightDensity * absorptionFactor), (exp(-lightDensity * absorptionFactor * 0.25f) * 0.7f));
    /*
     * HENYEY GREENSTEIN DEPENDING ON LIGHT-CAMERA-ANGLE
     */

    float scattering = 1.f;
    if(useScattering)
        scattering = max(HenyeyGreenstein(dotLightRay, eccentricity),
                         silverIntensity * HenyeyGreenstein(dotLightRay, 0.99f - silverSpread)); // https://github.com/Aman-Sachan-asach/Meteoros

    /*
     * COMPUTE POWDER EFFECT
     */
    float powderTerm = 1.0f;
    if(usePowder)
        powderTerm = mix(powder(lightDensity), 1.0f, height);

    float lightEnergy = beerLaw * scattering * powderTerm * 2.f;
    /*
     * MIX SKYCOLOR, SUNCOLOR DEPENDING ON LIGHT TRANSMITTANCE
     */
    vec3 sampleColor = 0.6f * mix(sunColor, vec3(1.0f), lightDensity) * lightEnergy;
    sampleColor -= sampleColor * Ti;
    return sampleColor;
}
/*
 * RAYMARCHING
 */
vec4 raymarchToCloud(vec3 start, vec3 end, float dotLightRay, vec3 skyColor, vec4 objectColor)
{
    // compute length of path
    vec3  path        = end - start;
    float pathLength  = length(path);
    vec3  pathDir     = path / pathLength;
    vec3  pathDirLong = pathDir;
    int   a           = int(gl_FragCoord.x) % 4;
    int   b           = int(gl_FragCoord.y) % 4;
    start += texture(blueNoise, getUVProjection(start)).xyz; // pathDir * bayerFilter[a * 4 + b];
    vec3 pos = start;

    // adaptive number of steps

    float nSteps = mix(float(minSample), float(maxSample), clamp(((pathLength - SPHERE_DELTA) / SPHERE_DELTA), 0.f, 1.f));
    numSamples   = nSteps;
    // compute stepsize
    float stepSize       = pathLength / nSteps;
    float biggerStepSize = (pathLength / nSteps) * 1.1f;
    // set path to length of one step
    pathDir *= stepSize;
    pathDirLong *= biggerStepSize;
    // parameters needed for computation
    float       density       = 0.0f;
    float       transmittance = 1.0f;
    const float absorb        = 0.01f;
    float       sigmaDs       = stepSize * absorb;

    // color to return
    vec4 col = vec4(0.0f);

    vec3  ambientlight = vec3(255.f, 255.f, 235.f) / 255.f;
    float Ti;
    bool  firstHit   = false;
    int   takenSteps = 0;
    float dist       = 1000.f;
    for(int i = 0; i < nSteps; ++i)
    {
        dist = 1000.f;
        if(col.a >= 0.99f || density >= 0.99f)
        {
            return col;
        }
        if(isInsideBox(pos) && !nocutscene)
        {
            float xLookUp = remap(pos.x, bbmin.x, bbmax.x, 0, voxelRes.x) / voxelRes.x;
            float yLookUp = remap(pos.y, bbmin.y, bbmax.y, 0, voxelRes.y) / voxelRes.y;
            float zLookUp = remap(pos.z, bbmin.z, bbmax.z, 0, voxelRes.z) / voxelRes.z;
            float dist    = texture(sdfTexture, vec3(xLookUp, yLookUp, zLookUp)).a;
            if(dist < 10.f && objectColor.a > 0)
            {
                col.rgb += transmittance * (objectColor.rgb);
                densityMap = vec4(density, 0.f, 0.f, 1.f);
                if(!firstHit)
                {
                    float distToCam = (-(viewMatrix * vec4(pos, 1.0f)).z - 0.1f) / (100000.f - .1f);
                    depthBuffer     = vec4(vec3(distToCam), 1);
                    firstHit        = true;
                }
                return vec4(col.rgb, 1);
            }
        }

        vec4  weatherData  = texture(weatherMap, getUVProjection(pos));
        float cloudDensity = sampleCloudDensity(start, pos, weatherData, highRes, 0);

        if(cloudDensity > 0 && dist > 100)
        {
            if(!firstHit)
            {
                float distToCam = (-(viewMatrix * vec4(pos, 1.0f)).z - 0.1f) / (100000.f - .1f);
                depthBuffer     = vec4(vec3(distToCam), 1);
                firstHit        = true;
            }

            /*
             * BEER LAW
             */
            Ti = exp(-cloudDensity * sigmaDs); // transmittance at sample position

            /*
             * COMPUTE FINAL COLOR
             */

            col.rgb += transmittance * sampleColor(pos, cloudDensity, weatherData, Ti, dotLightRay, stepSize);
            col.a = 1.0f - transmittance;

            /*
             * UPDATE TOTAL TRANSMITTANCE
             */
            density += cloudDensity;
            transmittance *= Ti; // total transmittance

            if(transmittance <= CLOUDS_MIN_TRANSMITTANCE)
                break;

            takenSteps++;
            pos += pathDir;
        }
        else
        {
            pos += pathDirLong;
        }
    }

    // determine alpha value
    col.a      = (1.0f - transmittance);
    densityMap = vec4(density, 0, 0, 1);
    return col;
}

/*
 * TRACE OBJECT IN LOWER ATMOSPHERE LAYER
 */
vec4 raymarchToObject(vec3 start, vec3 end, vec3 skyColor, vec4 objectColor)
{
    // compute length of path
    vec3  path       = end - start;
    float pathLength = length(path);
    vec3  pathDir    = path / pathLength;
    int   a          = int(gl_FragCoord.x) % 4;
    int   b          = int(gl_FragCoord.y) % 4;
    start += pathDir * bayerFilter[a * 4 + b];
    vec3 pos = start;

    // adaptive number of steps
    float nSteps = mix(float(minSample), float(256), clamp(((pathLength - SPHERE_DELTA) / SPHERE_DELTA), 0.f, 1.f));

    // compute stepsize
    float stepSize = pathLength / nSteps;
    // set path to length of one step
    pathDir *= stepSize;

    // color to return
    vec4 col = vec4(0.0f);

    for(int i = 0; i < nSteps; ++i)
    {
        if(isInsideBox(pos))
        {
            float xLookUp = remap(pos.x, bbmin.x, bbmax.x, 0, voxelRes.x) / voxelRes.x;
            float yLookUp = remap(pos.y, bbmin.y, bbmax.y, 0, voxelRes.y) / voxelRes.y;
            float zLookUp = remap(pos.z, bbmin.z, bbmax.z, 0, voxelRes.z) / voxelRes.z;
            if(texture(sdfTexture, vec3(xLookUp, yLookUp, zLookUp)).a < 10 && objectColor.a > 0)
            {
                float distToCam = (-(viewMatrix * vec4(pos, 1.0f)).z - 0.1f) / (100000.f - .1f);
                depthBuffer     = vec4(vec3(distToCam), 1);
                objectColor.a   = 1;
                return vec4(objectColor);
            }
        }
        pos += pathDir;
    }
    // determine alpha value
    return col;
}
// MAIN
// FUNCTION//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void main()
{
    /*
     * COMPUTE RAY IN WORLD COORDINATES
     */
    vec4 rayInImg       = vec4(computeImgSpaceCoord(), 1.0f);
    vec4 rayView        = invProj * rayInImg;
    rayView             = vec4(rayView.xy, -1.0f, 0.0f);
    vec3 rayInWorld     = (invView * rayView).xyz;
    vec3 rayInWorldNorm = normalize(rayInWorld);

    float dotLightRay = dot(rayInWorldNorm, lightDir);
    /*
     * COMPUTE START & END OF SAMPLING
     */
    vec3 startPos, endPos, skyPos, skyPos2, skyPos3;
    raySphereintersection(camPos, rayInWorldNorm, SPHERE_INNER_RADIUS, startPos);
    raySphereintersection(camPos, rayInWorldNorm, SPHERE_OUTER_RADIUS, endPos);
    raySphereintersection(camPos, rayInWorldNorm, SKYDOME_RADIUS, skyPos);
    skyPos2 = skyPos - 80.f * rayInWorld;
    skyPos3 = skyPos + 100.f * rayInWorld;

    vec4 skyColor = computeSkyColor(skyPos - camPos, dotLightRay);

    /*
     * COMPUTE SKY COLOR
     */

    vec4 bgColor = skyColor;

    if(fogAmount > 0.965f)
    {
        fragmentColor = bgColor; // down half sphere
    }

    /*
     * MARCH TO CLOUD
     */
    vec4 objectColor = texture(colorTexture, TexCoords);
    objectColor.rgb  = objectColor.rgb / 1.8f + 0.1f; // vec4(0.4,0.2,0.0,1.0);//
    vec4 lowerColor  = vec4(0.f);
    vec4 cloudColor  = vec4(0.f);
    if(rayInWorldNorm.y > 0)
    {
        if(bbmin.y < startPos.y)
        {
            lowerColor = raymarchToObject(lightDir, startPos, bgColor.rgb, objectColor * 1.8f - 0.1f);
        }
        if(lowerColor.a == 0)
        {
            cloudColor     = raymarchToCloud(startPos, endPos, dotLightRay, bgColor.rgb, objectColor);
            cloudColor.rgb = cloudColor.rgb * 1.8f - 0.1f; // contrast-illumination tuning
        }
    }

    if(rayInWorld.y <= 0 || nocutscene)
    {
        if(texture(depthTexture, TexCoords).r < 1.0f) // erase pixel if it's occluded by the terrain
        {
            lowerColor.rgb = objectColor.rgb * 1.8f - 0.1f;
            lowerColor.a   = 1;
        }
    }
    if(useWind)
    {
        skyPos += getHeightFractionForPoint(skyPos) * rotateY(30.f) * windDirection * CLOUD_TOP_OFFSET;
        skyPos += rotateY(30.f) * windDirection * iTime * (cloudSpeed - 100.f);
        skyPos2 += getHeightFractionForPoint(skyPos2) * rotateY(45.f) * windDirection * CLOUD_TOP_OFFSET;
        skyPos2 += rotateY(45.f) * windDirection * iTime * (cloudSpeed - 150.f);
        skyPos3 += getHeightFractionForPoint(skyPos3) * rotateY(20.f) * windDirection * CLOUD_TOP_OFFSET;
        skyPos3 += rotateY(20.f) * windDirection * iTime * (cloudSpeed + 150.f);
    }
    vec2 uv  = getUVProjection(skyPos);
    vec2 uv2 = getUVProjection(skyPos2);
    vec2 uv3 = getUVProjection(skyPos3);

    vec4 highCloudColor  = texture(highClouds, uv * 10.f) * coverageMultiplierHigh;
    vec4 highCloudColor2 = texture(highClouds2, uv2 * 10.f) * coverageMultiplierHigh;
    vec4 highCloudColor3 = texture(highClouds3, uv3 * 10.f) * coverageMultiplierHigh;

    /*
     * MIX CLOUD AND SKY DEPENDING ON FOG
     */

    vec3  ambientColor  = bgColor.rgb;
    float blendFactor   = pow(e, -500.f * pow(abs(dot(rayInWorldNorm, vec3(0.f, 1.f, 0.f))), 2.f));
    cloudColor.rgb      = mix(cloudColor.rgb, skyColor.rgb * cloudColor.a, max(fogAmount, blendFactor));
    lowerColor.rgb      = mix(lowerColor.rgb, skyColor.rgb * lowerColor.a, fogAmount);
    highCloudColor.rgb  = mix(highCloudColor.rgb, ambientColor * highCloudColor.a, max(fogAmount, blendFactor));
    highCloudColor2.rgb = mix(highCloudColor2.rgb, ambientColor * highCloudColor2.a, max(fogAmount, blendFactor));
    highCloudColor3.rgb = mix(highCloudColor3.rgb, ambientColor * highCloudColor3.a, max(fogAmount, blendFactor));

    vec4 highColor = max(highCloudColor, max(highCloudColor2, highCloudColor3));
    /*
     * ADD SUN GLARE TO CLOUDS
     */

    /*
     * COMPUTE FINAL COLOR
     */
    cloudColor = cloudColor * (1.0f - lowerColor.a) + lowerColor;

    bgColor.rgb += cloudColor.rgb * (1.0f - highColor.r) * (1.0f - cloudColor.a);

    if(addSunGlare && dotLightRay >= 0)
    {
        vec3 s = 1.0f * sunColor * pow((dotLightRay), 256.0f);
        cloudColor.rgb += s * cloudColor.a;
    }
    bgColor.rgb = bgColor.rgb * (1.0f - cloudColor.a) + cloudColor.rgb;
    bgColor.rgb = bgColor.rgb * (1.0f - lowerColor.a) + lowerColor.rgb;
    bgColor.a   = 1.0f;

    alphaness = vec4(threshold(cloudColor.a, 0.2f), 0.0f, 0.0f, 1.0f);

    fragmentColor = bgColor;
    fragmentColor.rgb *= clamp((0.2989f * sunColor.r + 0.587f * sunColor.g + 0.114f * sunColor.b), 0.0f, 1.0f);

    if(lensFlare)
        fragmentColor.rgb += vec3(1.4f, 1.2f, 1.0f) * lensflare(gl_FragCoord.xy / iResolution.xy - 0.5f) * smoothstep(-.3f, .5f, dotLightRay);

    if(onlyFirstPass)
        fragmentColor = texture(colorTexture, TexCoords).rgba;
}
