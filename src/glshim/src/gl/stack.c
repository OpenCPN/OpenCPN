#include "error.h"
#include "stack.h"
#include "loader.h"
#include "remote.h"

void glPushAttrib(GLbitfield mask) {
    ERROR_IN_BLOCK();
    FORWARD_IF_REMOTE(glPushAttrib);
    glstack_t *cur = malloc(sizeof(glstack_t));

    cur->mask = mask;
    cur->clip_planes_enabled = NULL;
    cur->clip_planes = NULL;
    cur->lights_enabled = NULL;
    cur->lights = NULL;

    // TODO: GL_ACCUM_BUFFER_BIT

    // TODO: will tracking these myself be much faster than glGet?
    if (mask & GL_COLOR_BUFFER_BIT) {
        cur->alpha_test = glIsEnabled(GL_ALPHA_TEST);
        glGetIntegerv(GL_ALPHA_TEST_FUNC, &cur->alpha_test_func);
        glGetFloatv(GL_ALPHA_TEST_REF, &cur->alpha_test_ref);

        cur->blend = glIsEnabled(GL_BLEND);
        glGetIntegerv(GL_BLEND_SRC, &cur->blend_src_func);
        glGetIntegerv(GL_BLEND_DST, &cur->blend_dst_func);

        cur->dither = glIsEnabled(GL_DITHER);
        cur->color_logic_op = glIsEnabled(GL_COLOR_LOGIC_OP);
        glGetIntegerv(GL_LOGIC_OP_MODE, &cur->logic_op);

        glGetFloatv(GL_COLOR_CLEAR_VALUE, cur->clear_color);
        glGetFloatv(GL_COLOR_WRITEMASK, cur->color_mask);
    }

    if (mask & GL_CURRENT_BIT) {
        glGetFloatv(GL_CURRENT_COLOR, cur->color);
        glGetFloatv(GL_CURRENT_NORMAL, cur->normal);
        for (int i = 0; i < MAX_TEX; i++) {
            memcpy(cur->tex[i], state.current.tex[i], 2 * sizeof(GLfloat));
        }
    }

    if (mask & GL_DEPTH_BUFFER_BIT) {
        cur->depth_test = glIsEnabled(GL_DEPTH_TEST);
        glGetIntegerv(GL_DEPTH_FUNC, &cur->depth_func);
        glGetFloatv(GL_DEPTH_CLEAR_VALUE, &cur->clear_depth);
        glGetIntegerv(GL_DEPTH_WRITEMASK, &cur->depth_mask);
    }

    if (mask & GL_ENABLE_BIT) {
        int i;
        GLint max_clip_planes;
        glGetIntegerv(GL_MAX_CLIP_PLANES, &max_clip_planes);
        cur->clip_planes_enabled = (GLboolean *)malloc(max_clip_planes * sizeof(GLboolean));
        for (i = 0; i < max_clip_planes; i++) {
            *(cur->clip_planes_enabled + i) = glIsEnabled(GL_CLIP_PLANE0 + i);
        }

        GLint max_lights;
        glGetIntegerv(GL_MAX_LIGHTS, &max_lights);
        cur->lights_enabled = (GLboolean *)malloc(max_lights * sizeof(GLboolean));
        for (i = 0; i < max_lights; i++) {
            *(cur->lights_enabled + i) = glIsEnabled(GL_LIGHT0 + i);
        }

        cur->alpha_test = glIsEnabled(GL_ALPHA_TEST);
        cur->blend = glIsEnabled(GL_BLEND);

        cur->cull_face = glIsEnabled(GL_CULL_FACE);
        cur->depth_test = glIsEnabled(GL_DEPTH_TEST);
        cur->dither = glIsEnabled(GL_DITHER);
        cur->fog = glIsEnabled(GL_FOG);
        cur->lighting = glIsEnabled(GL_LIGHTING);
        cur->line_smooth = glIsEnabled(GL_LINE_SMOOTH);
        cur->line_stipple = glIsEnabled(GL_LINE_STIPPLE);
        cur->color_logic_op = glIsEnabled(GL_COLOR_LOGIC_OP);
        cur->multisample = glIsEnabled(GL_MULTISAMPLE);
        cur->normalize = glIsEnabled(GL_NORMALIZE);
        cur->point_smooth = glIsEnabled(GL_POINT_SMOOTH);
        cur->polygon_offset_fill = glIsEnabled(GL_POLYGON_OFFSET_FILL);
        cur->sample_alpha_to_coverage = glIsEnabled(GL_SAMPLE_ALPHA_TO_COVERAGE);
        cur->sample_alpha_to_one = glIsEnabled(GL_SAMPLE_ALPHA_TO_ONE);
        cur->sample_coverage = glIsEnabled(GL_SAMPLE_COVERAGE);
        cur->scissor_test = glIsEnabled(GL_SCISSOR_TEST);
        cur->stencil_test = glIsEnabled(GL_STENCIL_TEST);
    }

    // TODO: GL_EVAL_BIT

    if (mask & GL_FOG_BIT) {
        cur->fog = glIsEnabled(GL_FOG);
        glGetFloatv(GL_FOG_COLOR, cur->fog_color);
        glGetFloatv(GL_FOG_DENSITY, &cur->fog_density);
        glGetFloatv(GL_FOG_START, &cur->fog_start);
        glGetFloatv(GL_FOG_END, &cur->fog_end);
        glGetIntegerv(GL_FOG_MODE, &cur->fog_mode);
    }

    if (mask & GL_HINT_BIT) {
        glGetIntegerv(GL_PERSPECTIVE_CORRECTION_HINT, &cur->perspective_hint);
        glGetIntegerv(GL_POINT_SMOOTH_HINT, &cur->point_smooth_hint);
        glGetIntegerv(GL_LINE_SMOOTH_HINT, &cur->line_smooth_hint);
        glGetIntegerv(GL_FOG_HINT, &cur->fog_hint);
        glGetIntegerv(GL_GENERATE_MIPMAP_HINT, &cur->mipmap_hint);
    }

    if (mask & GL_LIGHTING_BIT) {
        cur->lighting = glIsEnabled(GL_LIGHTING);
        glGetIntegerv(GL_LIGHT_MODEL_AMBIENT, cur->light_model_ambient);
        glGetIntegerv(GL_LIGHT_MODEL_TWO_SIDE, &cur->light_model_two_side);

        int i;
        GLint max_lights;
        glGetIntegerv(GL_MAX_LIGHTS, &max_lights);
        cur->lights_enabled = (GLboolean *)malloc(max_lights * sizeof(GLboolean));
        cur->lights = (GLfloat *)malloc(max_lights * sizeof(GLfloat));
        for (i = 0; i < max_lights; i++) {
            *(cur->lights_enabled + i) = glIsEnabled(GL_LIGHT0 + i);
            /* TODO: record all data about the lights
            glGetFloatv(GL_LIGHT0 + i, cur->lights + i);
            */
        }
        glGetIntegerv(GL_SHADE_MODEL, &cur->shade_model);
    }

    if (mask & GL_LINE_BIT) {
        cur->line_smooth = glIsEnabled(GL_LINE_SMOOTH);
        // TODO: stipple stuff here
        glGetFloatv(GL_LINE_WIDTH, &cur->line_width);
    }

    if (mask & GL_LIST_BIT) {
        // TODO: local storage of glListBase
    }

    if (mask & GL_MULTISAMPLE_BIT) {
        cur->multisample = glIsEnabled(GL_MULTISAMPLE);
        cur->sample_alpha_to_coverage = glIsEnabled(GL_SAMPLE_ALPHA_TO_COVERAGE);
        cur->sample_alpha_to_one = glIsEnabled(GL_SAMPLE_ALPHA_TO_ONE);
        cur->sample_coverage = glIsEnabled(GL_SAMPLE_COVERAGE);
    }

    // TODO: GL_PIXEL_MODE_BIT

    if (mask & GL_POINT_BIT) {
        cur->point_smooth = glIsEnabled(GL_POINT_SMOOTH);
        glGetFloatv(GL_POINT_SIZE, &cur->point_size);
    }

    // TODO: GL_POLYGON_BIT
    // TODO: GL_POLYGON_STIPPLE_BIT

    if (mask & GL_SCISSOR_BIT) {
        cur->scissor_test = glIsEnabled(GL_SCISSOR_TEST);
        glGetFloatv(GL_SCISSOR_BOX, cur->scissor_box);
    }

    // TODO: GL_STENCIL_BUFFER_BIT

    // TODO: GL_TEXTURE_BIT (incomplete)
    if (mask & GL_ENABLE_BIT || mask & GL_TEXTURE_BIT) {
        glGetIntegerv(GL_ACTIVE_TEXTURE, &cur->active_texture);
        for (int i = 0; i < MAX_TEX; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            cur->texture[i].texgen.s = state.enable.texgen_s[i];
            cur->texture[i].texgen.t = state.enable.texgen_t[i];
            cur->texture[i].texgen.r = state.enable.texgen_r[i];
            cur->texture[i].texgen.q = state.enable.texgen_q[i];
            cur->texture[i].enable_2d = glIsEnabled(GL_TEXTURE_2D);
        }
        if (mask & GL_TEXTURE_BIT) {
            for (int i = 0; i < MAX_TEX; i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &cur->texture[i].min_filter);
                glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &cur->texture[i].mag_filter);
                glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &cur->texture[i].wrap_s);
                glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &cur->texture[i].wrap_t);
                glGetIntegerv(GL_TEXTURE_BINDING_2D, &cur->texture[i].bind);
                memcpy(&cur->texture[i].texgen.state, &state.texgen[i], sizeof(texgen_state_t));
            }
        }
        glActiveTexture(cur->active_texture);
    }

    // TODO: GL_TRANSFORM_BIT (incomplete)
    if (mask & GL_TRANSFORM_BIT) {
        glGetIntegerv(GL_MATRIX_MODE, &cur->matrix_mode);
    }

    // TODO: GL_VIEWPORT_BIT

    tack_push(&state.stack.attrib, cur);
}

void glPushClientAttrib(GLbitfield mask) {
    glclientstack_t *cur = malloc(sizeof(glclientstack_t));
    cur->mask = mask;

    if (mask & GL_CLIENT_PIXEL_STORE_BIT) {
        glGetIntegerv(GL_PACK_ALIGNMENT, &cur->pack_align);
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &cur->unpack_align);
        glGetIntegerv(GL_UNPACK_ROW_LENGTH, &cur->unpack_row_length);
        glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &cur->unpack_skip_pixels);
        glGetIntegerv(GL_UNPACK_SKIP_ROWS, &cur->unpack_skip_rows);
    }

    if (mask & GL_CLIENT_VERTEX_ARRAY_BIT) {
        cur->vert_enable = state.enable.vertex_array;
        cur->color_enable = state.enable.color_array;
        cur->normal_enable = state.enable.normal_array;
        memcpy(&cur->tex_enable, &state.enable.tex_coord_array, sizeof(GLboolean) * MAX_TEX);
        memcpy(&cur->pointers, &state.pointers, sizeof(pointer_states_t));
    }
    tack_push(&state.stack.client, cur);
}

#define enable_disable(pname, enabled) \
    if (enabled) glEnable(pname);      \
    else glDisable(pname)

#define v2(c) c[0], c[1]
#define v3(c) v2(c), c[2]
#define v4(c) v3(c), c[3]

void glPopAttrib() {
    ERROR_IN_BLOCK();
    FORWARD_IF_REMOTE(glPopAttrib);
    glstack_t *cur = tack_pop(&state.stack.attrib);
    if (cur == NULL) {
        ERROR(GL_STACK_UNDERFLOW);
    }

    if (cur->mask & GL_COLOR_BUFFER_BIT) {
#ifndef USE_ES2
        enable_disable(GL_ALPHA_TEST, cur->alpha_test);
        glAlphaFunc(cur->alpha_test_func, cur->alpha_test_ref);
#endif

        enable_disable(GL_BLEND, cur->blend);
        glBlendFunc(cur->blend_src_func, cur->blend_dst_func);

        enable_disable(GL_DITHER, cur->dither);
#ifndef USE_ES2
        enable_disable(GL_COLOR_LOGIC_OP, cur->color_logic_op);
        glLogicOp(cur->logic_op);
#endif

        GLfloat *c;
        glClearColor(v4(cur->clear_color));
        glColorMask(v4(cur->color_mask));
    }

    if (cur->mask & GL_CURRENT_BIT) {
        glColor4f(v4(cur->color));
#ifndef USE_ES2
        glNormal3f(v3(cur->normal));
#endif
        for (int i = 0; i < MAX_TEX; i++) {
            glMultiTexCoord2f(GL_TEXTURE0 + i, v2(cur->tex[i]));
        }
    }

    if (cur->mask & GL_DEPTH_BUFFER_BIT) {
        enable_disable(GL_DEPTH_TEST, cur->depth_test);
        glDepthFunc(cur->depth_func);
        glClearDepth(cur->clear_depth);
        glDepthMask(cur->depth_mask);
    }

    if (cur->mask & GL_ENABLE_BIT) {
        int i;
        GLint max_clip_planes;
        glGetIntegerv(GL_MAX_CLIP_PLANES, &max_clip_planes);
        for (i = 0; i < max_clip_planes; i++) {
            enable_disable(GL_CLIP_PLANE0 + i, *(cur->clip_planes_enabled + i));
        }

        GLint max_lights;
        glGetIntegerv(GL_MAX_LIGHTS, &max_lights);
        for (i = 0; i < max_lights; i++) {
            enable_disable(GL_LIGHT0 + i, *(cur->lights_enabled + i));
        }

        enable_disable(GL_ALPHA_TEST, cur->alpha_test);
        enable_disable(GL_BLEND, cur->blend);
        enable_disable(GL_CULL_FACE, cur->cull_face);
        enable_disable(GL_DEPTH_TEST, cur->depth_test);
        enable_disable(GL_DITHER, cur->dither);
        enable_disable(GL_FOG, cur->fog);
        enable_disable(GL_LIGHTING, cur->lighting);
        enable_disable(GL_LINE_SMOOTH, cur->line_smooth);
        enable_disable(GL_LINE_STIPPLE, cur->line_stipple);
        enable_disable(GL_COLOR_LOGIC_OP, cur->color_logic_op);
        enable_disable(GL_MULTISAMPLE, cur->multisample);
        enable_disable(GL_NORMALIZE, cur->normalize);
        enable_disable(GL_POINT_SMOOTH, cur->point_smooth);
        enable_disable(GL_POLYGON_OFFSET_FILL, cur->polygon_offset_fill);
        enable_disable(GL_SAMPLE_ALPHA_TO_COVERAGE, cur->sample_alpha_to_coverage);
        enable_disable(GL_SAMPLE_ALPHA_TO_ONE, cur->sample_alpha_to_one);
        enable_disable(GL_SAMPLE_COVERAGE, cur->sample_coverage);
        enable_disable(GL_SCISSOR_TEST, cur->scissor_test);
        enable_disable(GL_STENCIL_TEST, cur->stencil_test);
    }

#ifndef USE_ES2
    if (cur->mask & GL_FOG_BIT) {
        enable_disable(GL_FOG, cur->fog);
        glFogfv(GL_FOG_COLOR, cur->fog_color);
        glFogf(GL_FOG_DENSITY, cur->fog_density);
        glFogf(GL_FOG_START, cur->fog_start);
        glFogf(GL_FOG_END, cur->fog_end);
        glFogf(GL_FOG_MODE, cur->fog_mode);
    }
#endif

    if (cur->mask & GL_HINT_BIT) {
        enable_disable(GL_PERSPECTIVE_CORRECTION_HINT, cur->perspective_hint);
        enable_disable(GL_POINT_SMOOTH_HINT, cur->point_smooth_hint);
        enable_disable(GL_LINE_SMOOTH_HINT, cur->line_smooth_hint);
        enable_disable(GL_FOG_HINT, cur->fog_hint);
        enable_disable(GL_GENERATE_MIPMAP_HINT, cur->mipmap_hint);
    }

    if (cur->mask & GL_LINE_BIT) {
        enable_disable(GL_LINE_SMOOTH, cur->line_smooth);
        // TODO: stipple stuff here
        glLineWidth(cur->line_width);
    }

    if (cur->mask & GL_MULTISAMPLE_BIT) {
        enable_disable(GL_MULTISAMPLE, cur->multisample);
        enable_disable(GL_SAMPLE_ALPHA_TO_COVERAGE, cur->sample_alpha_to_coverage);
        enable_disable(GL_SAMPLE_ALPHA_TO_ONE, cur->sample_alpha_to_one);
        enable_disable(GL_SAMPLE_COVERAGE, cur->sample_coverage);
    }

#ifndef USE_ES2
    if (cur->mask & GL_POINT_BIT) {
        if (! cur->mask & GL_ENABLE_BIT) {
            enable_disable(GL_POINT_SMOOTH, cur->point_smooth);
        }
        glPointSize(cur->point_size);
    }
#endif

    if (cur->mask & GL_SCISSOR_BIT) {
        enable_disable(GL_SCISSOR_TEST, cur->scissor_test);
        glScissor(v4(cur->scissor_box));
    }

    if (cur->mask & GL_TEXTURE_BIT || cur->mask & GL_ENABLE_BIT) {
        GLint active_texture;
        glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
        for (int i = 0; i < MAX_TEX; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            enable_disable(GL_TEXTURE_GEN_S, cur->texture[i].texgen.s);
            enable_disable(GL_TEXTURE_GEN_T, cur->texture[i].texgen.t);
            enable_disable(GL_TEXTURE_GEN_R, cur->texture[i].texgen.r);
            enable_disable(GL_TEXTURE_GEN_Q, cur->texture[i].texgen.q);
            enable_disable(GL_TEXTURE_2D, cur->texture[i].enable_2d);
        }
        if (cur->mask & GL_TEXTURE_BIT) {
            for (int i = 0; i < MAX_TEX; i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, cur->texture[i].bind);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, cur->texture[i].min_filter);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, cur->texture[i].mag_filter);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, cur->texture[i].wrap_s);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, cur->texture[i].wrap_t);
            }
            glActiveTexture(cur->active_texture);
        } else {
            glActiveTexture(active_texture);
        }
    }

    if (cur->mask & GL_TRANSFORM_BIT) {
        glMatrixMode(cur->matrix_mode);
    }

    free(cur->clip_planes_enabled);
    free(cur->clip_planes);
    free(cur->lights_enabled);
    free(cur->lights);
    free(cur);
}

#undef enable_disable
#define enable_disable(pname, enabled)             \
    if (enabled) glEnableClientState(pname);       \
    else glDisableClientState(pname)

void glPopClientAttrib() {
    glclientstack_t *cur = tack_pop(&state.stack.client);
    if (cur == NULL) {
        ERROR(GL_STACK_UNDERFLOW);
    }

    if (cur->mask & GL_CLIENT_PIXEL_STORE_BIT) {
        glPixelStorei(GL_PACK_ALIGNMENT, cur->pack_align);
        glPixelStorei(GL_UNPACK_ALIGNMENT, cur->unpack_align);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, cur->unpack_row_length);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, cur->unpack_skip_pixels);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, cur->unpack_skip_rows);
    }

    if (cur->mask & GL_CLIENT_VERTEX_ARRAY_BIT) {
        enable_disable(GL_VERTEX_ARRAY, cur->vert_enable);
        enable_disable(GL_NORMAL_ARRAY, cur->normal_enable);
        enable_disable(GL_COLOR_ARRAY, cur->color_enable);
        for (int i = 0; i < MAX_TEX; i++) {
            GLboolean bit = cur->tex_enable[i];
            if (bit != state.enable.tex_coord_array[i]) {
                enable_disable(GL_TEXTURE_COORD_ARRAY, bit);
            }
        }
        memcpy(&state.pointers, &cur->pointers, sizeof(pointer_states_t));
    }
    free(cur);
}

#undef enable_disable
#undef v2
#undef v3
#undef v4
