#define GL_QUADS      7
#define GL_QUAD_STRIP 8
#define GL_POLYGON    9

/* texture mapping */
#define GL_TEXTURE_ENV              0x2300
#define GL_TEXTURE_ENV_MODE         0x2200
#define GL_TEXTURE_1D               0x0DE0
#define GL_TEXTURE_2D               0x0DE1
#define GL_TEXTURE_3D               0x806F
#define GL_TEXTURE_WRAP_S           0x2802
#define GL_TEXTURE_WRAP_T           0x2803
#define GL_TEXTURE_MAG_FILTER       0x2800
#define GL_TEXTURE_MIN_FILTER       0x2801
#define GL_TEXTURE_ENV_COLOR        0x2201
#define GL_TEXTURE_GEN_S            0x0C60
#define GL_TEXTURE_GEN_T            0x0C61
#define GL_TEXTURE_GEN_MODE         0x2500
#define GL_TEXTURE_BORDER_COLOR     0x1004
#define GL_TEXTURE_WIDTH            0x1000
#define GL_TEXTURE_HEIGHT           0x1001
#define GL_TEXTURE_BORDER           0x1005
#define GL_TEXTURE_COMPONENTS       0x1003
#define GL_TEXTURE_RED_SIZE         0x805C
#define GL_TEXTURE_GREEN_SIZE       0x805D
#define GL_TEXTURE_BLUE_SIZE        0x805E
#define GL_TEXTURE_ALPHA_SIZE       0x805F
#define GL_TEXTURE_LUMINANCE_SIZE   0x8060
#define GL_TEXTURE_INTENSITY_SIZE   0x8061
#define GL_NEAREST_MIPMAP_NEAREST   0x2700
#define GL_NEAREST_MIPMAP_LINEAR    0x2702
#define GL_LINEAR_MIPMAP_NEAREST    0x2701
#define GL_LINEAR_MIPMAP_LINEAR     0x2703
#define GL_OBJECT_LINEAR            0x2401
#define GL_OBJECT_PLANE             0x2501
#define GL_EYE_LINEAR               0x2400
#define GL_EYE_PLANE                0x2502
#define GL_SPHERE_MAP               0x2402
#define GL_DECAL                    0x2101
#define GL_MODULATE                 0x2100
#define GL_NEAREST                  0x2600
#define GL_REPEAT                   0x2901
#define GL_CLAMP                    0x2900
#define GL_S                        0x2000
#define GL_T                        0x2001
#define GL_R                        0x2002
#define GL_Q                        0x2003
#define GL_TEXTURE_GEN_R            0x0C62
#define GL_TEXTURE_GEN_Q            0x0C63
#define GL_PROXY_TEXTURE_1D         0x8063
#define GL_PROXY_TEXTURE_2D         0x8064
#define GL_PROXY_TEXTURE_3D         0x8070

// GL_ARB_texture_rectangle
#define GL_TEXTURE_RECTANGLE_ARB          0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_ARB  0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE_ARB    0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB 0x84F8

#define GL_LINE_SMOOTH            0x0B20
#define GL_LINE_STIPPLE           0x0B24
#define GL_LINE_STIPPLE_PATTERN   0x0B25
#define GL_LINE_STIPPLE_REPEAT    0x0B26
#define GL_LINE_WIDTH             0x0B21
#define GL_LINE_WIDTH_GRANULARITY 0x0B23
#define GL_LINE_WIDTH_RANGE       0x0B22

#define GL_OBJECT_LINEAR    0x2401
#define GL_OBJECT_PLANE     0x2501
#define GL_EYE_LINEAR       0x2400
#define GL_EYE_PLANE        0x2502
#define GL_SPHERE_MAP       0x2402

#define GL_CURRENT_BIT          0x00001
#define GL_POINT_BIT            0x00002
#define GL_LINE_BIT             0x00004
#define GL_POLYGON_BIT          0x00008
#define GL_POLYGON_STIPPLE_BIT  0x00010
#define GL_PIXEL_MODE_BIT       0x00020
#define GL_LIGHTING_BIT         0x00040
#define GL_FOG_BIT              0x00080
// some of these are already defined in GLES
// #define GL_DEPTH_BUFFER_BIT     0x00100
#define GL_ACCUM_BUFFER_BIT     0x00200
// #define GL_STENCIL_BUFFER_BIT   0x00400
#define GL_VIEWPORT_BIT         0x00800
#define GL_TRANSFORM_BIT        0x01000
#define GL_ENABLE_BIT           0x02000
// #define GL_COLOR_BUFFER_BIT     0x04000
#define GL_HINT_BIT             0x08000
#define GL_EVAL_BIT             0x10000
#define GL_LIST_BIT             0x20000
#define GL_TEXTURE_BIT          0x40000
#define GL_SCISSOR_BIT          0x80000
#define GL_ALL_ATTRIB_BITS      0xFFFFF
#define GL_MULTISAMPLE_BIT      0x20000000

#define GL_CLIENT_PIXEL_STORE_BIT  0x00000001
#define GL_CLIENT_VERTEX_ARRAY_BIT 0x00000002
#define GL_ALL_CLIENT_ATTRIB_BITS  0xFFFFFFFF
#define GL_CLIENT_ALL_ATTRIB_BITS  0xFFFFFFFF


// pixel formats
#define GL_RED                         0x1903
#define GL_RG                          0x8227
#define GL_BGR                         0x80E0
#define GL_BGRA                        0x80E1
#define GL_UNSIGNED_BYTE_3_3_2         0x8032
#define GL_UNSIGNED_BYTE_2_3_3_REV     0x8362
#define GL_UNSIGNED_SHORT_5_6_5        0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV    0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4      0x8033
#define GL_UNSIGNED_SHORT_4_4_4_4_REV  0x8365
#define GL_UNSIGNED_SHORT_5_5_5_1      0x8034
#define GL_UNSIGNED_SHORT_1_5_5_5_REV  0x8366
#define GL_UNSIGNED_INT_8_8_8_8        0x8035
#define GL_UNSIGNED_INT_8_8_8_8_REV    0x8367
#define GL_UNSIGNED_INT_10_10_10_2     0x8036
#define GL_UNSIGNED_INT_2_10_10_10_REV 0x8368

// types
#define GL_BYTE                 0x1400
#define GL_UNSIGNED_BYTE        0x1401
#define GL_SHORT                0x1402
#define GL_UNSIGNED_SHORT       0x1403
#define GL_INT                  0x1404
#define GL_UNSIGNED_INT         0x1405
#define GL_FLOAT                0x1406
#define GL_2_BYTES              0x1407
#define GL_3_BYTES              0x1408
#define GL_4_BYTES              0x1409
#define GL_DOUBLE               0x140A

#define GL_COMPILE              0x1300
#define GL_COMPILE_AND_EXECUTE  0x1301

// fog
#define GL_FOG                  0x0B60
#define GL_FOG_MODE             0x0B65
#define GL_FOG_DENSITY          0x0B62
#define GL_FOG_COLOR            0x0B66
#define GL_FOG_INDEX            0x0B61
#define GL_FOG_START            0x0B63
#define GL_FOG_END              0x0B64
#define GL_LINEAR               0x2601
#define GL_EXP                  0x0800
#define GL_EXP2                 0x0801

// lighting
#define GL_LIGHTING             0x0B50
#define GL_LIGHT0               0x4000
#define GL_LIGHT1               0x4001
#define GL_LIGHT2               0x4002
#define GL_LIGHT3               0x4003
#define GL_LIGHT4               0x4004
#define GL_LIGHT5               0x4005
#define GL_LIGHT6               0x4006
#define GL_LIGHT7               0x4007
#define GL_SPOT_EXPONENT        0x1205
#define GL_SPOT_CUTOFF          0x1206
#define GL_CONSTANT_ATTENUATION 0x1207
#define GL_LINEAR_ATTENUATION   0x1208
#define GL_QUADRATIC_ATTENUATION 0x1209
#define GL_AMBIENT              0x1200
#define GL_DIFFUSE              0x1201
#define GL_SPECULAR             0x1202
#define GL_SHININESS            0x1601
#define GL_EMISSION             0x1600
#define GL_POSITION             0x1203
#define GL_SPOT_DIRECTION       0x1204
#define GL_AMBIENT_AND_DIFFUSE  0x1602
#define GL_COLOR_INDEXES        0x1603
#define GL_LIGHT_MODEL_TWO_SIDE 0x0B52
#define GL_LIGHT_MODEL_LOCAL_VIEWER 0x0B51
#define GL_LIGHT_MODEL_AMBIENT  0x0B53
#define GL_FRONT_AND_BACK       0x0408
#define GL_SHADE_MODEL          0x0B54
#define GL_FLAT                 0x1D00
#define GL_SMOOTH               0x1D01
#define GL_COLOR_MATERIAL       0x0B57
#define GL_COLOR_MATERIAL_FACE  0x0B55
#define GL_COLOR_MATERIAL_PARAMETER 0x0B56
#define GL_NORMALIZE            0x0BA1

// pixel transfer
#define GL_MAP_COLOR             0x0D10
#define GL_MAP_STENCIL           0x0D11
#define GL_INDEX_SHIFT           0x0D12
#define GL_INDEX_OFFSET          0x0D13
#define GL_RED_SCALE             0x0D14
#define GL_RED_BIAS              0x0D15
#define GL_GREEN_SCALE           0x0D18
#define GL_GREEN_BIAS            0x0D19
#define GL_BLUE_SCALE            0x0D1A
#define GL_BLUE_BIAS             0x0D1B
#define GL_ALPHA_SCALE           0x0D1C
#define GL_ALPHA_BIAS            0x0D1D
#define GL_DEPTH_SCALE           0x0D1E
#define GL_DEPTH_BIAS            0x0D1F
#define GL_PIXEL_MAP_S_TO_S_SIZE 0x0CB1
#define GL_PIXEL_MAP_I_TO_I_SIZE 0x0CB0
#define GL_PIXEL_MAP_I_TO_R_SIZE 0x0CB2
#define GL_PIXEL_MAP_I_TO_G_SIZE 0x0CB3
#define GL_PIXEL_MAP_I_TO_B_SIZE 0x0CB4
#define GL_PIXEL_MAP_I_TO_A_SIZE 0x0CB5
#define GL_PIXEL_MAP_R_TO_R_SIZE 0x0CB6
#define GL_PIXEL_MAP_G_TO_G_SIZE 0x0CB7
#define GL_PIXEL_MAP_B_TO_B_SIZE 0x0CB8
#define GL_PIXEL_MAP_A_TO_A_SIZE 0x0CB9
#define GL_PIXEL_MAP_S_TO_S      0x0C71
#define GL_PIXEL_MAP_I_TO_I      0x0C70
#define GL_PIXEL_MAP_I_TO_R      0x0C72
#define GL_PIXEL_MAP_I_TO_G      0x0C73
#define GL_PIXEL_MAP_I_TO_B      0x0C74
#define GL_PIXEL_MAP_I_TO_A      0x0C75
#define GL_PIXEL_MAP_R_TO_R      0x0C76
#define GL_PIXEL_MAP_G_TO_G      0x0C77
#define GL_PIXEL_MAP_B_TO_B      0x0C78
#define GL_PIXEL_MAP_A_TO_A      0x0C79
#define GL_PACK_ALIGNMENT        0x0D05
#define GL_PACK_LSB_FIRST        0x0D01
#define GL_PACK_ROW_LENGTH       0x0D02
#define GL_PACK_SKIP_PIXELS      0x0D04
#define GL_PACK_SKIP_ROWS        0x0D03
#define GL_PACK_SWAP_BYTES       0x0D00
#define GL_UNPACK_ALIGNMENT      0x0CF5
#define GL_UNPACK_LSB_FIRST      0x0CF1
#define GL_UNPACK_ROW_LENGTH     0x0CF2
#define GL_UNPACK_SKIP_PIXELS    0x0CF4
#define GL_UNPACK_SKIP_ROWS      0x0CF3
#define GL_UNPACK_SWAP_BYTES     0x0CF0
#define GL_ZOOM_X                0x0D16
#define GL_ZOOM_Y                0x0D17

// blending
#define GL_BLEND                 0x0BE2
#define GL_BLEND_SRC             0x0BE1
#define GL_BLEND_DST             0x0BE0
#define GL_SRC_COLOR             0x0300
#define GL_ONE_MINUS_SRC_COLOR   0x0301
#define GL_SRC_ALPHA             0x0302
#define GL_ONE_MINUS_SRC_ALPHA   0x0303
#define GL_DST_ALPHA             0x0304
#define GL_ONE_MINUS_DST_ALPHA   0x0305
#define GL_DST_COLOR             0x0306
#define GL_ONE_MINUS_DST_COLOR   0x0307
#define GL_SRC_ALPHA_SATURATE    0x0308

// glGet
#define GL_AUX_BUFFERS           0x0C00
#define GL_MAX_ELEMENTS_VERTICES 0x80E8
#define GL_MAX_ELEMENTS_INDICES  0x80E9

// evaluators
#define GL_AUTO_NORMAL           0x0D80
#define GL_MAP1_COLOR_4          0x0D90
#define GL_MAP1_INDEX            0x0D91
#define GL_MAP1_NORMAL           0x0D92
#define GL_MAP1_TEXTURE_COORD_1  0x0D93
#define GL_MAP1_TEXTURE_COORD_2  0x0D94
#define GL_MAP1_TEXTURE_COORD_3  0x0D95
#define GL_MAP1_TEXTURE_COORD_4  0x0D96
#define GL_MAP1_VERTEX_3         0x0D97
#define GL_MAP1_VERTEX_4         0x0D98
#define GL_MAP2_COLOR_4          0x0DB0
#define GL_MAP2_INDEX            0x0DB1
#define GL_MAP2_NORMAL           0x0DB2
#define GL_MAP2_TEXTURE_COORD_1  0x0DB3
#define GL_MAP2_TEXTURE_COORD_2  0x0DB4
#define GL_MAP2_TEXTURE_COORD_3  0x0DB5
#define GL_MAP2_TEXTURE_COORD_4  0x0DB6
#define GL_MAP2_VERTEX_3         0x0DB7
#define GL_MAP2_VERTEX_4         0x0DB8
#define GL_MAP1_GRID_DOMAIN      0x0DD0
#define GL_MAP1_GRID_SEGMENTS    0x0DD1
#define GL_MAP2_GRID_DOMAIN      0x0DD2
#define GL_MAP2_GRID_SEGMENTS    0x0DD3
#define GL_COEFF                 0x0A00
#define GL_ORDER                 0x0A01
#define GL_DOMAIN                0x0A02

/* polygons */
#define GL_POINT                 0x1B00
#define GL_LINE                  0x1B01
#define GL_FILL                  0x1B02
#define GL_CW                    0x0900
#define GL_CCW                   0x0901
#define GL_FRONT                 0x0404
#define GL_BACK                  0x0405
#define GL_POLYGON_MODE          0x0B40
#define GL_POLYGON_SMOOTH        0x0B41
#define GL_POLYGON_STIPPLE       0x0B42
#define GL_EDGE_FLAG             0x0B43
#define GL_CULL_FACE             0x0B44
#define GL_CULL_FACE_MODE        0x0B45
#define GL_FRONT_FACE            0x0B46
#define GL_POLYGON_OFFSET_FACTOR 0x8038
#define GL_POLYGON_OFFSET_UNITS  0x2A00
#define GL_POLYGON_OFFSET_POINT  0x2A01
#define GL_POLYGON_OFFSET_LINE   0x2A02

/* Shader Source */
#define GL_COMPILE_STATUS                 0x8B81
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_SHADER_SOURCE_LENGTH           0x8B88
#define GL_SHADER_COMPILER                0x8DFA

/* Shader Binary */
#define GL_SHADER_BINARY_FORMATS          0x8DF8
#define GL_NUM_SHADER_BINARY_FORMATS      0x8DF9

/* Shader Precision-Specified Types */
#define GL_LOW_FLOAT                      0x8DF0
#define GL_MEDIUM_FLOAT                   0x8DF1
#define GL_HIGH_FLOAT                     0x8DF2
#define GL_LOW_INT                        0x8DF3
#define GL_MEDIUM_INT                     0x8DF4
#define GL_HIGH_INT                       0x8DF5
