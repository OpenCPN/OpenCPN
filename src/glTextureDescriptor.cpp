#include "glTextureDescriptor.h"

glTextureDescriptor::glTextureDescriptor()
{
    for( int i = 0; i < 10; i++ )
        map_array[i] = NULL;

    tex_mult = -1;
    level_min = -1;
    level_max = -1;
    base_size = -1;
    tex_name = 0;
    GPU_base = -1;
}

glTextureDescriptor::~glTextureDescriptor()
{
    for( int i = 0; i < 10; i++ )
        free( map_array[i] );
}

