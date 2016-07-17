#include <stdio.h>

#include "defines.h"
#include "error.h"
#include "gl_str.h"
#include "matrix.h"
#include "types.h"

GLint glRenderMode(GLenum mode) {
    // If the feedback data required more room than was available in buffer, glRenderMode returns a negative value
    int ret = 0;
    if (mode != GL_SELECT && mode != GL_FEEDBACK && mode != GL_RENDER) {
        gl_set_error(GL_INVALID_ENUM);
        return 0;
    }
    if (state.block.active) {
        gl_set_error(GL_INVALID_OPERATION);
        return 0;
    }
    if (state.render.mode == GL_SELECT) {
        ret = state.select.overflow ? -1 : state.select.count / 4;
    } else if (state.render.mode == GL_FEEDBACK) {
        ret = state.feedback.overflow ? -1 : state.feedback.values;
    }
    if (mode == GL_SELECT) {
        if (state.select.buffer == NULL) {
            gl_set_error(GL_INVALID_OPERATION);
            return 0;
        }
        state.select.count = 0;
    } else if (mode == GL_FEEDBACK) {
        if (state.feedback.buffer == NULL) {
            gl_set_error(GL_INVALID_OPERATION);
            return 0;
        }
        state.feedback.count = 0;
    }
    state.render.mode = mode;
    return ret;
}

void glInitNames() {
    ERROR_IN_BLOCK();
    tack_clear(&state.select.names);
}

void glPopName() {
    ERROR_IN_BLOCK();
    if (state.render.mode != GL_SELECT) {
        return;
    }
    if (tack_pop(&state.select.names) == NULL) {
        ERROR(GL_STACK_UNDERFLOW);
    }
}

void glPushName(GLuint name) {
    ERROR_IN_BLOCK();
    if (state.render.mode != GL_SELECT) {
        return;
    }
    tack_push_int(&state.select.names, name);
}

void glLoadName(GLuint name) {
    if (state.render.mode != GL_SELECT) {
        return;
    }
    ERROR_IN_BLOCK();
    int len = tack_len(&state.select.names);
    if (len > 0) {
        tack_set_int(&state.select.names, len - 1, name);
    } else {
        ERROR(GL_INVALID_OPERATION);
    }
}

void glSelectBuffer(GLsizei size, GLuint *buffer) {
    if (size < 0) {
        ERROR(GL_INVALID_VALUE);
    }
    if (state.block.active || state.render.mode == GL_SELECT) {
        ERROR(GL_INVALID_OPERATION);
    }
    state.feedback.overflow = false;
    state.select.buffer = buffer;
    state.select.size = size;
}

static bool test_point(GLfloat v[3]) {
    // test if a point is inside the viewport
    return (v[0] >= -1.0f && v[0] <= 1.0f && v[1] >= -1.0f && v[1] <= 1.0f);
}

static bool test_line(GLfloat a[3], GLfloat b[3]) {
    // test if a line intersects the viewport
    GLfloat vx, vy;
    vx = b[0] - a[0];
    vy = b[1] - a[1];
    GLfloat p[4] = {-vx, vx, -vy, vy};
    GLfloat q[4] = {a[0] + 1.0f, -1.0f - a[0], a[1] + 1.0f, -1.0f - a[1]};
    GLfloat u1 = -1e10;
    GLfloat u2 = 1e10;

    for (int i = 0; i < 4; i++) {
        if (p[i] == 0.0f) {
            if (q[i] < 0) {
                return false;
            }
        } else {
            GLfloat t = q[i] / p[i];
            if ((p[i] < 0.0f) && (u1 < t)) {
                u1 = t;
            } else if ((p[i] > 0.0f) && (u2 > t)) {
                u2 = t;
            }
        }
    }

    if ((u1 > u2) || (u1 > 1) || (u1 < 0)) {
        return false;
    }
    return true;
}

static bool test_tri(GLfloat a[3], GLfloat b[3], GLfloat c[3]) {
#define sign(p1, p2, p3) (p1[0]-p3[0])*(p2[1]-p3[1])-(p2[0]-p3[0])*(p1[1]-p3[1])
    // test if the viewport is inside a triangle
    for (int i = 0; i < 4; i++) {
        bool b1, b2, b3;
        GLfloat pt[2] = {
            (i % 2) ? -1.0f : 1.0f,
            (i > 2) ? -1.0f : 1.0f,
        };
        b1 = (sign(pt, a, b)) < 0.0f;
        b2 = (sign(pt, b, c)) < 0.0f;
        b3 = (sign(pt, c, a)) < 0.0f;
        if ((b1 == b2) && (b2 == b3)) {
            return true;
        }
    }
    return false;
#undef sign
}

static inline int _index(block_t *block, int i) {
    if (block->indices) {
        i = block->indices[i];
    }
    return i;
}

static inline GLfloat *_vert(block_t *block, int i) {
    return &block->vert[_index(block, i) * 3];
}

static void select_match(block_t *block, GLfloat zmin, GLfloat zmax, int i) {
#define push(val) state.select.buffer[state.select.count++] = val;
    GLfloat cur[3];
    for (; i < block->len; i++) {
        gl_transform_vertex(cur, _vert(block, i));
        zmin = MIN(zmin, cur[2]);
        zmax = MAX(zmax, cur[2]);
    }
    if (zmin < 0.0f) {
        zmin = 0.0f;
    }
    if (zmax > 1.0f) {
        zmax = 1.0f;
    }
    if (state.select.count + 4 < state.select.size) {
        push(state.select.count / 4);
        push(zmin * INT_MAX);
        push(zmax * INT_MAX);
        push(tack_peek_int(&state.select.names));
    }
#undef push
}

void gl_select_block(block_t *block) {
    if (block->len == 0) {
        return;
    }
    GLfloat zmax = 0.0f, zmin = 1.0f;
#define test(func, ...) if (test_##func(__VA_ARGS__)) { select_match(block, zmin, zmax, i); return; }
#define test_point(a) test(point, a)
#define test_line(a, b) test(line, a, b)
#define test_tri(a, b, c) test(tri, a, b, c)
    GLfloat data[3][3], first[3], *tmp;
    GLfloat *a = data[0], *b = data[1], *c = data[2];
    gl_transform_vertex(first, _vert(block, 0));
    for (int i = 0; i < block->len; i++) {
        tmp = c;
        c = b;
        b = a;
        a = tmp;
        gl_transform_vertex(a, _vert(block, i));
        zmin = MIN(zmin, a[2]);
        zmax = MAX(zmax, a[2]);
        switch (block->mode) {
            case GL_LINES:
                if (i % 2 == 1) {
                    test_point(a);
                    test_point(b);
                    test_line(a, b);
                }
                break;
            case GL_LINE_LOOP:
                // catch the loop segment
                if (i == block->len - 1) {
                    test_line(a, first);
                }
            case GL_LINE_STRIP:
                if (i > 0) {
                    if (i == 1) {
                        test_point(b);
                    }
                    test_point(a);
                    test_line(a, b);
                }
                break;
            case GL_TRIANGLES:
                if (i % 3 == 2) {
                    test_point(a);
                    test_point(b);
                    test_point(c);
                    test_line(a, b);
                    test_line(b, c);
                    test_line(c, a);
                    test_tri(a, b, c);
                }
            case GL_TRIANGLE_FAN:
                // this isn't *quite* right
                // as the point won't be drawn if we run out of verts
                test_point(a);
                if (i > 1) {
                    test_line(a, b);
                    test_line(a, first);
                    test_tri(first, a, b);
                }
                break;
            case GL_TRIANGLE_STRIP:
                test_point(a);
                if (i == 1) {
                    test_line(a, b);
                } else if (i > 1) {
                    test_line(a, b);
                    test_tri(a, b, c);
                }
                break;
            case GL_POINTS:
                test_point(a);
                break;
            default:
                printf("warning: unsupported GL_SELECT mode: %s\n", gl_str(block->mode));
                return;
        }
    }
#undef test_point
#undef test_line
#undef test_tri
}

void glFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer) {
    if (size < 0) {
        ERROR(GL_INVALID_VALUE);
    }
    if (state.block.active || state.render.mode == GL_FEEDBACK) {
        ERROR(GL_INVALID_OPERATION);
    }
    state.feedback.buffer = buffer;
    state.feedback.overflow = false;
    state.feedback.size = size;
    state.feedback.type = type;
    state.feedback.values = 0;
    switch (type) {
        case GL_2D:
        case GL_3D:
            break;
        case GL_3D_COLOR:
        case GL_3D_COLOR_TEXTURE:
        case GL_4D_COLOR_TEXTURE:
            printf("warning: GL_FEEDBACK does not transform color]n");
            break;
        default:
            ERROR(GL_INVALID_ENUM);
            break;
    }
}

static bool feedback_overflow(int n) {
    if (state.feedback.count + n > state.feedback.size) {
        state.feedback.overflow = true;
        return true;
    }
    return false;
}

static void feedback_push(GLfloat value) {
    if (state.render.mode != GL_FEEDBACK || !state.feedback.buffer || feedback_overflow(1)) {
        return;
    }
    state.feedback.values += 1;
    state.feedback.buffer[state.feedback.count++] = value;
}

static void feedback_push_n(GLfloat *values, int length) {
    for (int i = 0; i < length; i++) {
        feedback_push(values[i]);
    }
}

static void feedback_polygon(int n) {
    feedback_push(GL_POLYGON_TOKEN);
    feedback_push(n);
}

static int feedback_sizeof(GLenum type) {
    switch (type) {
        case GL_2D: return 2;
        case GL_3D: return 3;
        case GL_3D_COLOR: return 7;
        case GL_3D_COLOR_TEXTURE: return 11;
        case GL_4D_COLOR_TEXTURE: return 12;
        default:
            printf("warning: unknown feedback_sizeof(%s)\n", gl_str(type));
            return 0;
    }
}

static void feedback_vertex(block_t *block, int i) {
    static GLfloat color[] = {0, 0, 0, 1};
    static GLfloat tex[] = {0, 0, 0, 0};
    GLfloat v[4], *c, *t;
    c = block->color ?: color;
    // glFeedbackBuffer returns only the texture coordinate of texture unit GL_TEXTURE0.
    t = block->tex[0] ?: tex;

    // TODO: this will be called extra times on stuff like triangle strips
    gl_transform_vertex(v, &block->vert[i * 3]);
    switch (state.feedback.type) {
        case GL_2D:
            feedback_push_n(v, 2);
            break;
        case GL_3D:
            feedback_push_n(v, 3);
            break;
        case GL_3D_COLOR:
            feedback_push_n(v, 3);
            feedback_push_n(c, 4);
            break;
        case GL_3D_COLOR_TEXTURE:
            feedback_push_n(v, 3);
            feedback_push_n(c, 4);
            feedback_push_n(t, 2);
            // we only store 2d texture coordinates for now
            feedback_push(0.0f);
            feedback_push(0.0f);
            break;
        case GL_4D_COLOR_TEXTURE:
            feedback_push_n(v, 3);
            // our vertices are already normalized, so W is redundant here
            feedback_push(1.0f);
            feedback_push_n(c, 4);
            feedback_push_n(t, 4);
            break;
    }
}

void gl_feedback_block(block_t *block) {
    if (block->len == 0) {
        return;
    }
    int size = feedback_sizeof(state.feedback.type);

    GLfloat *v, *c, *t;
    int v1, v2, v3;
    int first = _index(block, 0);
    for (int j = 0; j < block->len; j++) {
        int i = _index(block, j);
        v2 = v1;
        v3 = v2;
        v1 = i;

        // TODO: overflow'd feedback returns -1 in glRenderMode
#define polygon(n) { if (feedback_overflow(size * n)) return; feedback_polygon(n); state.feedback.values -= 1; }
        switch (block->mode) {
            case GL_LINES:
                if (i % 2 == 1) {
                    polygon(2);
                    feedback_vertex(block, v2);
                    feedback_vertex(block, v1);
                }
                break;
            case GL_LINE_LOOP:
                // catch the loop segment
                if (i == block->len - 1) {
                    polygon(2);
                    feedback_vertex(block, v1);
                    feedback_vertex(block, first);
                }
            case GL_LINE_STRIP:
                if (i > 0) {
                    polygon(2);
                    feedback_vertex(block, v2);
                    feedback_vertex(block, v1);
                }
                break;
            case GL_TRIANGLES:
                if (i % 3 == 2) {
                    polygon(3);
                    feedback_vertex(block, v3);
                    feedback_vertex(block, v2);
                    feedback_vertex(block, v1);
                }
            case GL_TRIANGLE_FAN:
                if (i > 1) {
                    polygon(3);
                    feedback_vertex(block, v2);
                    feedback_vertex(block, v1);
                    feedback_vertex(block, first);
                }
                break;
            case GL_TRIANGLE_STRIP:
                if (i > 1) {
                    polygon(3);
                    feedback_vertex(block, v3);
                    feedback_vertex(block, v2);
                    feedback_vertex(block, v1);
                }
                break;
            case GL_POINTS:
                polygon(1);
                feedback_vertex(block, v1);
                break;
            default:
                printf("warning: unsupported GL_SELECT mode: %s\n", gl_str(block->mode));
                return;
        }
    }
}

void glPassThrough(GLfloat token) {
    ERROR_IN_BLOCK();
    if (feedback_overflow(2)) return;
    feedback_push(GL_PASS_THROUGH_TOKEN);
    feedback_push(token);
    state.feedback.values -= 1;
}
