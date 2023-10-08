#include "headers/obj_parser.h"

static vec4f *loadvectors(const char path[]);
static vec4f *loadtextors(const char path[]);
static vec4f *loadnormals(const char path[]);
static face *loadfaces(const char path[]);
static unsigned int *loadindices(const char path[]);

/* vectors array indexes */
static int v_indexes = 0;
/* normals array indexes */
static int n_indexes = 0;
/* textures array indexes */
static int t_indexes = 0;
/* Face array indexes */
static int f_indexes = 0;
/* Indices array indexes */
static int i_indexes = 0;

const Mesh loadmesh(const char path[]) {
    Mesh r = { 0 };

    r.v = loadvectors(path);
    if (!r.v)
        fprintf(stderr, "Could not reallocate Vectors array. load_obj() - get_vectors()\n");
    r.v_indexes = v_indexes;

    r.t = loadtextors(path);
    if (!r.t)
        fprintf(stderr, "Could not create Vectors array. load_obj() - get_textors()\n");
    r.t_indexes = t_indexes;

    r.n = loadnormals(path);
    if (!r.n)
        fprintf(stderr, "Could not create Vectors array. load_obj() - get_normals()\n");
    r.n_indexes = n_indexes;

    r.f = loadfaces(path);
    if (!r.f)
        fprintf(stderr, "Could not create Faces array. load_obj() - get_faces()\n");
    r.f_indexes = f_indexes;

    r.i = loadindices(path);
    if (!r.i)
        fprintf(stderr, "Could not create Faces array. load_obj() - get_faces()\n");
    r.i_indexes = i_indexes;

    return r;
}
static face *loadfaces(const char path[]) {
    size_t facesize = sizeof(face);
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "Could not open file : %s.\n", path);
        return NULL;
    }

    face *f = malloc(facesize);
    if (!f) {
        fprintf(stderr, "Could not allocate memory for Face struct. load_obj() -- malloc().\n");
        fclose(fp);
        return NULL;
    }
    int dynamic_inc = 1;
    int index = 0;
    int va, vb, vc, vd, ve, vf, vg, vh, vi;
    char ch;
    while (!feof(fp)) {
        if ( (ch = getc(fp)) == 'f' )
            if ( (ch = getc(fp)) == ' ' )
                if (fscanf(fp, "%d/%d/%d %d/%d/%d %d/%d/%d",
                    &va, &vb, &vc,
                    &vd, &ve, &vf,
                    &vg, &vh, &vi ) == 9){

                    f = realloc(f, facesize * dynamic_inc);
                    f[index].a[0] = va - 1, f[index].a[1] = vb - 1, f[index].a[2] = vc - 1,
                    f[index].b[0] = vd - 1, f[index].b[1] = ve - 1, f[index].b[2] = vf - 1,
                    f[index].c[0] = vg - 1, f[index].c[1] = vh - 1, f[index].c[2] = vi - 1;


                    if (!f) {
                        fprintf(stderr, "Could not reallocate memory for Face struct array. load_obj() -- realloc().\n");
                        fclose(fp);
                        free(f);
                        return NULL;
                    }
                    index++;
                    dynamic_inc++;
                }
    }

    f_indexes = index;
    fclose(fp);
    return f;
}
static unsigned int *loadindices(const char path[]) {
    size_t indice_size = sizeof(unsigned int) * 3;
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "Could not open file : %s.\n", path);
        return NULL;
    }

    unsigned int *inx = malloc(indice_size);
    if (!inx) {
        fprintf(stderr, "Could not allocate memory for Face struct. load_obj() -- malloc().\n");
        fclose(fp);
        return NULL;
    }
    int dynamic_inc = 1;
    int index = 0;
    int va, vb, vc, vd, ve, vf, vg, vh, vi;
    char ch;
    while (!feof(fp)) {
        if ( (ch = getc(fp)) == 'f' )
            if ( (ch = getc(fp)) == ' ' )
                if (fscanf(fp, "%d/%d/%d %d/%d/%d %d/%d/%d",
                    &va, &vb, &vc,
                    &vd, &ve, &vf,
                    &vg, &vh, &vi ) == 9){

                    inx = realloc(inx, indice_size * dynamic_inc);
                    inx[index + 0] = va - 1;//, inx[index + 3] -= 1, inx[index + 6] -= 1,
                    inx[index + 1] = vd - 1;//, inx[index + 4] -= 1, inx[index + 7] -= 1,
                    inx[index + 2] = vg - 1;//, inx[index + 5] -= 1, inx[index + 8] -= 1;
                    if (!inx) {
                        fprintf(stderr, "Could not reallocate memory for Face struct array. load_obj() -- realloc().\n");
                        fclose(fp);
                        free(inx);
                        return NULL;
                    }
                    index += 3;
                    dynamic_inc++;
                }
    }

    i_indexes = index;
    fclose(fp);
    return inx;
}
static vec4f *loadvectors(const char path[]) {
    size_t vecsize = sizeof(vec4f);
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "Could not open file : %s.\n", path);
        return NULL;
    }

    vec4f *v = malloc(vecsize);
    if (!v) {
        fprintf(stderr, "Could not allocate memory for vec4f struct. get_vectors() -- calloc().\n");
        fclose(fp);
        return NULL;
    }

    int dynamic_inc = 2;
    int index = 0;

    char ch;
    while (!feof(fp)) {
        if ( (ch = getc(fp)) == 'v' )
            if ( (ch = getc(fp)) == ' ' )
                if (fscanf(fp, "%f %f %f", &v[index][0], &v[index][1], &v[index][2]) == 3) {

                    v = realloc(v, vecsize * dynamic_inc);
                    if (!v) {
                        fprintf(stderr, "Could not reallocate memory for vec4f struct array. load_obj() -- realloc().\n");
                        fclose(fp);
                        free(v);
                        return NULL;
                    }
                    v[index][3] = 1.00;

                    index++;
                    dynamic_inc++;
                }
    }
    v_indexes = index;
    fclose(fp);
    return v;
}
static vec4f *loadtextors(const char path[]) {
    size_t vecsize = sizeof(vec4f);
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "Could not open file : %s.\n", path);
        return NULL;
    }

    vec4f *t = malloc(vecsize);
    if (!t) {
        fprintf(stderr, "Could not allocate memory for Textor struct. get_textors() -- calloc().\n");
        fclose(fp);
        return NULL;
    }

    int dynamic_inc = 2;
    int index = 0;

    char ch;
    while (!feof(fp)) {
        if ( (ch = getc(fp)) == 'v' )
            if ( (ch = getc(fp)) == 't' )
                if ( (ch = getc(fp)) == ' ' )
                    if (fscanf(fp, "%f %f", &t[index][0], &t[index][1]) == 2) {

                        t = realloc(t, vecsize * dynamic_inc);
                        if (!t) {
                            fprintf(stderr, "Could not reallocate memory for vec4f struct array. get_textors() -- realloc().\n");
                            fclose(fp);
                            free(t);
                            return NULL;
                        }
                        t[index][2] = 0.0;
                        t[index][3] = 0.0;

                        index++;
                        dynamic_inc++;
                    }
    }
    t_indexes = index;
    fclose(fp);
    return t;
}
static vec4f *loadnormals(const char path[]) {
    size_t vecsize = sizeof(vec4f);
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "Could not open file : %s.\n", path);
        return NULL;
    }

    vec4f *n = malloc(vecsize);
    if (!n) {
        fprintf(stderr, "Could not allocate memory for Textor struct. get_normals() -- calloc().\n");
        fclose(fp);
        return NULL;
    }

    int dynamic_inc = 2;
    int index = 0;

    char ch;
    while (!feof(fp)) {
        if ( (ch = getc(fp)) == 'v' )
            if ( (ch = getc(fp)) == 'n' )
                if ( (ch = getc(fp)) == ' ' )
                    if (fscanf(fp, "%f %f %f", &n[index][0], &n[index][1], &n[index][2]) == 3) {

                        n = realloc(n, sizeof(vec4f) * dynamic_inc);
                        if (!n) {
                            fprintf(stderr, "Could not reallocate memory for vec4f struct array. load_obj() -- realloc().\n");
                            fclose(fp);
                            free(n);
                            return NULL;
                        }
                        n[index][3] = 0.0f;

                        index++;
                        dynamic_inc++;
                    }
    }
    n_indexes = index;
    fclose(fp);
    return n;
}
