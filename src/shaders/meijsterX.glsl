layout(local_size_x = WG_X, local_size_y = 1, local_size_z = 1) in;

layout(binding = 0, r32f) uniform image3D img;

shared float imgC[RES_X];

void main()
{
    int z = int(gl_GlobalInvocationID.z);
    int y = int(gl_GlobalInvocationID.y);
    int x = int(gl_GlobalInvocationID.x);
    if(x >= RES_X)
        return;

    for(int x_c = int(gl_LocalInvocationID.x); x_c < RES_X; x_c += int(gl_WorkGroupSize.x))
    {
        imgC[x_c] = imageLoad(img, ivec3(x_c, y, z)).r * RES_X * RES_Y * RES_Z;
    }
    barrier();
    float value = imgC[x];
    int   d     = 1;
    for(int x_i = 1; x_i < RES_X - x; ++x_i)
    {
        value = min(value, imgC[x + x_i] + d);
        d += 1 + 2 * x_i;
    }
    d = 1;
    for(int x_i = 1; x_i < x; ++x_i)
    {
        value = min(value, imgC[x - x_i] + d);
        d += 1 + 2 * x_i;
    }
    imageStore(img, ivec3(x, y, z), vec4(value));
}