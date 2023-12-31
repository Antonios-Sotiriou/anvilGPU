#include "headers/kinetics.h"

// extern Light sunlight;
extern const void initMesh(Mesh *a, const Mesh b);
extern const void releaseMesh(Mesh *c);
extern const float radians(const float value);

/* Rotates object according to World X axis. */
const void rotate_x(Mesh *c, const float angle) {
    Mesh cache = *c;
    initMesh(&cache, *c);
    Mat4x4 m = rotateXMatrix(radians(angle));
    c->v = meshxm(cache.v, cache.v_indexes, m);
    c->n = meshxm(cache.n, cache.n_indexes, m);
    releaseMesh(&cache);
}
/* Rotates object according to World Y axis. */
const void rotate_y(Mesh *c, const float angle) {
    Mesh cache;
    initMesh(&cache, *c);
    Mat4x4 m = rotateYMatrix(radians(angle));
    c->v = meshxm(cache.v, cache.v_indexes, m);
    c->n = meshxm(cache.n, cache.n_indexes, m);
    releaseMesh(&cache);
}
/* Rotates object according to World Z axis. */
const void rotate_z(Mesh *c, const float angle) {
    Mesh cache;
    initMesh(&cache, *c);
    Mat4x4 m = rotateZMatrix(radians(angle));
    c->v = meshxm(cache.v, cache.v_indexes, m);
    c->n = meshxm(cache.n, cache.n_indexes, m);
    releaseMesh(&cache);
}
/* Rotates object according to own axis. */
const void rotate_origin(Mesh *c, const float angle, float x, float y, float z) {
    vec4f pos = { 0.0, 0.0, 0.0 };
    vec4f axis = { x, y, z };
    Quat n = setQuat(0, pos);

    Quat xrot = rotationQuat(angle, axis);
    Mat4x4 m = MatfromQuat(xrot, n.v);

    Mesh cache;
    initMesh(&cache, *c);
    c->v = meshxm(cache.v, cache.v_indexes, m);
    c->n = meshxm(cache.n, cache.n_indexes, m);
    releaseMesh(&cache);
}
/* Rotates light arround scene center. */
const void rotate_light(Light *l, const float angle, float x, float y, float z) {
    vec4f pos = { 0.0, 0.0, 498.0 };
    vec4f axis = { x, y, z };
    Quat n = setQuat(0, pos);

    Quat xrot = rotationQuat(1, axis);
    Mat4x4 m = MatfromQuat(xrot, n.v);

    l->pos = vecxm(l->pos, m);
    l->u = vecxm(l->u, m);
    l->v = vecxm(l->v, m);
    l->n = vecxm(l->n, m);
}


