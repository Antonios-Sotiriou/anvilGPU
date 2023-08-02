/* general headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <unistd.h>
#include <sys/time.h>

/* OpenGL headers. */
#include <GL/glew.h>     /* libglew-dev */
#include <GL/glx.h>     /* libglx-dev */
#include <GL/glu.h>     /* libglu1-mesa-dev */

/* OpenGL shader. */
const char *vertexShaderSource = "#version 450 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"

    "layout (location = 0) out vec4 bPos;\n"
    "layout (location = 1) out vec4 newColor;\n"

    "void main() {\n"
    "    bPos = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "    gl_Position = bPos;\n"
    "    newColor = vec4(aColor, 1.0f);\n"
    "}\n\0";
unsigned int vertexShader;
const char *fragmentShaderSource = "#version 450 core\n"
    "layout (location = 0) in vec4 bPos;\n"
    "layout (location = 1) in vec4 newColor;\n"

    "layout (location = 0) out vec4 FragColor;\n"

    "void main() {\n"
    "    FragColor = newColor;\n"
    "}\n\0";
unsigned int fragmentShader;
unsigned int shaderProgram;

/* signal */
#include <signal.h>
// #include <immintrin.h>

/* Project specific headers */
#include "headers/locale.h"
#include "headers/anvil_structs.h"
#include "headers/matrices.h"
#include "headers/kinetics.h"
#include "headers/grfk_pipeline.h"
#include "headers/camera.h"
#include "headers/world_objects.h"
#include "headers/general_functions.h"

/* testing */
#include "headers/exec_time.h"
#include "headers/logging.h"
#include "headers/test_shapes.h"

enum { Win_Close, Win_Name, Atom_Type, Atom_Last};
enum { Pos, U, V, N, C };

#define WIDTH                     1000
#define HEIGHT                    1000
#define POINTERMASKS              ( ButtonPressMask )
#define KEYBOARDMASKS             ( KeyPressMask )
#define EXPOSEMASKS               ( StructureNotifyMask )

/* X Global Structures. */
Display *displ;
Window root, win;
Pixmap pixmap;
GC gc;
XGCValues gcvalues;
XWindowAttributes wa;
XSetWindowAttributes sa;
Atom wmatom[Atom_Last];

/* OpenGl Global variables. */
GLXContext              glc;
Colormap                cmap;
XVisualInfo             *vinfo;
GLint                   att[]   = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };

/* BUFFERS. */
u_int8_t *frame_buffer;

/* Project Global Variables. */
static float FOV          = 45.0;
static float ZNEAR        = 0.01;
static float ZFAR         = 1000.0;
float FPlane              = 0.00001;
float NPlane              = 1.0;
static float ASPECTRATIO  = 1;
static int EYEPOINT       = 0;

/* Camera and Global light Source. */
vec4f camera[N + 1] = {
    { 0.0, 0.0, 0.0, 1.0 },
    { 1.0, 0.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0, 0.0 },
    { 0.0, 0.0, -1.0, 0.0 }
};
vec4f light[C + 1] = {
    { -56.215076, -47.867058, 670.036438, 1.0 },
    { -0.907780, -0.069064, -0.413726, 0.0 },
    { -0.178108, 0.956481, 0.231131, 0.0 },
    { 0.379759, 0.283504, -0.880576, 0.0 },
    { 1.0, 1.0, 1.0}
};

/* Global Matrices */
Mat4x4 perspMat, lookAt, viewMat, worldMat;

/* Anvil global Objects Meshes and Scene. */
Scene scene = { 0 };

/* X11 and window Global variables. */
static int INIT = 0;
static int RUNNING = 1;
int HALFW = 0; // Half width of the screen; This variable is initialized in configurenotify function.Its Helping us decrease the number of divisions.
int HALFH = 0; // Half height of the screen; This variable is initialized in configurenotify function.Its Helping us decrease the number of divisions.
static int DEBUG = 0;
int VAO_CREATED = 0;

/* Display usefull measurements. */
float			        TimeCounter, LastFrameTimeCounter, DeltaTime, prevTime = 0.0, FPS;
struct timeval		    tv, tv0;
int			            Frame = 1, FramesPerFPS;

/* Event handling functions. */
const static void clientmessage(XEvent *event);
const static void reparentnotify(XEvent *event);
const static void mapnotify(XEvent *event);
const static void resizerequest(XEvent *event);
const static void configurenotify(XEvent *event);
const static void buttonpress(XEvent *event);
const static void keypress(XEvent *event);

/* Represantation functions */
const static void project(void);
const static void drawFrame(void);

/* Xlib relative functions and event dispatcher. */
const static KeySym getKeysym(XEvent *event);
const static void initMainWindow(void);
const static void setupGL(void);
const static void InitTimeCounter(void);
const static void UpdateTimeCounter(void);
const static void CalculateFPS(void);
const static void displayInfo(void);
const static void initGlobalGC(void);
const static void initDependedVariables(void);
const static void atomsinit(void);
const static void sigsegv_handler(const int sig);
const static int registerSig(const int signal);
const static void initBuffers(void);
static int board(void);
static void (*handler[LASTEvent]) (XEvent *event) = {
    [ClientMessage] = clientmessage,
    [ReparentNotify] = reparentnotify,
    [MapNotify] = mapnotify,
    [ResizeRequest] = resizerequest,
    [ConfigureNotify] = configurenotify,
    [ButtonPress] = buttonpress,
    [KeyPress] = keypress,
};

const static void clientmessage(XEvent *event) {
    printf("Received client message event\n");
    if (event->xclient.data.l[0] == wmatom[Win_Close]) {

        releaseScene(&scene);

        free(frame_buffer);
        XFreeGC(displ, gc);
        XFree(vinfo);

        glDeleteProgram(shaderProgram);
        glXMakeCurrent(displ, None, NULL);
        glXDestroyContext(displ, glc);
        XDestroyWindow(displ, win);

        RUNNING = 0;
    }
}
const static void reparentnotify(XEvent *event) {

    printf("reparentnotify event received\n");
}
const static void mapnotify(XEvent *event) {

    printf("mapnotify event received\n");
}
const static void resizerequest(XEvent *event) {

    printf("resizerequest event received\n");
}
const static void configurenotify(XEvent *event) {

    if (!event->xconfigure.send_event) {
        printf("configurenotify event received\n");
        int old_height = wa.height;
        XGetWindowAttributes(displ, win, &wa);

        if (INIT) {
            free(frame_buffer);
            initBuffers();

            initDependedVariables();
        }

        if (!INIT) {
            INIT = 1;
        }
    }
}
const static void buttonpress(XEvent *event) {

    printf("buttonpress event received\n");
    printf("X: %f\n", ((event->xbutton.x - (WIDTH / 2.00)) / (WIDTH / 2.00)));
    printf("Y: %f\n", ((event->xbutton.y - (HEIGHT / 2.00)) / (HEIGHT / 2.00)));
}

const static void keypress(XEvent *event) {
    
    KeySym keysym = XLookupKeysym(&event->xkey, 0);

    vec4f *eye;
    if (EYEPOINT)
        eye = &light[0];
    else
        eye = &camera[0];

    printf("Key Pressed: %ld\n", keysym);
    printf("\x1b[H\x1b[J");
    switch (keysym) {
        case 97 : look_left(eye, 0.2);       /* a */
            break;
        case 100 : look_right(eye, 0.2);     /* d */
            break;
        case 119 : move_forward(eye, 0.2);         /* w */
            break;
        case 115 : move_backward(eye, 0.2);        /* s */
            break;
        case 65361 : move_left(eye, 0.2);          /* left arrow */
            break;
        case 65363 : move_right(eye, 0.2);         /* right arrow */
            break;
        case 65362 : move_up(eye, 0.2);            /* up arror */
            break;
        case 65364 : move_down(eye, 0.2);          /* down arrow */
            break;
        case 120 : rotate_x(&scene.m[0], 1.2);                     /* x */
            break;
        case 121 : rotate_y(&scene.m[0], 1.2);                     /* y */
            break;
        case 122 : rotate_z(&scene.m[0], 1.2);                     /* z */
            break;
        // case 114 : rotate_light(light, 1, 0.0, 1.0, 0.0);        /* r */
        //     break;
        // case 99 : rotate_origin(&scene.m[2], 1, 1.0, 0.0, 0.0);  /* c */
        //     break;
    }
    // lookAt = lookat(eye[Pos], eye[U], eye[V], eye[N]);
    // viewMat = inverse_mat(lookAt);
    // project();
}
// ##############################################################################################################################################
/* Starts the Projection Pipeline. */ // ########################################################################################################
const void initfaceVertices(Mesh *m, const int len) {
    for (int i = 0; i < len; i++) {
        m->f[i].v[0] = m->v[m->f[i].a[0] - 1];
        m->f[i].v[1] = m->v[m->f[i].b[0] - 1];
        m->f[i].v[2] = m->v[m->f[i].c[0] - 1];

        m->f[i].vt[0] = m->t[m->f[i].a[1] - 1];
        m->f[i].vt[1] = m->t[m->f[i].b[1] - 1];
        m->f[i].vt[2] = m->t[m->f[i].c[1] - 1];

        m->f[i].vn[0] = m->n[m->f[i].a[2] - 1];
        m->f[i].vn[1] = m->n[m->f[i].b[2] - 1];
        m->f[i].vn[2] = m->n[m->f[i].c[2] - 1];
    }
}
void createVAO(Mesh *m) {
    const int len = 32 * (m->f_indexes * 3);
    m->VAO = malloc(len);
    if (!m->VAO)
        perror("Error: ");

    int vao_inc = 0;
    for (int i = 0; i < m->f_indexes; i++) {
        memcpy(&m->VAO[vao_inc], &m->f[i].v[0], 12);
        memcpy(&m->VAO[vao_inc + 3], &m->f[i].vt[0], 8);
        memcpy(&m->VAO[vao_inc + 5], &m->f[i].vn[0], 12);
        vao_inc += 8;
        memcpy(&m->VAO[vao_inc], &m->f[i].v[1], 12);
        memcpy(&m->VAO[vao_inc + 3], &m->f[i].vt[1], 8);
        memcpy(&m->VAO[vao_inc + 5], &m->f[i].vn[1], 12);
        vao_inc += 8;
        memcpy(&m->VAO[vao_inc], &m->f[i].v[2], 12);
        memcpy(&m->VAO[vao_inc + 3], &m->f[i].vt[2], 8);
        memcpy(&m->VAO[vao_inc + 5], &m->f[i].vn[2], 12);
        vao_inc += 8;
    }
}
const static void project() {

    // if (!VAO_CREATED) {
        initfaceVertices(&scene.m[0], scene.m[0].f_indexes);
        createVAO(&scene.m[0]);
        VAO_CREATED = 1;
    // }

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    // 0. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, scene.m[0].f_indexes * 32 * 3, scene.m[0].VAO, GL_STATIC_DRAW);
    // 1. then set the vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 1. then set the color attributes pointers
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // 3. use our shader program when we want to render an object
    glUseProgram(shaderProgram);

    // glEnableClientState(GL_VERTEX_ARRAY);
    // glVertexPointer(3, GL_FLOAT, 0, vertices);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, scene.m[0].f_indexes * 3);

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        fprintf(stderr, "An OpenGL ERROR has occured: %s", err);
    }
    glXSwapBuffers(displ, win);

    // glDisableClientState(GL_VERTEX_ARRAY);

    // 4. Unbinding the Vertex and Buffer arrays. 
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}
/* Starts the Projection Pipeline. */ // ########################################################################################################
// ##############################################################################################################################################
const static void initMainWindow(void) {
    int screen = XDefaultScreen(displ);
    if ( (vinfo = glXChooseVisual(displ, screen, att)) == 0 ) {
        fprintf(stderr, "No matching visual. initMainWindow() -- glxChooseVisual().\n");
        exit(0);
    }

    /* The root window which is controled by the window manager. */
    root = DefaultRootWindow(displ);
    if ( (cmap = XCreateColormap(displ, root, vinfo->visual, AllocNone)) == 0 ) {
        fprintf(stderr, "Unable to create colormap. initMainWindow() -- XCreateColormap().\n");
        exit(0);
    }

    sa.event_mask = EXPOSEMASKS | KEYBOARDMASKS | POINTERMASKS;
    sa.background_pixel = 0x000000;
    sa.colormap = cmap;
    win = XCreateWindow(displ, XRootWindow(displ, XDefaultScreen(displ)), 0, 0, WIDTH, HEIGHT, 0, vinfo->depth, InputOutput, vinfo->visual, CWBackPixel | CWEventMask | CWColormap, &sa);
    XMapWindow(displ, win);
    XGetWindowAttributes(displ, win, &wa);
}
////////////////////////////////////
//        SETUP GL CONTEXT        //
////////////////////////////////////
const static void setupGL(void) {
    /////////////////////////////////////////////////
    //	CREATE GL CONTEXT AND MAKE IT CURRENT	//
    /////////////////////////////////////////////////
    if ( (glc = glXCreateContext(displ, vinfo, NULL, GL_TRUE)) == 0 ) {
        fprintf(stderr, "Unable to create gl context. setupGL() -- glxCreateContext().\n");
        exit(0);
    }

    glXMakeCurrent(displ, win, glc);
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CCW);
    // glEnable(GL_CULL_FACE);
    glClearColor(0.00, 0.00, 0.00, 1.00);

    /* Initialize Glew and check for Errors. */
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
}
const static void InitTimeCounter(void) {
    gettimeofday(&tv0, NULL);
    FramesPerFPS = 30;
}
const static void UpdateTimeCounter(void) {
    LastFrameTimeCounter = TimeCounter;
    gettimeofday(&tv, NULL);
    TimeCounter = (float)(tv.tv_sec - tv0.tv_sec) + 0.000001 * ((float)(tv.tv_usec - tv0.tv_usec));
    DeltaTime = TimeCounter - LastFrameTimeCounter;
}
const static void CalculateFPS(void) {
    Frame ++;

    if ( (Frame % FramesPerFPS) == 0 ) {
        FPS = ((float)(FramesPerFPS)) / (TimeCounter-prevTime);
        prevTime = TimeCounter;
    }
}
const static void displayInfo(void) {
    char info_string[50];

    sprintf(info_string, "Resolution: %d x %d\0", wa.width, wa.height);
    XDrawString(displ ,win ,gc, 5, 12, info_string, strlen(info_string));

    sprintf(info_string, "Running Time: %4.1f\0", TimeCounter);
    XDrawString(displ ,win ,gc, 5, 24, info_string, strlen(info_string));

    sprintf(info_string, "%4.1f fps\0", FPS);
    XDrawString(displ ,win ,gc, 5, 36, info_string, strlen(info_string));
}
const static void initGlobalGC(void) {
    gcvalues.foreground = 0xffffff;
    gcvalues.background = 0x000000;
    gcvalues.graphics_exposures = False;
    gc = XCreateGC(displ, win, GCBackground | GCForeground | GCGraphicsExposures, &gcvalues);
}
const static void atomsinit(void) {

    wmatom[Win_Close] = XInternAtom(displ, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(displ, win, &wmatom[Win_Close], 1);

    wmatom[Win_Name] = XInternAtom(displ, "WM_NAME", False);
    wmatom[Atom_Type] =  XInternAtom(displ, "STRING", False);
    XChangeProperty(displ, win, wmatom[Win_Name], wmatom[Atom_Type], 8, PropModeReplace, (unsigned char*)"Anvil", 5);
}
/* Signal handler to clean memory before exit, after receiving a given signal. */
const static void sigsegv_handler(const int sig) {
    printf("Received Signal from OS with ID: %d\n", sig);
    XEvent event = { 0 };
    event.type = ClientMessage;
    event.xclient.data.l[0] = wmatom[Win_Close];
    /* Send the signal to our event dispatcher for further processing. */
    if(RUNNING)
        handler[event.type](&event);
}
/* Registers the given Signal. */
const static int registerSig(const int signal) {
    /* Signal handling for effectivelly releasing the resources. */
    struct sigaction sig = { 0 };
    sig.sa_handler = &sigsegv_handler;
    int sig_val = sigaction(signal, &sig, NULL);
    if (sig_val == -1) {
        fprintf(stderr, "Warning: board() -- sigaction()\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
const static void initDependedVariables(void) {
    glViewport(0, 0, wa.width, wa.height);

    /* Vertex shader initialization. */
    int  success;
    char infoLog[512];
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::VERTEX::COMPILATION_FAILED.\n%s\n", infoLog);
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED.\n%s\n", infoLog);
    }

    /* Shaders programm creation and linking. */
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::PROGRAMM::LINKING_FAILED.\n%s\n", infoLog);
    }
    // glUseProgram(shaderProgram);
    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, fragmentShader);    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    ASPECTRATIO = ((float)wa.width / (float)wa.height);
    HALFH = wa.height >> 1;
    HALFW = wa.width >> 1;

    /* Matrices initialization. */
    perspMat = perspectiveMatrix(FOV, ASPECTRATIO, ZNEAR, ZFAR);
}
/* Creates and Initializes the importand buffers. (frame, depth, shadow). */
const static void initBuffers(void) {
    frame_buffer = calloc(wa.width * wa.height * 4, 1);
}
const static void announceReady(void) {
    printf("Announcing ready process state event\n");
    XEvent event = { 0 };
    event.xkey.type = KeyPress;
    event.xkey.keycode = 49;
    event.xkey.display = displ;

    /* Send the signal to our event dispatcher for further processing. */
    handler[event.type](&event);
}
/* General initialization and event handling. */
static int board(void) {

    if (!XInitThreads()) {
        fprintf(stderr, "Warning: board() -- XInitThreads()\n");
        return EXIT_FAILURE;
    }

    XEvent event;

    displ = XOpenDisplay(NULL);
    if (displ == NULL) {
        fprintf(stderr, "Warning: board() -- XOpenDisplay()\n");
        return EXIT_FAILURE;
    }

    initMainWindow();
    setupGL();
    InitTimeCounter();
    initGlobalGC();
    atomsinit();
    // registerSig(SIGSEGV);

    initDependedVariables();
    initBuffers();

    createScene(&scene);
    posWorldObjects(&scene);

    /* Announcing to event despatcher that starting initialization is done. We send a Keyress event to Despatcher to awake Projection. */
    announceReady();

    while (RUNNING) {

        // clock_t start_time = start();
        UpdateTimeCounter();
        CalculateFPS();
        displayInfo();
        rotate_z(&scene.m[0], 0.2f);
        rotate_x(&scene.m[0], 0.3f);
        rotate_y(&scene.m[0], 0.4f);
        project();
        // end(start_time);

        while(XPending(displ)) {

            XNextEvent(displ, &event);

            if (handler[event.type])
                handler[event.type](&event);
        }
        usleep(1000);
    }

    return EXIT_SUCCESS;
}
const int main(int argc, char *argv[]) {

    if (argc > 1) {
        printf("argc: %d\n", argc);
        if (strcasecmp(argv[1], "Debug") == 0) {
            if (strcasecmp(argv[2], "1") == 0) {
                fprintf(stderr, "INFO : ENABLED DEBUG Level 1\n");
                DEBUG = 1;
            } else if (strcasecmp(argv[2], "2") == 0) {
                fprintf(stderr, "INFO : ENABLED DEBUG Level 2\n");
                DEBUG = 2;
            }
        }
    }

    if (board()) {
        fprintf(stderr, "ERROR: main() -- board()\n");
        return EXIT_FAILURE;
    }

    XCloseDisplay(displ);
    
    return EXIT_SUCCESS;
}

