#ifndef __EMBOSS_DATA_H__
#define __EMBOSS_DATA_H__

#include "dychart.h"

class emboss_data
{
      public:
            emboss_data(){ pmap = NULL; gltexind = 0; }
            ~emboss_data(){ free(pmap); }

            int         *pmap;
            int         width;
            int         height;

            GLuint      gltexind;
};

#endif
