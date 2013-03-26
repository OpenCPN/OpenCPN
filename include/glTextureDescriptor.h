#ifndef __GLTEXTUREDESCRIPTOR_H__
#define __GLTEXTUREDESCRIPTOR_H__

#include "dychart.h"

class glTextureDescriptor
{
public:
    glTextureDescriptor();
    ~glTextureDescriptor();

    GLuint tex_name;
    int tex_mult;
    int level_min;
    int level_max;
    int base_size;
    int GPU_base;

    unsigned char *map_array[10];
};


#endif
