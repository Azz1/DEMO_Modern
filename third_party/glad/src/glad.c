
#define GLAPI
#include "glad/glad.h"

PFNGLCLEARCOLORPROC              glad_glClearColor;
PFNGLCLEARPROC                   glad_glClear;
PFNGLVIEWPORTPROC                glad_glViewport;
PFNGLENABLEPROC                  glad_glEnable;
PFNGLDISABLEPROC                 glad_glDisable;
PFNGLBLENDFUNCPROC               glad_glBlendFunc;
PFNGLGENVERTEXARRAYSPROC         glad_glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC         glad_glBindVertexArray;
PFNGLGENBUFFERSPROC              glad_glGenBuffers;
PFNGLBINDBUFFERPROC              glad_glBindBuffer;
PFNGLBUFFERDATAPROC              glad_glBufferData;
PFNGLVERTEXATTRIBPOINTERPROC     glad_glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray;
PFNGLCREATESHADERPROC            glad_glCreateShader;
PFNGLSHADERSOURCEPROC            glad_glShaderSource;
PFNGLCOMPILESHADERPROC           glad_glCompileShader;
PFNGLCREATEPROGRAMPROC           glad_glCreateProgram;
PFNGLATTACHSHADERPROC            glad_glAttachShader;
PFNGLLINKPROGRAMPROC             glad_glLinkProgram;
PFNGLUSEPROGRAMPROC              glad_glUseProgram;
PFNGLDELETESHADERPROC            glad_glDeleteShader;
PFNGLDELETEVERTEXARRAYSPROC      glad_glDeleteVertexArrays;
PFNGLDELETEBUFFERSPROC           glad_glDeleteBuffers;
PFNGLDELETEPROGRAMPROC           glad_glDeleteProgram;
PFNGLGETUNIFORMLOCATIONPROC      glad_glGetUniformLocation;
PFNGLUNIFORMMATRIX4FVPROC        glad_glUniformMatrix4fv;
PFNGLUNIFORM3FPROC               glad_glUniform3f;
PFNGLDRAWARRAYSPROC              glad_glDrawArrays;
PFNGLGETERRORPROC                glad_glGetError;
PFNGLGETSTRINGPROC               glad_glGetString;

static void* get_proc(GLADloadproc load, const char* name) {
    return load(name);
}

int gladLoadGL(GLADloadproc load) {
    glad_glClearColor              = (PFNGLCLEARCOLORPROC)             get_proc(load, "glClearColor");
    glad_glClear                   = (PFNGLCLEARPROC)                  get_proc(load, "glClear");
    glad_glViewport                = (PFNGLVIEWPORTPROC)               get_proc(load, "glViewport");
    glad_glEnable                  = (PFNGLENABLEPROC)                 get_proc(load, "glEnable");
    glad_glDisable                 = (PFNGLDISABLEPROC)                get_proc(load, "glDisable");
    glad_glBlendFunc               = (PFNGLBLENDFUNCPROC)              get_proc(load, "glBlendFunc");
    glad_glGenVertexArrays         = (PFNGLGENVERTEXARRAYSPROC)        get_proc(load, "glGenVertexArrays");
    glad_glBindVertexArray         = (PFNGLBINDVERTEXARRAYPROC)        get_proc(load, "glBindVertexArray");
    glad_glGenBuffers              = (PFNGLGENBUFFERSPROC)             get_proc(load, "glGenBuffers");
    glad_glBindBuffer              = (PFNGLBINDBUFFERPROC)             get_proc(load, "glBindBuffer");
    glad_glBufferData              = (PFNGLBUFFERDATAPROC)             get_proc(load, "glBufferData");
    glad_glVertexAttribPointer     = (PFNGLVERTEXATTRIBPOINTERPROC)    get_proc(load, "glVertexAttribPointer");
    glad_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)get_proc(load, "glEnableVertexAttribArray");
    glad_glCreateShader            = (PFNGLCREATESHADERPROC)           get_proc(load, "glCreateShader");
    glad_glShaderSource            = (PFNGLSHADERSOURCEPROC)           get_proc(load, "glShaderSource");
    glad_glCompileShader           = (PFNGLCOMPILESHADERPROC)          get_proc(load, "glCompileShader");
    glad_glCreateProgram           = (PFNGLCREATEPROGRAMPROC)          get_proc(load, "glCreateProgram");
    glad_glAttachShader            = (PFNGLATTACHSHADERPROC)           get_proc(load, "glAttachShader");
    glad_glLinkProgram             = (PFNGLLINKPROGRAMPROC)            get_proc(load, "glLinkProgram");
    glad_glUseProgram              = (PFNGLUSEPROGRAMPROC)             get_proc(load, "glUseProgram");
    glad_glDeleteShader            = (PFNGLDELETESHADERPROC)           get_proc(load, "glDeleteShader");
    glad_glDeleteVertexArrays      = (PFNGLDELETEVERTEXARRAYSPROC)     get_proc(load, "glDeleteVertexArrays");
    glad_glDeleteBuffers           = (PFNGLDELETEBUFFERSPROC)          get_proc(load, "glDeleteBuffers");
    glad_glDeleteProgram           = (PFNGLDELETEPROGRAMPROC)          get_proc(load, "glDeleteProgram");
    glad_glGetUniformLocation      = (PFNGLGETUNIFORMLOCATIONPROC)     get_proc(load, "glGetUniformLocation");
    glad_glUniformMatrix4fv        = (PFNGLUNIFORMMATRIX4FVPROC)       get_proc(load, "glUniformMatrix4fv");
    glad_glUniform3f               = (PFNGLUNIFORM3FPROC)              get_proc(load, "glUniform3f");
    glad_glDrawArrays              = (PFNGLDRAWARRAYSPROC)             get_proc(load, "glDrawArrays");
    glad_glGetError                = (PFNGLGETERRORPROC)               get_proc(load, "glGetError");
    glad_glGetString               = (PFNGLGETSTRINGPROC)              get_proc(load, "glGetString");
    return glad_glGenVertexArrays != NULL;
}
