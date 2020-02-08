#include "render-ll.h"

#include "kn.h"

#include "assets.h"
#include "assets-fileio.h"
#include "color.h"
#include "compat-sdl.h"
#include "image.h"
#include "log.h"
#include "math4.h"
#include "memory.h"
#include "sprite.h"

#include <spa_fu/spa_fu.h>

#ifdef _WIN32
	#include "compat-windows.h"

	#include <GL/glew.h>
	#define GL_GLEXT_PROTOTYPES 1
	#include <GL/gl.h>
	#include <SDL_opengl_glext.h>
#else // linux
	// Get prototypes without manually loading each one.
	#define GL_GLEXT_PROTOTYPES 1
	#include <GL/gl.h>
	#include <GL/glext.h>
#endif

#if KN_DEBUG
	#define KN_ASSERT_NO_GL_ERROR() RLL_CheckGLError(__FILE__, __LINE__)
	const char* RLL_GLTypeToString(GLenum type);
	void RLL_PrintProgram(GLuint program);
	void RLL_PrintGLVersion(void);
	void RLL_CheckGLError(const char* file, int line);
#else
	#define KN_ASSERT_NO_GL_ERROR()
#endif

/**
 * The window on which to draw.
 */
extern struct SDL_Window* window;
static GLsizei windowWidth, windowHeight;

/**
 * The render system must carry around the OpenGL context to be used to draw to
 * the window.
 */
static SDL_GLContext* gl;

/**
 * Allocated number of 4-element vertices for specifically debug drawing.
 *
 * TODO: 128 might be too high, look into tracking used buffer space.
 */
#define RLL_MAX_DEBUG_POINTS 128

/**
 * Vertex `GL_ARRAY_BUFFER` containing vertex information for drawing debug shapes.
 */
static GLuint debugDrawBuffer;
static GLuint fullScreenQuadBuffer;
static GLuint spriteBuffer;

/**
 * Program to use for drawing debug shapes.
 */
static GLuint solidPolygonProgram;
static GLuint spriteProgram;
static GLuint fullScreenDebugProgram;

/*
 * TODO: Not supporting reusable sprites yet.
 */
#define RLL_MAX_SPRITE_TYPES 512
SpriteId nextSpriteId;
GLuint spriteTextures[RLL_MAX_SPRITE_TYPES];

#define MAX_INFO_LOG_LENGTH 4096

/**
 * Logging ID for renderer logging.
 */
uint32_t LogSysRender;

#define RLL_MAX_ATTRIBUTES 8

KN_STATIC_ASSERT(RLL_MAX_ATTRIBUTES <= GL_MAX_VERTEX_ATTRIBS, "RLL supports more"
	"active attributes than the API allows");
#define RLL_MAX_UNIFORMS 32
#define RLL_MAX_PROGRAMS 16
#define RLL_MAX_ATTRIBUTE_NAME_LENGTH 64
#define RLL_MAX_UNIFORM_NAME_LENGTH 64

typedef union {
	int i;
	float2 f2;
	float4 f4;
	float4x4 f44;
} AnyGLValue;

typedef struct {
	char name[RLL_MAX_ATTRIBUTE_NAME_LENGTH];

	/** Corresponding semantic type to use at this index. */
	uint32_t semanticName;
	GLint location;
	GLint size;

	/**
	 * Not necessarily the actual type to use for the vertex pointer.  Note that
	 * the type returned by glGetActiveAttrib IS NOT the same type as used by
	 * glVertexAttribPointer
	 */
	GLenum type;
	GLboolean normalized;
} Attribute;

typedef struct {
	char name[RLL_MAX_UNIFORM_NAME_LENGTH];
	GLuint location;
	uint32_t semanticName;
	GLint size;
	GLenum type;
} Uniform;

typedef struct {
	GLuint id;
	Attribute attributes[RLL_MAX_ATTRIBUTES];
	Uniform uniforms[RLL_MAX_UNIFORMS];
	uint32_t numAttributes;
	uint32_t numUniforms;
} Program;

static Program programs[RLL_MAX_PROGRAMS];

enum {
	ProgramIndexSprite = 0,
	ProgramIndexFullScreen = 1,
	ProgramIndexSolidPolygon = 2
};

enum {
	AttributeSemanticNamePosition = 0,
	AttributeSemanticNamePosition2 = 0,
	AttributeSemanticNamePosition3 = 0,
	AttributeSemanticNamePosition4 = 0,
	AttributeSemanticNameTexCoord2 = 1,
	AttributeSemanticNameTypes = 5,
	AttributeSemanticNameUnknown
};

enum {
	UniformSemanticNameProjection = 0,
	UniformSemanticNameModelView = 1,
	UniformSemanticNameViewModel = 1,
	UniformSemanticNameTexture = 2,
	UniformSemanticNameTexture2D0 = 2,
	UniformSemanticNamePolygonColor = 3,
	UniformSemanticNameTypes = 6,
	UniformSemanticNameUnknown
};

AnyGLValue uniformSemanticStorage[UniformSemanticNameTypes];

typedef struct {
	const char* str;
	uint32_t id; // TODO: Rename "location"
	GLenum type;
	GLint size;
} SemanticName;

static SemanticName attributeSemanticNames[] = {
	{ "Position", AttributeSemanticNamePosition, GL_FLOAT, 4 },
	{ "Position2", AttributeSemanticNamePosition2, GL_FLOAT, 2 },
	{ "Position3", AttributeSemanticNamePosition3, GL_FLOAT, 3 },
	{ "Position4", AttributeSemanticNamePosition4, GL_FLOAT, 4 },
	{ "TexCoord2", AttributeSemanticNameTexCoord2, GL_FLOAT, 2 }
};

KN_STATIC_ASSERT(AttributeSemanticNameTypes == sizeof(attributeSemanticNames) / sizeof(attributeSemanticNames[0]),
	"Number of attribute semantic names doesn't match data array");

// TODO: Naming misnomer, uniform->semanticName doesn't map into this array,
// it maps into the uniform storage.
static SemanticName uniformSemanticNames[] = {
	{ "Projection", UniformSemanticNameProjection, GL_FLOAT_MAT4, 1 },
	{ "ModelView", UniformSemanticNameModelView, GL_FLOAT_MAT4, 1 },
	{ "ViewModel", UniformSemanticNameViewModel, GL_FLOAT_MAT4, 1 },
	{ "Texture", UniformSemanticNameTexture, GL_SAMPLER_2D, 1 },
	{ "Texture2D0", UniformSemanticNameTexture2D0, GL_SAMPLER_2D, 1 },
	{ "PolygonColor", UniformSemanticNamePolygonColor, GL_FLOAT_VEC4, 1 }
};

KN_STATIC_ASSERT(UniformSemanticNameTypes == sizeof(uniformSemanticNames) / sizeof(uniformSemanticNames[0]),
				 "Number of uniform semantic names doesn't match data array");

uint32_t lookupAttributeSemanticName(const char* name)
{
	for (uint32_t i=0; i < AttributeSemanticNameTypes; ++i) {
		if (strcmp(attributeSemanticNames[i].str, name) == 0) {
			return i;
		}
	}
	return AttributeSemanticNameUnknown;
}

uint32_t lookupUniformSemanticName(const char* name)
{
	for (uint32_t i=0; i < UniformSemanticNameTypes; ++i) {
		if (strcmp(uniformSemanticNames[i].str, name) == 0) {
			// TODO: bad mapping here?
			return uniformSemanticNames[i].id;
			//return i;
		}
	}

	for (uint32_t i=0; i < UniformSemanticNameTypes; ++i) {
		KN_TRACE(LogSysRender, "uniform: %s", uniformSemanticNames[i].str);
	}
	KN_TRACE(LogSysRender, "Unable to find %s", name);
	return UniformSemanticNameUnknown;
}

void RLL_ReadyTexture2(GLuint index, GLuint texture)
{
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, texture);
}

void RLL_ReadyUniform(Uniform* u)
{
	switch(u->type) {
		case GL_FLOAT_VEC2:
			KN_ASSERT(u->size == 1, "Arrays of float2 are not supported");
			glUniform2fv(u->location, 1, uniformSemanticStorage[u->semanticName].f2.v);
			break;
		case GL_FLOAT_VEC4:
			KN_ASSERT(u->size == 1, "Arrays of float3 are not supported");
			glUniform4fv(u->location, 1, uniformSemanticStorage[u->semanticName].f4.v);
			break;
		case GL_FLOAT_MAT4:
			KN_ASSERT(u->size == 1, "Arrays of float4x4 are not supported");
			glUniformMatrix4fv(u->location, 1, GL_FALSE,
				&uniformSemanticStorage[u->semanticName].f44.m[0][0]);
			break;
		case GL_SAMPLER_2D:
			glUniform1i(u->location, uniformSemanticStorage[u->semanticName].i);
			break;
		default:
			KN_FATAL_ERROR("Unknown uniform type: %i", u->type);
	}
	KN_ASSERT_NO_GL_ERROR();
/*
	GL_FLOAT
	GL_FLOAT_VEC3
	GL_FLOAT_MAT2
	GL_FLOAT_MAT3
	GL_FLOAT_MAT2x3
	GL_FLOAT_MAT2x4
	GL_FLOAT_MAT3x2
	GL_FLOAT_MAT3x4
	GL_FLOAT_MAT4x2
	GL_FLOAT_MAT4x3
	GL_INT
	GL_INT_VEC2
	GL_INT_VEC3
	GL_INT_VEC4
	GL_UNSIGNED_INT
	GL_UNSIGNED_INT_VEC2
	GL_UNSIGNED_INT_VEC3
	GL_UNSIGNED_INT_VEC4
	GL_SAMPLER
	GL_SAMPLER_1D
	GL_SAMPLER_3D
	GL_SAMPLER_1D_ARRAY
	GL_SAMPLER_2D_ARRAY
	GL_SAMPLER_1D_SHADOW
	GL_SAMPLER_2D_SHADOW
*/
}

/**
 * Registers a program to the given index.
 *
 * Pulls out a list of attributes to use.
 *
 * Also pulls out the list of uniforms to apply.
 */
void RLL_RegisterProgram(uint32_t index, GLuint program)
{
	KN_ASSERT_NO_GL_ERROR();
	KN_ASSERT(index <= RLL_MAX_PROGRAMS, "Trying to register a program %" PRIu32
		"outside of the valid range of programs %" PRIu32, index, program);
	KN_TRACE(LogSysRender, "Registering: %u to %" PRIu32, program, index);
	Program* p = &programs[index];

	p->id = program;
	GLint numActiveAttributes;
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numActiveAttributes);
	KN_TRACE(LogSysRender, "Active Attributes: %d", numActiveAttributes);
	KN_ASSERT_NO_GL_ERROR();
	for (GLint i = 0; i < numActiveAttributes; ++i) {
		GLint size;
		GLenum type;
		glGetActiveAttrib(program, (GLuint)i, RLL_MAX_ATTRIBUTE_NAME_LENGTH,
			NULL, &size, &type, p->attributes[i].name);
		KN_TRACE(LogSysRender, "[%d]: %s '%s'   %d", i, RLL_GLTypeToString(type),
			p->attributes[i].name, size);
		
		uint32_t semanticName = lookupAttributeSemanticName(p->attributes[i].name);
		KN_ASSERT(semanticName < AttributeSemanticNameTypes, "Couldn't find attribute "
			"semantic name for %s", p->attributes[i].name);
		p->attributes[i].location = i;
		p->attributes[i].semanticName = semanticName;
		p->attributes[i].type = type;
		p->attributes[i].size = size;
		p->attributes[i].normalized = GL_FALSE; // TODO: make this configurable.

		KN_ASSERT_NO_GL_ERROR();
	}
	p->numAttributes = numActiveAttributes;

	GLint numActiveUniforms;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numActiveUniforms);
	KN_TRACE(LogSysRender, "Active Uniforms: %d", numActiveUniforms);
	for (GLint i = 0; i < numActiveUniforms; ++i) {
		GLint size;
		GLenum type;
		glGetActiveUniform(program, (GLuint)i, RLL_MAX_UNIFORM_NAME_LENGTH,
			NULL, &size, &type, p->uniforms[i].name);
		KN_TRACE(LogSysRender, "[%d]: %s '%s'   %d", i, RLL_GLTypeToString(type),
			p->uniforms[i].name, size);

		uint32_t semanticName = lookupUniformSemanticName(p->uniforms[i].name);
		KN_ASSERT(semanticName < UniformSemanticNameTypes, "Couldn't find uniform "
			"semantic name for %s", p->uniforms[i].name);
		p->uniforms[i].size = size;
		p->uniforms[i].type = type;
		p->uniforms[i].location = i;
		p->uniforms[i].semanticName = semanticName;
	}
	p->numUniforms = numActiveUniforms;

	KN_ASSERT_NO_GL_ERROR();
}

/**
 * Set uniforms according to global uniform storage, and enable attribute
 * pointers.
 */
void RLL_EnableProgram(uint32_t id, GLsizei vertexStride, void* vertexPointer)
{
	Program* p = &programs[id];
	KN_ASSERT(glIsProgram(p->id), "%" PRIu32 " is not a valid program.", id);
	glUseProgram(p->id);
	KN_ASSERT_NO_GL_ERROR();

	for (uint32_t i = 0; i < p->numAttributes; ++i) {
		GLint size = 0;
		GLenum type;

		switch (p->attributes[i].type) {
			case GL_FLOAT_VEC4:
				size = 4;
				type = GL_FLOAT;
				break;
			default:
				KN_FATAL_ERROR("Unknown attribute type: %s", RLL_GLTypeToString(p->attributes[i].type));
		}

		glEnableVertexAttribArray(p->attributes[i].location);
		KN_ASSERT_NO_GL_ERROR();
		glVertexAttribPointer(
			p->attributes[i].location,
			size,
			type,
			p->attributes[i].normalized,
			vertexStride,
			vertexPointer
		);
		KN_ASSERT_NO_GL_ERROR();
	}

	for (uint32_t i = 0; i < p->numUniforms; ++i) {
		RLL_ReadyUniform(&p->uniforms[i]);
	}
}

void RLL_DisableProgram(uint32_t id)
{
	Program* p = &programs[id];
	for (uint32_t i = 0; i < p->numAttributes; ++i) {
		glDisableVertexAttribArray(p->attributes[i].location);
	}
}

bool RLL_CreateProgram(GLuint vertexShader, GLuint fragmentShader, GLuint* program,
	uint32_t programIndex);
void RLL_FillBuffers(void);
void RLL_InitSprites(void);
void RLL_LoadShaders(void);

void RLL_DumpState()
{
	KN_ASSERT_NO_GL_ERROR();
	int i;
	KN_UNUSED(i);
#define DebugI(name) glGetIntegerv(name, &i);  KN_TRACE(LogSysRender, "%s: %i", #name, i);
	//DebugI(GL_ARRAY_BUFFER_BINDING);
	//DebugI(GL_CURRENT_PROGRAM);
	//DebugI(GL_ACTIVE_TEXTURE);
#undef DebugI
	KN_ASSERT_NO_GL_ERROR();
}

#if KN_DEBUG
void RLL_CheckGLError(const char* file, int line)
{
	const GLenum glError = glGetError();
	switch (glError)
	{
		case GL_NO_ERROR:
			return;
#define label_print(label) case label: KN_ERROR(LogSysRender, "OpenGL Error: %s:%d " #label, file, line); break;
		label_print(GL_INVALID_ENUM)
		label_print(GL_INVALID_VALUE)
		label_print(GL_INVALID_OPERATION)
		label_print(GL_INVALID_FRAMEBUFFER_OPERATION)
		label_print(GL_OUT_OF_MEMORY)
		label_print(GL_STACK_UNDERFLOW)
		label_print(GL_STACK_OVERFLOW)
#undef label_print
		default:
			KN_ERROR(LogSysRender, "Unknown error: %s:%d %d", file, line, glError);
	}
	KN_DEBUG_BREAK();
}

/**
 * Converts `GLenum` to a string for debugging.
 */
const char* RLL_GLTypeToString(GLenum type)
{
#define strType(t) case t: return #t
	switch (type) {
		strType(GL_FLOAT);
		strType(GL_FLOAT_VEC2);
		strType(GL_FLOAT_VEC3);
		strType(GL_FLOAT_VEC4);
		strType(GL_FLOAT_MAT2);
		strType(GL_FLOAT_MAT3);
		strType(GL_FLOAT_MAT4);
		strType(GL_FLOAT_MAT2x3);
		strType(GL_FLOAT_MAT2x4);
		strType(GL_FLOAT_MAT3x2);
		strType(GL_FLOAT_MAT3x4);
		strType(GL_FLOAT_MAT4x2);
		strType(GL_FLOAT_MAT4x3);
		strType(GL_INT);
		strType(GL_INT_VEC2);
		strType(GL_INT_VEC3);
		strType(GL_INT_VEC4);
		strType(GL_UNSIGNED_INT);
		strType(GL_UNSIGNED_INT_VEC2);
		strType(GL_UNSIGNED_INT_VEC3);
		strType(GL_UNSIGNED_INT_VEC4);
		strType(GL_SAMPLER);
		strType(GL_SAMPLER_1D);
		strType(GL_SAMPLER_2D);
		strType(GL_SAMPLER_3D);
		strType(GL_SAMPLER_1D_ARRAY);
		strType(GL_SAMPLER_2D_ARRAY);
		strType(GL_SAMPLER_1D_SHADOW);
		strType(GL_SAMPLER_2D_SHADOW);
		default: return "Unknown type";
	}
#undef strType
}

/**
 * Print a shader program and all of its active attributes and uniforms.
 * Failing to use attributes results in them not being active.  This function
 * is used before dropping into renderdoc or heavier tools for monitoring the
 * shader system while it is being developed.
 */
void RLL_PrintProgram(GLuint program)
{
	KN_ASSERT(glIsProgram(program), "Program is not a program");

	enum { bufferSize = 1024 };
	GLchar name[bufferSize];

	// Print attributes.
	GLint numActiveAttributes;
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numActiveAttributes);
	KN_TRACE(LogSysRender, "Active Attributes: %d", numActiveAttributes);
	for (GLint i = 0; i < numActiveAttributes; ++i) {
		GLint size;
		GLenum type;
		glGetActiveAttrib(program, (GLuint)i, bufferSize, NULL, &size, &type, name);
		KN_TRACE(LogSysRender, "[%d]: %s '%s'   %d", i, RLL_GLTypeToString(type), name, size);
	}

	GLint numActiveUniforms;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numActiveUniforms);
	KN_TRACE(LogSysRender, "Active Uniforms: %d", numActiveUniforms);
	for (GLint i = 0; i < numActiveUniforms; ++i) {
		GLint size;
		GLenum type;
		glGetActiveUniform(program, (GLuint)i, bufferSize, NULL, &size, &type, name);
		KN_TRACE(LogSysRender, "[%d]: %s '%s'   %d", i, RLL_GLTypeToString(type), name, size);
	}

	// Print validation status.
	glValidateProgram(program);
	GLint validateStatus;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &validateStatus);
	KN_TRACE(LogSysRender, "Validate status: %d", validateStatus);

	// Print the info log.
	GLint infoLogLength;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		if (MAX_INFO_LOG_LENGTH < infoLogLength) {
			KN_TRACE(LogSysRender, "Info log is too small to hold all output");
		}
		char infoLog[MAX_INFO_LOG_LENGTH];
		glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);
		KN_TRACE(LogSysRender, "Validation log: %s\n", infoLog);
	}
}

/**
 * The renderer may have requested a specific version of OpenGL, but this
 * program provides the current version being used.
 */
void RLL_PrintGLVersion(void)
{
	// TODO: set the opengl context?
	KN_ASSERT_NO_GL_ERROR();
	GLint majorVersion, minorVersion;
	glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
	KN_TRACE(LogSysRender, "Using OpenGL %i.%i", majorVersion, minorVersion);
	KN_TRACE(LogSysRender, "    Vendor: %s", glGetString(GL_VENDOR));
	KN_TRACE(LogSysRender, "    Renderer: %s", glGetString(GL_RENDERER));
	KN_TRACE(LogSysRender, "    Version: %s", glGetString(GL_VERSION));
	KN_TRACE(LogSysRender, "    GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	KN_ASSERT_NO_GL_ERROR();
}

#endif /* KN_DEBUG */

/**
 * An openGL orthographic matrix.  Note that DirectX will need a related, but
 * slightly different version.
 *
 * Orthographic projection matrix to match the same dimensions as the window
 * size.  Scaling based on the same aspect ratio but big enough to see might be
 * a better option.
 */
static float4x4 RLL_OrthoProjection(const uint32_t width, const uint32_t height)
{
	const float far = -100;
	const float near = 100;
	const float w = (float)width;
	const float h = (float)height;
	const float4x4 scale = float4x4_NonUniformScale(2.0f / w, 2.0f / h, 2.0f / (far - near));
	const float4x4 trans = float4x4_Translate(-w / 2.0f, -h / 2.0f, -(far + near) / 2.0f);
	//return float4x4_Multiply(scale, trans);
	return float4x4_Multiply(trans, scale);
}

static bool RLL_CreateShader(GLuint* shader, const char* source, const uint32_t sourceLength)
{
	const GLchar* sources[] = { source };
	const GLint sizes[] = { sourceLength };
	glShaderSource(*shader, 1, sources, sizes);
	glCompileShader(*shader);

	GLint compileResult = GL_FALSE;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &compileResult);
	if (!compileResult) {
		KN_ERROR(LogSysRender, "Unable to compile shader: %s", source);
		return false;
	}

	GLint infoLogLength;
	glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		if (MAX_INFO_LOG_LENGTH < infoLogLength) {
			KN_ERROR(LogSysRender, "Info log buffer is too small to hold all output");
			return false;
		}
		char infoLog[MAX_INFO_LOG_LENGTH];
		glGetShaderInfoLog(*shader, MAX_INFO_LOG_LENGTH, NULL, infoLog);
		KN_TRACE(LogSysRender, "Compilation results for %s", infoLog);
	}
	return true;
}

void RLL_InitGL(void)
{
	Log_RegisterSystem(&LogSysRender, "Render", KN_LOG_TRACE);

	// Get up and running quickly with OpenGL 3.1 with old-school functions.
	// TODO: Replace with Core profile once something is working.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	gl = SDL_GL_CreateContext(window);
	if (gl == NULL) {
		KN_FATAL_ERROR("Unable to create OpenGL context: %s", SDL_GetError());
	}

#ifdef _WIN32
	// Use GLEW on Windows to load function pointers to advanced OpenGL functions.
	glewInit();
#endif

	KN_TRACE(LogSysRender, "OpenGL renderer initialized");
}

void RLL_ConfigureVSync(void)
{
	// VSync causes the draw to stall until the frame is displayed.
	// This might be more useful if/when drawing gets moved to it's own thread.
	const bool useVSync = true;
	if (useVSync) {
		SDL_GL_SetSwapInterval(1);
	}
	else {
		SDL_GL_SetSwapInterval(0);
	}
}

/**
 * Even if VAOs aren't used, a dummy VAO must be bound.
 */
void RLL_InitDummyVAO(void)
{
	static GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
    KN_ASSERT_NO_GL_ERROR();
}

void RLL_FillFullScreenQuadBuffer(void)
{
	// OpenGL ndc is a cube from -1 to 1
	float4 vertices[] = {
		float4_Make(-1.0f, -1.0f, 0.0f, 0.0f),
		float4_Make(-1.0f, 1.0f, 0.0f, 0.0f),
		float4_Make(1.0f, -1.0f, 0.0f, 0.0f),
		float4_Make(1.0f, 1.0f, 0.0f, 0.0f)
	};
	glGenBuffers(1, &fullScreenQuadBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, fullScreenQuadBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	KN_ASSERT(fullScreenQuadBuffer, "Cannot allocate a buffer for the full screen quad");
	KN_ASSERT_NO_GL_ERROR();
}

void RLL_FillSpriteBuffer(void)
{
	float4 vertices[4];
	vertices[0] = float4_Make(0.0f, 0.0f, 0.0f, 0.0f);
	vertices[1] = float4_Make(0.0f, 1.0f, 0.0f, 0.0f);
	vertices[2] = float4_Make(1.0f, 0.0f, 0.0f, 0.0f);
	vertices[3] = float4_Make(1.0f, 1.0f, 0.0f, 0.0f);

	glGenBuffers(1, &spriteBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, spriteBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	KN_ASSERT(spriteBuffer, "Cannot allocate a buffer for the sprite buffer");
	KN_ASSERT(glIsBuffer(spriteBuffer), "Could not create sprite buffer");
	KN_ASSERT_NO_GL_ERROR();
	KN_ASSERT_NO_GL_ERROR();
}

void RLL_FillDebugQuadBuffer(void)
{
	// Doesn't matter, will be overwritten.
	float4 vertices[RLL_MAX_DEBUG_POINTS];
	memset(&vertices[0], 0, RLL_MAX_DEBUG_POINTS * sizeof(float4));
	glGenBuffers(1, &debugDrawBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, debugDrawBuffer);

	// Allocate the maximum used spaced and then override it.
	glBufferData(GL_ARRAY_BUFFER, RLL_MAX_DEBUG_POINTS * 4 * sizeof(float), vertices, GL_DYNAMIC_DRAW);

	KN_ASSERT(debugDrawBuffer, "Cannot allocate a buffer for the debug drawing");
	KN_ASSERT_NO_GL_ERROR();
}

void RLL_FillBuffers(void)
{
	RLL_FillSpriteBuffer();
	RLL_FillFullScreenQuadBuffer();
	RLL_FillDebugQuadBuffer();
}

void RLL_InitSprites(void)
{
	nextSpriteId = 0;
}

void RLL_LoadFullScreenDebugShader(void)
{
	const int maxShaderTextLength = 1024;
    PathBuffer fragmentShaderPath;
    PathBuffer vertexShaderPath;
	DynamicBuffer fragmentShaderBuffer;
	DynamicBuffer vertexShaderBuffer;

	// Read fragment shader
	if (Assets_PathBufferFor("shaders/uv_as_red_green.frag", &fragmentShaderPath)) {
		if (SPA_IsFile(fragmentShaderPath.str)) {
			KN_TRACE(LogSysRender, "%s found", fragmentShaderPath.str);
		}
		else {
			KN_TRACE(LogSysRender, "%s not found", fragmentShaderPath.str);
		}
	}

	if (!Assets_ReadFile(fragmentShaderPath.str, KN_FILE_TYPE_TEXT, &fragmentShaderBuffer)) {
		KN_ERROR(LogSysRender, "Unable to read fragment shader text");
	}

	// Read vertex shader
	if (Assets_PathBufferFor("shaders/fullscreen_textured_quad.vert", &vertexShaderPath)) {
		if (SPA_IsFile(vertexShaderPath.str)) {
			KN_TRACE(LogSysRender, "%s found", vertexShaderPath.str);
		}
		else {
			KN_TRACE(LogSysRender, "%s not found", vertexShaderPath.str);
		}
	}

	if (!Assets_ReadFile(vertexShaderPath.str, KN_FILE_TYPE_TEXT, &vertexShaderBuffer)) {
		KN_ERROR(LogSysRender, "Unable to read vertex shader text");
	}

	//KN_TRACE(LogSysRender, "Fragment shader %s", fragmentShaderBuffer.contents);
	//KN_TRACE(LogSysRender, "Vertex shader %s", vertexShaderBuffer.contents);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	if (!glIsShader(vertexShader)) {
		KN_ERROR(LogSysRender, "Unable to allocate space for vertex shader");
	}

	if (!glIsShader(fragmentShader)) {
		KN_ERROR(LogSysRender, "Unable to allocate space for fragment shader");
	}

	RLL_CreateShader(&fragmentShader, fragmentShaderBuffer.contents, fragmentShaderBuffer.size);
	RLL_CreateShader(&vertexShader, vertexShaderBuffer.contents, vertexShaderBuffer.size);

	RLL_CreateProgram(vertexShader, fragmentShader, &fullScreenDebugProgram, ProgramIndexFullScreen);
	Mem_Free(&vertexShaderBuffer);
	Mem_Free(&fragmentShaderBuffer);
}

void RLL_LoadSolidPolygonShader(void)
{
	const int maxShaderTextLength = 1024;
    PathBuffer fragmentShaderPath;
    PathBuffer vertexShaderPath;
	DynamicBuffer fragmentShaderBuffer;
	DynamicBuffer vertexShaderBuffer;

	// Read fragment shader
	if (Assets_PathBufferFor("shaders/solid_polygon.frag", &fragmentShaderPath)) {
		if (SPA_IsFile(fragmentShaderPath.str)) {
			KN_TRACE(LogSysRender, "%s found", fragmentShaderPath.str);
		}
		else {
			KN_TRACE(LogSysRender, "%s not found", fragmentShaderPath.str);
		}
	}

	if (!Assets_ReadFile(fragmentShaderPath.str, KN_FILE_TYPE_TEXT, &fragmentShaderBuffer)) {
		KN_ERROR(LogSysRender, "Unable to read fragment shader text");
	}

	// Read vertex shader
	if (Assets_PathBufferFor("shaders/solid_polygon.vert", &vertexShaderPath)) {
		if (SPA_IsFile(vertexShaderPath.str)) {
			KN_TRACE(LogSysRender, "%s found", vertexShaderPath.str);
		}
		else {
			KN_TRACE(LogSysRender, "%s not found", vertexShaderPath.str);
		}
	}

	if (!Assets_ReadFile(vertexShaderPath.str, KN_FILE_TYPE_TEXT, &vertexShaderBuffer)) {
		KN_ERROR(LogSysRender, "Unable to read vertex shader text");
	}

	//KN_TRACE(LogSysRender, "Fragment shader %s", fragmentShaderBuffer.contents);
	//KN_TRACE(LogSysRender, "Vertex shader %s", vertexShaderBuffer.contents);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	if (!glIsShader(vertexShader)) {
		KN_ERROR(LogSysRender, "Unable to allocate space for vertex shader");
	}

	if (!glIsShader(fragmentShader)) {
		KN_ERROR(LogSysRender, "Unable to allocate space for fragment shader");
	}

	RLL_CreateShader(&fragmentShader, fragmentShaderBuffer.contents, fragmentShaderBuffer.size);
	RLL_CreateShader(&vertexShader, vertexShaderBuffer.contents, vertexShaderBuffer.size);

	RLL_CreateProgram(vertexShader, fragmentShader, &solidPolygonProgram, ProgramIndexSolidPolygon);
	Mem_Free(&vertexShaderBuffer);
	Mem_Free(&fragmentShaderBuffer);
}

void RLL_LoadSpriteShader(void)
{
	PathBuffer fragmentShaderPath;
	PathBuffer vertexShaderPath;
	DynamicBuffer fragmentShaderBuffer;
	DynamicBuffer vertexShaderBuffer;

	KN_TRACE(LogSysRender, "Loading sprite shader");

	// Read fragment shader
	if (Assets_PathBufferFor("shaders/sprite.frag", &fragmentShaderPath)) {
		if (SPA_IsFile(fragmentShaderPath.str)) {
			KN_TRACE(LogSysRender, "%s found", fragmentShaderPath.str);
		}
		else {
			KN_TRACE(LogSysRender, "%s not found", fragmentShaderPath.str);
		}
	}

	if (!Assets_ReadFile(fragmentShaderPath.str, KN_FILE_TYPE_TEXT, &fragmentShaderBuffer)) {
		KN_ERROR(LogSysRender, "Unable to read fragment shader text");
	}

	// Read vertex shader
	if (Assets_PathBufferFor("shaders/sprite.vert", &vertexShaderPath)) {
		if (SPA_IsFile(vertexShaderPath.str)) {
			KN_TRACE(LogSysRender, "%s found", vertexShaderPath.str);
		}
		else {
			KN_TRACE(LogSysRender, "%s not found", vertexShaderPath.str);
		}
	}

	if (!Assets_ReadFile(vertexShaderPath.str, KN_FILE_TYPE_TEXT, &vertexShaderBuffer)) {
		KN_ERROR(LogSysRender, "Unable to read vertex shader text");
	}

	//KN_TRACE(LogSysRender, "Fragment shader %s", fragmentShaderBuffer.contents);
	//KN_TRACE(LogSysRender, "Vertex shader %s", vertexShaderBuffer.contents);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	if (!glIsShader(vertexShader)) {
		KN_ERROR(LogSysRender, "Unable to allocate space for vertex shader");
	}

	if (!glIsShader(fragmentShader)) {
		KN_ERROR(LogSysRender, "Unable to allocate space for fragment shader");
	}

	RLL_CreateShader(&fragmentShader, fragmentShaderBuffer.contents, fragmentShaderBuffer.size);
	RLL_CreateShader(&vertexShader, vertexShaderBuffer.contents, vertexShaderBuffer.size);

	RLL_CreateProgram(vertexShader, fragmentShader, &spriteProgram, ProgramIndexSprite);
	Mem_Free(&vertexShaderBuffer);
	Mem_Free(&fragmentShaderBuffer);

	KN_TRACE(LogSysRender, "Done loading sprite shader");
}

void RLL_LoadShaders(void)
{
	RLL_LoadSolidPolygonShader();
	RLL_LoadFullScreenDebugShader();
	RLL_LoadSpriteShader();
}

bool RLL_CreateProgram(GLuint vertexShader, GLuint fragmentShader, GLuint* program,
	uint32_t programIndex)
{
	KN_ASSERT_NO_GL_ERROR();

	if (!program) {
		KN_ERROR(LogSysRender, "program is a NULL ptr");
		return false;
	}

	if (!glIsShader(vertexShader)) {
		KN_ERROR(LogSysRender, "Vertex shader doesn't exist");
		return false;
	}

	if (!glIsShader(fragmentShader)) {
		KN_ERROR(LogSysRender, "Fragment shader doesn't exist");
	}

	*program = glCreateProgram();
	glAttachShader(*program, vertexShader);
	glAttachShader(*program, fragmentShader);
	glLinkProgram(*program);

	GLint linkResult;
	glGetProgramiv(*program, GL_LINK_STATUS, &linkResult);

	GLint infoLogLength;
	glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		if (MAX_INFO_LOG_LENGTH < infoLogLength) {
			KN_ERROR(LogSysRender, "Info log is too small to hold all output");
		}
		char infoLog[MAX_INFO_LOG_LENGTH];
		glGetProgramInfoLog(*program, infoLogLength, NULL, infoLog);
		KN_TRACE(LogSysRender, "Link log: %s\n", infoLog);
	}

	if (linkResult == GL_FALSE) {
		KN_ERROR(LogSysRender, "Unable to link program");
		return false;
	}

	glDetachShader(*program, vertexShader);
	glDetachShader(*program, fragmentShader);

#if KN_DEBUG
	RLL_PrintProgram(*program);
#endif

	KN_ASSERT_NO_GL_ERROR();

	if (linkResult == GL_TRUE) {
		RLL_RegisterProgram(programIndex, *program);
	}
	KN_ASSERT_NO_GL_ERROR();

	return linkResult == GL_TRUE;
}

void RLL_Init(uint32_t width, uint32_t height)
{
	RLL_InitGL();
	RLL_ConfigureVSync();
	RLL_InitDummyVAO();
	RLL_FillBuffers();
	RLL_InitSprites();
	RLL_LoadShaders();

	windowWidth = (GLsizei)width;
	windowHeight = (GLsizei)height;
	uniformSemanticStorage[UniformSemanticNameProjection].f44 = RLL_OrthoProjection(width, height);
}

void RLL_StartFrame(void)
{
	SDL_GL_MakeCurrent(window, gl);
}

void RLL_EndFrame(void)
{
	SDL_GL_SwapWindow(window);
}

void RLL_Clear(rgba8i color)
{
	glClearColor(color.red, color.green, color.blue, color.alpha);
	glClear(GL_COLOR_BUFFER_BIT);
}

void RLL_SetFullScreenViewport(void)
{
	glViewport(0, 0, windowWidth, windowHeight);
}

bool RLL_CreateSprite(SpriteId* id)
{
	KN_ASSERT(id != NULL, "Cannot assign a sprite to a null id.");
	*id = ++nextSpriteId;
	return true;
}

bool RLL_LoadSprite(SpriteId id, const char* path)
{
	ImagePixels image;
	if (!Image_Allocate(&image, path)) {
		return false;
	}

	glGenTextures(1, &spriteTextures[id]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, spriteTextures[id]);

	// Don't mipmap for now.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    KN_ASSERT_NO_GL_ERROR();

	// TODO: Use proxy textures to test to see if sufficient space exists.
	// TODO: Should this be GL_RGBA8?
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width, image.height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, image.pixels.contents);

    KN_ASSERT_NO_GL_ERROR();

	// Set the texture parameters.
	// https://stackoverflow.com/questions/3643932/what-is-the-scope-of-gltexparameters-in-opengl
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    KN_ASSERT_NO_GL_ERROR();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    KN_ASSERT_NO_GL_ERROR();

	KN_ASSERT(glIsTexture(spriteTextures[id]), "Unable to reserve texture for "
		"sprite loading from path: %s", path);
    
	Image_Free(&image);

    KN_ASSERT_NO_GL_ERROR();
	return true;
}

void RLL_DrawSprite(SpriteId id, float2 position, dimension2f size)
{	KN_ASSERT_NO_GL_ERROR();

	RLL_SetFullScreenViewport();

	GLuint texture = spriteTextures[id];
	KN_ASSERT(glIsTexture(texture), "Sprite %" PRIu32 " does not have a valid"
		"texture", texture);
	RLL_ReadyTexture2(0, spriteTextures[id]);
	
	// TODO: This name is wrong, since Knell uses row-vectors, it should be ModelView.
	uniformSemanticStorage[UniformSemanticNameModelView].f44 = float4x4_Multiply(
		float4x4_NonUniformScale(size.width, size.height, 1.0f),
		float4x4_Translate(position.x, position.y, 0.0f));;

	glBindBuffer(GL_ARRAY_BUFFER, spriteBuffer);
	KN_ASSERT_NO_GL_ERROR();
	RLL_EnableProgram(ProgramIndexSprite, 4 * sizeof(float), 0);

	RLL_DumpState();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	RLL_DisableProgram(ProgramIndexSprite);

	KN_ASSERT_NO_GL_ERROR();
}

/**
 * Draw a fullscreen debug rect.
 */
void RLL_DrawDebugFullScreenRect(void)
{
	KN_ASSERT_NO_GL_ERROR();

	RLL_SetFullScreenViewport();

	glUseProgram(fullScreenDebugProgram);
	glBindBuffer(GL_ARRAY_BUFFER, fullScreenQuadBuffer);

	// set vertex arrays
	GLuint positionAttrib = glGetAttribLocation(fullScreenDebugProgram, "Position");
	glEnableVertexAttribArray(positionAttrib);
	KN_ASSERT_NO_GL_ERROR();
	glVertexAttribPointer(
		positionAttrib,
		4,
		GL_FLOAT,
		GL_FALSE,
		4 * sizeof(float),
		(void *)0
	);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(positionAttrib);

	KN_ASSERT_NO_GL_ERROR();
}

/**
 * Draws a rectangle at a given center point with known dimensions.
 */
void RLL_DrawDebugRect(float4 position, dimension2f dimensions, float4 color)
{
	RLL_SetFullScreenViewport();

	glBindBuffer(GL_ARRAY_BUFFER, debugDrawBuffer);

	RLL_EnableProgram(ProgramIndexSolidPolygon, 4 * sizeof(float), 0);

	uniformSemanticStorage[UniformSemanticNameViewModel].f44 = float4x4_Identity();
	uniformSemanticStorage[UniformSemanticNamePolygonColor].f4 = color;

	float4 vertices[4];
	vertices[0] = float4_Make(-dimensions.width / 2.0f, -dimensions.height / 2.0f, 0.0f, 1.0f);
	vertices[1] = float4_Make(dimensions.width / 2.0f, -dimensions.height / 2.0f, 0.0f, 1.0f);
	vertices[2] = float4_Make(-dimensions.width / 2.0f, dimensions.height / 2.0f, 0.0f, 1.0f);
	vertices[3] = float4_Make(dimensions.width / 2.0f, dimensions.height / 2.0f, 0.0f, 1.0f);

	for (uint32_t i = 0; i < 4; ++i) {
		vertices[i].x += position.x;
		vertices[i].y += position.y;
		vertices[i].z += position.z;
	}

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	RLL_DisableProgram(ProgramIndexSolidPolygon);

	KN_ASSERT_NO_GL_ERROR();
}

void RLL_DrawDebugLine(float x1, float y1, float x2, float y2, rgb8 color)
{
	// TODO: Probably shouldn't reset viewport.
	RLL_SetFullScreenViewport();

	glBindBuffer(GL_ARRAY_BUFFER, debugDrawBuffer);

	uniformSemanticStorage[UniformSemanticNameViewModel].f44 = float4x4_Identity();
	uniformSemanticStorage[UniformSemanticNamePolygonColor].f4 = float4_Make(
		(float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f, 1.0f);

	RLL_EnableProgram(ProgramIndexSolidPolygon, 4 * sizeof(float), 0);

	float4 vertices[2];
	vertices[0] = float4_Make(x1, y1, 0.0f, 1.0f);
	vertices[1] = float4_Make(x2, y2, 0.0f, 1.0f);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glDrawArrays(GL_LINES, 0, 2);

	RLL_DisableProgram(ProgramIndexSolidPolygon);
	KN_ASSERT_NO_GL_ERROR();
}

void RLL_DrawDebugLineStrip(float2* points, uint32_t numPoints, rgb8 color)
{
	KN_ASSERT(numPoints < RLL_MAX_DEBUG_POINTS, "Exceeded number of debug points "
		"to draw: %" PRIu32 " (%" PRIu32 " max)", numPoints, RLL_MAX_DEBUG_POINTS);

	// TODO: Probably shouldn't reset viewport.
	RLL_SetFullScreenViewport();

	glBindBuffer(GL_ARRAY_BUFFER, debugDrawBuffer);
	RLL_EnableProgram(ProgramIndexSolidPolygon, 4 * sizeof(float), 0);

	uniformSemanticStorage[UniformSemanticNameViewModel].f44 = float4x4_Identity();
	uniformSemanticStorage[UniformSemanticNamePolygonColor].f4 = float4_Make(
		(float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f, 1.0f);

	float4 vertices[RLL_MAX_DEBUG_POINTS];
	for (uint32_t i = 0; i < numPoints; ++i) {
		vertices[i] = float4_Make(points[i].x, points[i].y, 0.0f, 1.0f);
	}
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float4) * numPoints, vertices);
	glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)numPoints);

	RLL_DisableProgram(ProgramIndexSolidPolygon);

	KN_ASSERT_NO_GL_ERROR();
}
