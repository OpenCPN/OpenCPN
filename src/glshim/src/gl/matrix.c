#include "error.h"
#include "loader.h"
#include "matrix.h"
#include "types.h"
#include "remote.h"

#ifdef LOCAL_MATRIX
#define PROXY_MATRIX(name)
#else
#define PROXY_MATRIX(name) PROXY_GLES(name)
#endif

// helper functions
static matrix_state_t *get_matrix_state(GLenum mode) {
    matrix_state_t *m;
    switch (mode) {
        case GL_MODELVIEW:
            m = &state.matrix.model;
            break;
        case GL_PROJECTION:
            m = &state.matrix.projection;
            break;
        case GL_TEXTURE:
            m = &state.matrix.texture[state.texture.client];
            break;
    /* defined in ARB_imaging extension
        case GL_COLOR:
            m = &state.matrix.color;
            break;
    */
    }

    if (! m->init) {
        mat4_identity(&m->matrix);
        m->init = true;
    }
    return m;
}

static void transpose(GLfloat *out, const GLfloat *m) {
    mat4 tmp;
    mat4_load(&tmp, m);
    mat4_transpose(&tmp);
    mat4_save(&tmp, out);
}

static bool mvp_dirty = true;
static mat4 mvp;

static mat4 *get_matrix(GLenum mode) {
    return &get_matrix_state(mode)->matrix;
}

static mat4 *get_current_matrix() {
    return get_matrix(state.matrix.mode);
}

static matrix_state_t *get_current_state() {
    return get_matrix_state(state.matrix.mode);
}

static void update_mvp() {
    mat4 *model = get_matrix(GL_MODELVIEW);
    mat4 *projection = get_matrix(GL_PROJECTION);
    mvp = *projection;
    mat4_mul(&mvp, model);
    mvp_dirty = false;
}

static void upload_matrix() {
    GLfloat tmp[16];
    mat4_save(get_current_matrix(), tmp);
    if (state.remote) {
        forward_glLoadMatrixf(tmp);
    } else {
        LOAD_GLES(glLoadMatrixf);
        gles_glLoadMatrixf(tmp);
    }
}

// GL matrix functions
void glLoadIdentity() {
    PUSH_IF_COMPILING(glLoadIdentity);
    ERROR_IN_BLOCK();
    mvp_dirty = true;
    mat4_identity(get_current_matrix());
    PROXY_MATRIX(glLoadIdentity);
}

void glLoadMatrixf(const GLfloat *m) {
    PUSH_IF_COMPILING(glLoadMatrixf);
    ERROR_IN_BLOCK();
    mvp_dirty = true;
    mat4_load(get_current_matrix(), m);
    PROXY_MATRIX(glLoadMatrixf);
}

void glLoadTransposeMatrixf(const GLfloat *m) {
    PUSH_IF_COMPILING(glLoadTransposeMatrixf);
    ERROR_IN_BLOCK();
    GLfloat tmp[16];
    transpose(tmp, m);
    glLoadMatrixf(tmp);
}

void glMatrixMode(GLenum mode) {
    PUSH_IF_COMPILING(glMatrixMode);
    ERROR_IN_BLOCK();
    switch (mode) {
        case GL_MODELVIEW:
        case GL_PROJECTION:
        case GL_TEXTURE:
        case GL_COLOR:
            break;
        default:
            ERROR(GL_INVALID_ENUM);
    }
    state.matrix.mode = mode;
    PROXY_MATRIX(glMatrixMode);
}

void glMultMatrixf(const GLfloat *m) {
    PUSH_IF_COMPILING(glMultMatrixf);
    ERROR_IN_BLOCK();
    mvp_dirty = true;
    mat4 load, *cur = get_current_matrix();
    mat4_load(&load, m);
    mat4_mul(cur, &load);
    upload_matrix();
}

void glMultTransposeMatrixf(const GLfloat *m) {
    PUSH_IF_COMPILING(glMultTransposeMatrixf);
    ERROR_IN_BLOCK();
    GLfloat tmp[16];
    transpose(tmp, m);
    glMultMatrixf(tmp);
}

void glPopMatrix() {
    PUSH_IF_COMPILING(glPopMatrix);
    ERROR_IN_BLOCK();
    mvp_dirty = true;
    matrix_state_t *m = get_current_state();
    mat4 *top = tack_pop(&m->stack);
    if (top == NULL) {
       ERROR(GL_STACK_UNDERFLOW);
    }
    m->matrix = *top;
    free(top);
    upload_matrix();
}

void glPushMatrix() {
    PUSH_IF_COMPILING(glPushMatrix);
    ERROR_IN_BLOCK();
    matrix_state_t *m = get_current_state();
    mat4 *push = malloc(sizeof(mat4));
    *push = m->matrix;
    tack_push(&m->stack, push);
}

// GL transform functions
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
    PUSH_IF_COMPILING(glRotatef);
    ERROR_IN_BLOCK();
    mvp_dirty = true;
    mat4_rotate(get_current_matrix(), angle, x, y, z);
    upload_matrix();
}

void glScalef(GLfloat x, GLfloat y, GLfloat z) {
    PUSH_IF_COMPILING(glScalef);
    ERROR_IN_BLOCK();
    mvp_dirty = true;
    mat4_scale(get_current_matrix(), x, y, z);
    upload_matrix();
}

void glTranslatef(GLfloat x, GLfloat y, GLfloat z) {
    PUSH_IF_COMPILING(glTranslatef);
    ERROR_IN_BLOCK();
    mvp_dirty = true;
    mat4_translate(get_current_matrix(), x, y, z);
    upload_matrix();
}

void glOrthof(GLfloat left, GLfloat right,
              GLfloat bottom, GLfloat top,
              GLfloat near, GLfloat far) {
    PUSH_IF_COMPILING(glOrthof);
    ERROR_IN_BLOCK();
    if (left == right || bottom == top || near == far) {
        ERROR(GL_INVALID_VALUE);
    }
    mvp_dirty = true;
    mat4_ortho(get_current_matrix(), left, right, bottom, top, near, far);
    upload_matrix();
}

void glFrustumf(GLfloat left, GLfloat right,
                GLfloat bottom, GLfloat top,
                GLfloat near, GLfloat far) {
    PUSH_IF_COMPILING(glFrustumf);
    ERROR_IN_BLOCK();
    if (near < 0 || far < 0 || left == right || bottom == top || near == far) {
        ERROR(GL_INVALID_VALUE);
    }
    mvp_dirty = true;
    mat4_frustum(get_current_matrix(), left, right, bottom, top, near, far);
    upload_matrix();
}

void gl_get_matrix(GLenum mode, GLfloat *out) {
    mat4_save(get_matrix(mode), out);
}

void gl_transform_light(GLfloat out[4], const GLfloat in[4]) {
    mat4 *model = get_matrix(GL_MODELVIEW);
    mat4_mul_vec4(model, out, in);
}

void gl_transform_texture(GLenum texture, GLfloat out[2], const GLfloat in[2]) {
    matrix_state_t *unit = &state.matrix.texture[texture - GL_TEXTURE0];
    if (! unit->init) {
        out[0] = in[0];
        out[1] = in[1];
    } else {
        mat4_mul_vec2(&unit->matrix, out, in);
    }
}

void gl_transform_vertex(GLfloat out[3], GLfloat in[3]) {
    if (mvp_dirty) {
        update_mvp();
    }
    mat4_mul_vec3(&mvp, out, in);
}
