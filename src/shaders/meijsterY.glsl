layout(local_size_x = WG_Y, local_size_y = 1, local_size_z = 1) in;

layout(binding = 0, r32f) uniform image3D img;

shared float imgC[RES_Y];

void main()
{
    int z = int(gl_GlobalInvocationID.z);
    int y = int(gl_GlobalInvocationID.x);
    int x = int(gl_GlobalInvocationID.y);
    if(y >= RES_Y)
        return;

    for(int y_c = int(gl_LocalInvocationID.x); y_c < RES_Y; y_c += int(gl_WorkGroupSize.y))
    {
        imgC[y_c] = imageLoad(img, ivec3(x, y_c, z)).r;
    }
    barrier();
    float value = imgC[y];
    int   d     = 1;
    for(int y_i = 1; y_i < RES_Y - y; ++y_i)
    {
        value = min(value, imgC[y + y_i] + d);
        d += 1 + 2 * y_i;
    }
    d = 1;
    for(int y_i = 1; y_i < y; ++y_i)
    {
        value = min(value, imgC[y - y_i] + d);
        d += 1 + 2 * y_i;
    }
    imageStore(img, ivec3(x, y, z), vec4(value));
}
