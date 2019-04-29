/* Copyright 2011 Pyarelal Knowles, under GNU LGPL (see LICENCE.txt) */

#version 420 core

layout(binding = 1, r32ui) coherent uniform uimage3D cands;
uniform uvec3 res;
uniform uint maxIndex;
uniform int  stepSize;
uniform int  pass;

#define PASS_SUMUP 0
#define PASS_SUMDOWN 1

void main()
{
    ivec3 maxIndex_tc;

    int nextIndex = gl_VertexID;
    nextIndex     = (nextIndex + 1) * stepSize - 1;
    ivec3 nextIndex_tc =
            ivec3(nextIndex % res.x, (nextIndex / res.x) % res.y, nextIndex / (res.y * res.x));

    int   index    = nextIndex - (stepSize >> 1);
    ivec3 index_tc = ivec3(index % res.x, (index / res.x) % res.y, index / (res.y * res.x));

    uint a = imageLoad(cands, index_tc).r;
    uint b;

    if((index < maxIndex) && (nextIndex <= maxIndex))
    {
        b = imageLoad(cands, nextIndex_tc).r;
        imageStore(cands, nextIndex_tc, uvec4(a + b));
        if(pass == PASS_SUMDOWN)
            imageStore(cands, index_tc, uvec4(b));
    }

    if((index < maxIndex) && (nextIndex > maxIndex))
    {
        maxIndex_tc =
                ivec3(maxIndex % res.x, (maxIndex / res.x) % res.y, maxIndex / (res.y * res.x));
        b = imageLoad(cands, maxIndex_tc).r;
        imageStore(cands, maxIndex_tc, uvec4(a + b));
        if(pass == PASS_SUMDOWN)
            imageStore(cands, index_tc, uvec4(b));
    }
}
