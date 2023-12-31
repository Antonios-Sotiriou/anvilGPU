#ifndef _ANVIL_STRUCTS_H
#define _ANVIL_STRUCTS_H 1

#ifndef _XLIB_H
    #include <X11/Xlib.h>
#endif

#ifndef _IMMINTRIN_H
    #include <immintrin.h>
#endif

/* Primitive struct vec4 with 4 x 32bits integers as members */
typedef int vec4i __attribute__((vector_size(16)));
typedef union {
    __m128i mm;
    vec4i i;
} v128i;

/* Primitive struct vec4 with 4 x 32bits floats as members */
typedef float vec4f __attribute__((vector_size(16)));
typedef union {
    __m128 mm;
    vec4f f;
} v128f;

/* Material struct to hold the specific for each material values. */
typedef struct {
    vec4f basecolor;
    vec4f ambient;
    vec4f diffuse;
    vec4f specular;
    float shinniness;
} Material;

typedef struct {
    vec4f v[3];
    vec4f vn[3];
    vec4f vt[3];
    int a[3], b[3], c[3];
} face;

/* Initialization matrix */
typedef struct {
    vec4f m[4];
} Mat4x4;

/* Mesh struct which teams all the primitives like faces, vector arrays and textures. */
typedef struct {
    char texture_file[50];
    vec4f *v;
    vec4f *n;
    vec4f *t;
    face *f;
    unsigned int *i;
    float *VAO;
    // void (*drawMesh)(void *args);
    signed int texture_height, texture_width;
    int v_indexes, f_indexes, n_indexes, t_indexes, i_indexes;
    Material material;
} Mesh;

/* Scene structs which teams all the meshes into an objects array. */
typedef struct {
    Mesh *m;
    int m_indexes;
} Scene;

/* Light struct to create different kind of light sources. */
typedef struct {
    vec4f pos, u, v, n, newPos;
    Material material;
} Light;

#endif /* _ANVIL_STRUCTS_H */

