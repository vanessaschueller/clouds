layout(local_size_x = WG_Z, local_size_y = 1, local_size_z = 1) in;

layout(binding = 0, r32f) uniform image3D img;

uniform float scale;

shared float imgC[RES_Z];

void main()
{
    int z = int(gl_GlobalInvocationID.x);
    int y = int(gl_GlobalInvocationID.z);
    int x = int(gl_GlobalInvocationID.y);
    if(z >= RES_Z)
        return;

    for(int z_c = int(gl_LocalInvocationID.x); z_c < RES_Z; z_c += int(gl_WorkGroupSize.z))
    {
        imgC[z_c] = imageLoad(img, ivec3(x, y, z_c)).r;
    }
    barrier();
    float value = imgC[z];
    int   d     = 1;
    for(int z_i = 1; z_i < RES_Z - z; ++z_i)
    {
        value = min(value, imgC[z + z_i] + d);
        d += 1 + 2 * z_i;
    }
    d = 1;
    for(int z_i = 1; z_i < z; ++z_i)
    {
        value = min(value, imgC[z - z_i] + d);
        d += 1 + 2 * z_i;
    }
    imageStore(img, ivec3(x, y, z), vec4(scale * sqrt(value)));
}
