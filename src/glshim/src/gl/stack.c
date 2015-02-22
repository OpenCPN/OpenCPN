#include "stack.h"

glstack_t *stack = NULL;
glclientstack_t *clientStack = NULL;

void glPushAttrib(GLbitfield mask) {
    if (stack == NULL) {
        stack = (glstack_t *)malloc(STACK_SIZE * sizeof(glstack_t));
        stack->len = 0;
        stack->cap = STACK_SIZE;
    } else if (stack->len == stack->cap) {
        stack->cap += STACK_SIZE;
        stack = (glstack_t *)realloc(stack, stack->cap * sizeof(glstack_t));
    }

    glstack_t *cur = stack + stack->len;
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
        glGetFloatv(GL_CURRENT_TEXTURE_COORDS, cur->tex);
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
        cur->texture_2d = glIsEnabled(GL_TEXTURE_2D);
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

    // TODO: incomplete
    if (mask & GL_TEXTURE_BIT) {
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &cur->texture);
    }

    // TODO: GL_TRANSFORM_BIT
    // TODO: GL_VIEWPORT_BIT

    stack->len++;
}

void glPushClientAttrib(GLbitfield mask) {
    if (clientStack == NULL) {
        clientStack = (glclientstack_t *)malloc(STACK_SIZE * sizeof(glclientstack_t));
        clientStack->len = 0;
        clientStack->cap = STACK_SIZE;
    } else if (clientStack->len == clientStack->cap) {
        clientStack->cap += STACK_SIZE;
        clientStack = (glclientstack_t *)realloc(clientStack, clientStack->cap * sizeof(glclientstack_t));
    }

    glclientstack_t *cur = clientStack + clientStack->len;
    cur->mask = mask;

    if (mask & GL_CLIENT_PIXEL_STORE_BIT) {
        glGetIntegerv(GL_PACK_ALIGNMENT, &cur->pack_align);
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &cur->unpack_align);
        cur->unpack_row_length = state.texture.unpack_row_length;
        cur->unpack_skip_pixels = state.texture.unpack_skip_pixels;
        cur->unpack_skip_rows = state.texture.unpack_skip_rows;
    }

    if (mask & GL_CLIENT_VERTEX_ARRAY_BIT) {
        cur->vert_enable = state.enable.vertex_array;
        cur->color_enable = state.enable.color_array;
        cur->normal_enable = state.enable.normal_array;
        cur->tex_enable = state.enable.tex_coord_array;

        memcpy(&cur->verts, &state.pointers.vertex, sizeof(pointer_state_t));
        memcpy(&cur->color, &state.pointers.color, sizeof(pointer_state_t));
        memcpy(&cur->normal, &state.pointers.normal, sizeof(pointer_state_t));
        memcpy(&cur->tex, &state.pointers.tex_coord, sizeof(pointer_state_t));
    }

    clientStack->len++;
}

#define maybe_free(x) \
    if (x) free(x)

#define enable_disable(pname, enabled) \
    if (enabled) glEnable(pname);      \
    else glDisable(pname)

#define v2(c) c[0], c[1]
#define v3(c) v2(c), c[2]
#define v4(c) v3(c), c[3]

void glPopAttrib() {
    if (stack == NULL || stack->len == 0)
        return;

    glstack_t *cur = stack + stack->len-1;

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
        glTexCoord2f(v2(cur->tex));
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
        enable_disable(GL_TEXTURE_2D, cur->texture_2d);
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
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, cur->perspective_hint);
        glHint(GL_POINT_SMOOTH_HINT, cur->point_smooth_hint);
        glHint(GL_LINE_SMOOTH_HINT, cur->line_smooth_hint);
        glHint(GL_FOG_HINT, cur->fog_hint);
        glHint(GL_GENERATE_MIPMAP_HINT, cur->mipmap_hint);
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
        enable_disable(GL_POINT_SMOOTH, cur->point_smooth);
        glPointSize(cur->point_size);
    }
#endif

    if (cur->mask & GL_SCISSOR_BIT) {
        enable_disable(GL_SCISSOR_TEST, cur->scissor_test);
        glScissor(v4(cur->scissor_box));
    }

    if (cur->mask & GL_TEXTURE_BIT) {
        glBindTexture(GL_TEXTURE_2D, cur->texture);
    }

    maybe_free(cur->clip_planes_enabled);
    maybe_free(cur->clip_planes);
    maybe_free(cur->lights_enabled);
    maybe_free(cur->lights);
    stack->len--;
}

#undef enable_disable
#define enable_disable(pname, enabled)             \
    if (enabled) glEnableClientState(pname);       \
    else glDisableClientState(pname)

void glPopClientAttrib() {
    if (clientStack == NULL || clientStack->len == 0)
        return;

    glclientstack_t *cur = clientStack + clientStack->len-1;
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
        enable_disable(GL_TEXTURE_COORD_ARRAY, cur->tex_enable);

        memcpy(&state.pointers.vertex, &cur->verts, sizeof(pointer_state_t));
        memcpy(&state.pointers.color, &cur->color, sizeof(pointer_state_t));
        memcpy(&state.pointers.normal, &cur->normal, sizeof(pointer_state_t));
        memcpy(&state.pointers.tex_coord, &cur->tex, sizeof(pointer_state_t));
    }

    clientStack->len--;
}

#undef maybe_free
#undef enable_disable
#undef v2
#undef v3
#undef v4
