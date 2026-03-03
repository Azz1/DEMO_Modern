
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#ifndef GLAPI
#define GLAPI extern
#endif
typedef void* (*GLADloadproc)(const char *name);
typedef unsigned int GLenum;
typedef unsigned int GLbitfield;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLboolean;
typedef signed char GLbyte;
typedef short GLshort;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned long GLulong;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;

// Constants
#define GL_DEPTH_BUFFER_BIT    0x00000100
#define GL_COLOR_BUFFER_BIT    0x00004000
#define GL_TRUE                1
#define GL_FALSE               0
#define GL_TRIANGLES           0x0004
#define GL_LINE_STRIP          0x0003
#define GL_POINTS              0x0000
#define GL_FLOAT               0x1406
#define GL_UNSIGNED_INT        0x1405
#define GL_ARRAY_BUFFER        0x8892
#define GL_STATIC_DRAW         0x88B4
#define GL_DYNAMIC_DRAW        0x88E8
#define GL_FRAGMENT_SHADER     0x8B30
#define GL_VERTEX_SHADER       0x8B31
#define GL_COMPILE_STATUS      0x8B81
#define GL_LINK_STATUS         0x8B82
#define GL_INFO_LOG_LENGTH     0x8B84
#define GL_BLEND               0x0BE2
#define GL_SRC_ALPHA           0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_PROGRAM_POINT_SIZE  0x8642
#define GL_VIEWPORT            0x0BA2
#define GL_RENDERER            0x1F01
#define GL_VERSION             0x1F02
#define GL_INVALID_ENUM        0x0500
#define GL_NO_ERROR            0

// Function pointers
typedef void (*PFNGLCLEARCOLORPROC)(GLfloat,GLfloat,GLfloat,GLfloat);
typedef void (*PFNGLCLEARPROC)(GLbitfield);
typedef void (*PFNGLVIEWPORTPROC)(GLint,GLint,GLsizei,GLsizei);
typedef void (*PFNGLENABLEPROC)(GLenum);
typedef void (*PFNGLDISABLEPROC)(GLenum);
typedef void (*PFNGLBLENDFUNCPROC)(GLenum,GLenum);
typedef void (*PFNGLGENVERTEXARRAYSPROC)(GLsizei,GLuint*);
typedef void (*PFNGLBINDVERTEXARRAYPROC)(GLuint);
typedef void (*PFNGLGENBUFFERSPROC)(GLsizei,GLuint*);
typedef void (*PFNGLBINDBUFFERPROC)(GLenum,GLuint);
typedef void (*PFNGLBUFFERDATAPROC)(GLenum,GLsizeiptr,const void*,GLenum);
typedef void (*PFNGLVERTEXATTRIBPOINTERPROC)(GLuint,GLint,GLenum,GLboolean,GLsizei,const void*);
typedef void (*PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint);
typedef GLuint (*PFNGLCREATESHADERPROC)(GLenum);
typedef void (*PFNGLSHADERSOURCEPROC)(GLuint,GLsizei,const GLchar**,const GLint*);
typedef void (*PFNGLCOMPILESHADERPROC)(GLuint);
typedef GLuint (*PFNGLCREATEPROGRAMPROC)(void);
typedef void (*PFNGLATTACHSHADERPROC)(GLuint,GLuint);
typedef void (*PFNGLLINKPROGRAMPROC)(GLuint);
typedef void (*PFNGLUSEPROGRAMPROC)(GLuint);
typedef void (*PFNGLDELETESHADERPROC)(GLuint);
typedef void (*PFNGLDELETEVERTEXARRAYSPROC)(GLsizei,const GLuint*);
typedef void (*PFNGLDELETEBUFFERSPROC)(GLsizei,const GLuint*);
typedef void (*PFNGLDELETEPROGRAMPROC)(GLuint);
typedef GLint (*PFNGLGETUNIFORMLOCATIONPROC)(GLuint,const GLchar*);
typedef void (*PFNGLUNIFORMMATRIX4FVPROC)(GLint,GLsizei,GLboolean,const GLfloat*);
typedef void (*PFNGLUNIFORM3FPROC)(GLint,GLfloat,GLfloat,GLfloat);
typedef void (*PFNGLDRAWARRAYSPROC)(GLenum,GLint,GLsizei);
typedef GLenum (*PFNGLGETERRORPROC)(void);
typedef const GLubyte* (*PFNGLGETSTRINGPROC)(GLenum);
typedef void (*PFNGLPOINTPARAMETERFPROC)(GLenum,GLfloat);

// Declare function pointers as externs
GLAPI PFNGLCLEARCOLORPROC              glad_glClearColor;
GLAPI PFNGLCLEARPROC                   glad_glClear;
GLAPI PFNGLVIEWPORTPROC                glad_glViewport;
GLAPI PFNGLENABLEPROC                  glad_glEnable;
GLAPI PFNGLDISABLEPROC                 glad_glDisable;
GLAPI PFNGLBLENDFUNCPROC               glad_glBlendFunc;
GLAPI PFNGLGENVERTEXARRAYSPROC         glad_glGenVertexArrays;
GLAPI PFNGLBINDVERTEXARRAYPROC         glad_glBindVertexArray;
GLAPI PFNGLGENBUFFERSPROC              glad_glGenBuffers;
GLAPI PFNGLBINDBUFFERPROC              glad_glBindBuffer;
GLAPI PFNGLBUFFERDATAPROC              glad_glBufferData;
GLAPI PFNGLVERTEXATTRIBPOINTERPROC     glad_glVertexAttribPointer;
GLAPI PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray;
GLAPI PFNGLCREATESHADERPROC            glad_glCreateShader;
GLAPI PFNGLSHADERSOURCEPROC            glad_glShaderSource;
GLAPI PFNGLCOMPILESHADERPROC           glad_glCompileShader;
GLAPI PFNGLCREATEPROGRAMPROC           glad_glCreateProgram;
GLAPI PFNGLATTACHSHADERPROC            glad_glAttachShader;
GLAPI PFNGLLINKPROGRAMPROC             glad_glLinkProgram;
GLAPI PFNGLUSEPROGRAMPROC              glad_glUseProgram;
GLAPI PFNGLDELETESHADERPROC            glad_glDeleteShader;
GLAPI PFNGLDELETEVERTEXARRAYSPROC      glad_glDeleteVertexArrays;
GLAPI PFNGLDELETEBUFFERSPROC           glad_glDeleteBuffers;
GLAPI PFNGLDELETEPROGRAMPROC           glad_glDeleteProgram;
GLAPI PFNGLGETUNIFORMLOCATIONPROC      glad_glGetUniformLocation;
GLAPI PFNGLUNIFORMMATRIX4FVPROC        glad_glUniformMatrix4fv;
GLAPI PFNGLUNIFORM3FPROC               glad_glUniform3f;
GLAPI PFNGLDRAWARRAYSPROC              glad_glDrawArrays;
GLAPI PFNGLGETERRORPROC                glad_glGetError;
GLAPI PFNGLGETSTRINGPROC               glad_glGetString;

#define glClearColor              glad_glClearColor
#define glClear                   glad_glClear
#define glViewport                glad_glViewport
#define glEnable                  glad_glEnable
#define glDisable                 glad_glDisable
#define glBlendFunc               glad_glBlendFunc
#define glGenVertexArrays         glad_glGenVertexArrays
#define glBindVertexArray         glad_glBindVertexArray
#define glGenBuffers              glad_glGenBuffers
#define glBindBuffer              glad_glBindBuffer
#define glBufferData              glad_glBufferData
#define glVertexAttribPointer     glad_glVertexAttribPointer
#define glEnableVertexAttribArray glad_glEnableVertexAttribArray
#define glCreateShader            glad_glCreateShader
#define glShaderSource            glad_glShaderSource
#define glCompileShader           glad_glCompileShader
#define glCreateProgram           glad_glCreateProgram
#define glAttachShader            glad_glAttachShader
#define glLinkProgram             glad_glLinkProgram
#define glUseProgram              glad_glUseProgram
#define glDeleteShader            glad_glDeleteShader
#define glDeleteVertexArrays      glad_glDeleteVertexArrays
#define glDeleteBuffers           glad_glDeleteBuffers
#define glDeleteProgram           glad_glDeleteProgram
#define glGetUniformLocation      glad_glGetUniformLocation
#define glUniformMatrix4fv        glad_glUniformMatrix4fv
#define glUniform3f               glad_glUniform3f
#define glDrawArrays              glad_glDrawArrays
#define glGetError                glad_glGetError
#define glGetString               glad_glGetString

int gladLoadGL(GLADloadproc load);

#ifdef __cplusplus
}
#endif
