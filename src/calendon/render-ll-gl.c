/*
 * Single-threaded, single context, OpenGL rendering backend.
 *
 * The renderer works by analyzing the shader programs loaded and caching the
 * vertex attributes and uniforms to apply.  Uniforms get set from a bulk
 * storage location containing all uniforms, so non-object specific uniforms,
 * such as the camera transform can be placed into a common location to be
 * found for all shaders.
*/
#include <calendon/render-ll-gl.h>

#include <calendon/cn.h>

#include <calendon/assets.h>
#include <calendon/assets-fileio.h>
#include <calendon/color.h>
#include <calendon/compat-gl.h>
#include <calendon/compat-sdl.h>
#include <calendon/font-psf2.h>
#include <calendon/image.h>
#include <calendon/log.h>
#include <calendon/math4.h>
#include <calendon/memory.h>
#include <calendon/path.h>
#include <calendon/render-ll.h>
#include <calendon/render-resources.h>

/*
 * A macro to provide OpenGL error checking and reporting.
 */
#if CN_DEBUG
	#define CN_ASSERT_NO_GL_ERROR() cnRLL_CheckGLError(__FILE__, __LINE__)
	void cnRLL_CheckGLError(const char* file, int line);
#else
	#define CN_ASSERT_NO_GL_ERROR()
#endif

const char* cnRLL_GLTypeToString(GLenum type);
void cnRLL_PrintProgram(GLuint program);
void cnRLL_PrintGLVersion(void);

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

CN_DECLARE_HANDLE_TYPE(CnSpriteId, cnRLL_, Sprite, 8);
CN_DECLARE_HANDLE_TYPE(CnFontId, cnRLL_, Font, 8);

/**
 * Maps sprite IDs to their OpenGL textures.
 */
static GLuint spriteTextures[MaxSpriteId];

static GLuint fontTextures[MaxFontId];
static CnFontPSF2 fonts[MaxFontId];

/**
 * The maximum length of shader information logs which can be read.
 */
#define MAX_INFO_LOG_LENGTH 4096

/**
 * Logging ID for renderer logging.
 */
uint32_t LogSysRender;

/**
 * Index into `vertexFormats` array, to show which global vertex format to use.
 */
enum {
	CnVertexFormatP4 = 0,
	CnVertexFormatP2 = 1,
	CnVertexFormatP2T2Interleaved = 2,
	CnVertexFormatMax
};
static CnVertexFormat vertexFormats[CnVertexFormatMax];

/**
 * Indexes into `programs` array, of which shader program to use.
 */
enum {
	CnProgramIndexSprite = 0,
	CnProgramIndexFullScreen,
	CnProgramIndexSolidPolygon,
	CnProgramIndexMax
};
static CnProgram programs[CnProgramIndexMax];

/**
 * The total number of glyphs which can be drawn at once.
 */
#define RLL_MAX_GLYPHS_PER_DRAW 180
#define RLL_VERTICES_PER_GLYPH 6
#define RLL_MAX_GLYPH_VERTICES_PER_DRAW (RLL_VERTICES_PER_GLYPH * RLL_MAX_GLYPHS_PER_DRAW)
#define RLL_GLYPH_BUFFER_SIZE (2 * 2 * sizeof(float) * RLL_MAX_GLYPH_VERTICES_PER_DRAW)
static CnFloat2 glyphVertices[RLL_MAX_GLYPH_VERTICES_PER_DRAW];
static CnFloat2 glyphTexCoords[RLL_MAX_GLYPH_VERTICES_PER_DRAW];
static uint32_t usedGlyphs = 0;
static CnVertexFormat glyphFormat;
static GLuint glyphBuffer;

/**
 * Associates a name along with an indexed location, and type information.
 */
typedef struct {
	/**
	 * Name to look for in the shader source.
	 */
	const char* str;

	/**
	 * Uniforms: CnUniform location.
	 * Attributes: CnAttribute index in the vertex format.
	 */
	uint32_t id; // TODO: Rename "location"
	GLenum type;

	/**
	 * Uniforms: array size, 1 for singular values.
	 * Attributes: number of components
	 */
	GLint size;
} CnSemanticMapping;

enum {
	CnAttributeSemanticNamePosition = 0,
	CnAttributeSemanticNamePosition2 = 0,
	CnAttributeSemanticNamePosition3 = 0,
	CnAttributeSemanticNamePosition4 = 0,
	CnAttributeSemanticNameTexCoord2 = 1,
	CnAttributeSemanticNameTypes = 5,
	CnAttributeSemanticNameUnknown
};

/**
 * Currently unused.  Being set up in preparation for applying vertex formats
 * based on a mapping of shader inputs to semantic names.
 */
static CnSemanticMapping attributeSemanticNames[] = {
	{ "Position",  CnAttributeSemanticNamePosition,  GL_FLOAT, 4 },
	{ "Position2", CnAttributeSemanticNamePosition2, GL_FLOAT, 2 },
	{ "Position3", CnAttributeSemanticNamePosition3, GL_FLOAT, 3 },
	{ "Position4", CnAttributeSemanticNamePosition4, GL_FLOAT, 4 },
	{ "TexCoord2", CnAttributeSemanticNameTexCoord2, GL_FLOAT, 2 }
};

CN_STATIC_ASSERT(CnAttributeSemanticNameTypes == CN_ARRAY_SIZE(attributeSemanticNames),
				 "Number of attribute semantic names doesn't match data array");

enum {
	CnUniformNameProjection = 0,
	CnUniformNameModelView = 1,
	CnUniformNameViewModel = 1,
	CnUniformNameTexture = 2,
	CnUniformNameTexture2D0 = 2,
	CnUniformNamePolygonColor = 3,
	CnUniformNameTypes = 6,
	CnUniformNameUnknown
};

// TODO: Naming misnomer, uniform->semanticName doesn't map into this array,
// it maps into the uniform storage.
static CnSemanticMapping UniformNames[] = {
	{ "Projection",   CnUniformNameProjection,   GL_FLOAT_MAT4, 1 },
	{ "ModelView",    CnUniformNameModelView,    GL_FLOAT_MAT4, 1 },
	{ "ViewModel",    CnUniformNameViewModel,    GL_FLOAT_MAT4, 1 },
	{ "Texture",      CnUniformNameTexture,      GL_SAMPLER_2D, 1 },
	{ "Texture2D0",   CnUniformNameTexture2D0,   GL_SAMPLER_2D, 1 },
	{ "PolygonColor", CnUniformNamePolygonColor, GL_FLOAT_VEC4, 1 }
};

CN_STATIC_ASSERT(CnUniformNameTypes == CN_ARRAY_SIZE(UniformNames),
				 "Number of uniform semantic names doesn't match data array");

/**
 * Allocates enough space to store whatever data type is needed for a uniform.
 */
typedef union {
	int i;
	CnFloat2 f2;
	CnFloat4 f4;
	CnFloat4x4 f44;
} CnAnyGLValue;
typedef CnAnyGLValue CnUniformStorage[CnUniformNameTypes];
static CnUniformStorage uniformStorage;

uint32_t cnRLL_LookupAttributeSemanticName(const char* name)
{
	CN_ASSERT(name != NULL, "Cannot lookup a null attribute name.");
	for (uint32_t i=0; i < CnAttributeSemanticNameTypes; ++i) {
		if (strcmp(attributeSemanticNames[i].str, name) == 0) {
			return attributeSemanticNames[i].id;
		}
	}
	return CnAttributeSemanticNameUnknown;
}

/**
 * Looks up the storage index for a uniform of the given name.
 */
uint32_t cnRLL_LookupUniformStorageLocation(const char* name)
{
	CN_ASSERT(name != NULL, "Cannot lookup a null uniform name.");
	for (uint32_t i=0; i < CnUniformNameTypes; ++i) {
		if (strcmp(UniformNames[i].str, name) == 0) {
			return UniformNames[i].id;
		}
	}

	for (uint32_t i=0; i < CnUniformNameTypes; ++i) {
		CN_TRACE(LogSysRender, "uniform: %s", UniformNames[i].str);
	}
	CN_TRACE(LogSysRender, "Unable to find %s", name);
	return CnUniformNameUnknown;
}

/**
 * Applies a given texture to the given texture unit.
 */
void cnRLL_ReadyTexture2(GLuint index, GLuint texture)
{
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, texture);
}

/**
 * Applies a uniform from the given uniform storage.
 */
void cnRLL_ApplyUniform(CnUniform* u, CnUniformStorage storage)
{
	CN_ASSERT(u != NULL, "Cannot apply a null uniform");
	CN_ASSERT_NO_GL_ERROR();

	switch(u->type) {
		case GL_FLOAT_VEC2:
			CN_ASSERT(u->size == 1, "Arrays of CnFloat2 are not supported");
			glUniform2fv(u->location, 1, storage[u->storageLocation].f2.v);
			break;
		case GL_FLOAT_VEC4:
			CN_ASSERT(u->size == 1, "Arrays of CnFloat4 are not supported");
			glUniform4fv(u->location, 1, storage[u->storageLocation].f4.v);
			break;
		case GL_FLOAT_MAT4:
			CN_ASSERT(u->size == 1, "Arrays of CnFloat4x4 are not supported");
			glUniformMatrix4fv(u->location, 1, GL_FALSE,
				&uniformStorage[u->storageLocation].f44.m[0][0]);
			break;
		case GL_SAMPLER_2D:
			glUniform1i(u->location, storage[u->storageLocation].i);
			break;
		default:
			CN_FATAL_ERROR("Unknown uniform type: %i", u->type);
	}
	CN_ASSERT_NO_GL_ERROR();
}

/**
 * Registers a program to the given index.
 *
 * Pulls out a list of attributes to use.
 *
 * Also pulls out the list of uniforms to apply.
 */
void cnRLL_RegisterProgram(uint32_t index, GLuint program)
{
	CN_ASSERT_NO_GL_ERROR();
	CN_ASSERT(index <= CnProgramIndexMax, "Trying to register a program %" PRIu32
		"outside of the valid range of programs %" PRIu32, index, program);
	CN_TRACE(LogSysRender, " programRegistering: %u to global program index %" PRIu32, program, index);
	CnProgram* p = &programs[index];

	p->id = program;
	GLint numActiveAttributes;
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numActiveAttributes);
	CN_TRACE(LogSysRender, "Active Attributes: %d", numActiveAttributes);
	CN_ASSERT_NO_GL_ERROR();
	for (GLint i = 0; i < numActiveAttributes; ++i) {
		GLint size;
		GLenum type;
		glGetActiveAttrib(program, (GLuint)i, CN_RLL_MAX_ATTRIBUTE_NAME_LENGTH,
			NULL, &size, &type, p->attributes[i].name);
		CN_TRACE(LogSysRender, "[%d]: %s '%s'   %d", i, cnRLL_GLTypeToString(type),
				 p->attributes[i].name, size);
		
		const uint32_t semanticName = cnRLL_LookupAttributeSemanticName(p->attributes[i].name);
		CN_ASSERT(semanticName < CnAttributeSemanticNameTypes, "Couldn't find attribute "
			"semantic name for %s", p->attributes[i].name);
		const GLint location = glGetAttribLocation(p->id, p->attributes[i].name);
		CN_ASSERT(location >= 0, "CnAttribute %s cannot be found.", p->attributes[i].name);
		p->attributes[i].location = location;
		p->attributes[i].semanticName = semanticName;
		p->attributes[i].type = type;
		p->attributes[i].size = size;

		CN_ASSERT_NO_GL_ERROR();
	}
	p->numAttributes = numActiveAttributes;

	GLint numActiveUniforms;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numActiveUniforms);
	CN_TRACE(LogSysRender, "Active Uniforms: %d", numActiveUniforms);
	for (GLint i = 0; i < numActiveUniforms; ++i) {
		GLint size;
		GLenum type;
		glGetActiveUniform(program, (GLuint)i, CN_RLL_MAX_UNIFORM_NAME_LENGTH,
			NULL, &size, &type, p->uniforms[i].name);
		CN_TRACE(LogSysRender, "[%d]: %s '%s'   %d", i, cnRLL_GLTypeToString(type),
			p->uniforms[i].name, size);

		uint32_t storageLocation = cnRLL_LookupUniformStorageLocation(p->uniforms[i].name);
		CN_ASSERT(storageLocation < CnUniformNameTypes, "Couldn't find uniform "
			"semantic name for %s", p->uniforms[i].name);
		p->uniforms[i].size = size;
		p->uniforms[i].type = type;
		p->uniforms[i].location = glGetUniformLocation(p->id, p->uniforms[i].name);
		p->uniforms[i].storageLocation = storageLocation;
	}
	p->numUniforms = numActiveUniforms;

	CN_ASSERT_NO_GL_ERROR();
}

static void cnRLL_ApplyVertexAttribute(CnVertexFormat* f, uint32_t semanticName, uint32_t location)
{
	CN_ASSERT(f != NULL, "Cannot apply a vertex attribute from a null format");
	CN_ASSERT(semanticName < CnAttributeSemanticNameUnknown, "Unknown semantic name ID: %"
		PRIu32, semanticName);
	CN_ASSERT(f->attributes[semanticName].semanticName == semanticName,
		"CnAttribute semantic name (%" PRIu32 ") does not match expected (%" PRIu32 ")",
		f->attributes[semanticName].semanticName, semanticName);

	const CnVertexFormatAttribute* attribute = &f->attributes[semanticName];
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location,
		attribute->numComponents,
		attribute->componentType,
		attribute->normalized,
		attribute->stride,
		(void*)attribute->offset
		);

	CN_ASSERT_NO_GL_ERROR();
}

/**
 * Set uniforms according to global uniform storage, and enable attribute
 * pointers.
 */
static void cnRLL_EnableProgramForVertexFormat(uint32_t id, CnVertexFormat* format)
{
	CnProgram* p = &programs[id];
	CN_ASSERT(glIsProgram(p->id), "%" PRIu32 " is not a valid program.", id);
	CN_ASSERT(format != NULL, "Cannot enable program %" PRIu32 " for a null vertex format.", id);

	glUseProgram(p->id);
	CN_ASSERT_NO_GL_ERROR();

	for (uint32_t i = 0; i < p->numAttributes; ++i) {
		cnRLL_ApplyVertexAttribute(format, p->attributes[i].semanticName, p->attributes[i].location);
	}

	for (uint32_t i = 0; i < p->numUniforms; ++i) {
		cnRLL_ApplyUniform(&p->uniforms[i], uniformStorage);
	}
}

/**
 * Turns off vertex attributes associated with the given program.
 */
void cnRLL_DisableProgram(uint32_t id)
{
	CnProgram* p = &programs[id];
	for (uint32_t i = 0; i < p->numAttributes; ++i) {
		glDisableVertexAttribArray(p->attributes[i].location);
	}
}

bool cnRLL_CreateProgram(GLuint vertexShader, GLuint fragmentShader, GLuint* program,
	uint32_t programIndex);
void cnRLL_FillBuffers(void);
void cnRLL_InitSprites(void);
void cnRLL_LoadShaders(void);

void cnRLL_CheckGLError(const char* file, int line)
{
	const GLenum glError = glGetError();
	switch (glError)
	{
		case GL_NO_ERROR:
			return;
#define label_print(label) case label: CN_ERROR(LogSysRender, "OpenGL Error: %s:%d " #label, file, line); break;
		label_print(GL_INVALID_ENUM)
		label_print(GL_INVALID_VALUE)
		label_print(GL_INVALID_OPERATION)
		label_print(GL_INVALID_FRAMEBUFFER_OPERATION)
		label_print(GL_OUT_OF_MEMORY)
		label_print(GL_STACK_UNDERFLOW)
		label_print(GL_STACK_OVERFLOW)
#undef label_print
		default:
			CN_ERROR(LogSysRender, "Unknown error: %s:%d %d", file, line, glError);
	}
	CN_DEBUG_BREAK();
}

/**
 * Converts `GLenum` to a string for debugging and error reporting.
 */
const char* cnRLL_GLTypeToString(GLenum type)
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
void cnRLL_PrintProgram(GLuint program)
{
	CN_ASSERT(glIsProgram(program), "CnProgram is not a program");

	enum { bufferSize = 1024 };
	GLchar name[bufferSize];

	// Print attributes.
	GLint numActiveAttributes;
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numActiveAttributes);
	CN_TRACE(LogSysRender, "Active Attributes: %d", numActiveAttributes);
	for (GLint i = 0; i < numActiveAttributes; ++i) {
		GLint size;
		GLenum type;
		glGetActiveAttrib(program, (GLuint)i, bufferSize, NULL, &size, &type, name);
		CN_TRACE(LogSysRender, "[%d]: %s '%s'   %d", i, cnRLL_GLTypeToString(type), name, size);
	}

	GLint numActiveUniforms;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numActiveUniforms);
	CN_TRACE(LogSysRender, "Active Uniforms: %d", numActiveUniforms);
	for (GLint i = 0; i < numActiveUniforms; ++i) {
		GLint size;
		GLenum type;
		glGetActiveUniform(program, (GLuint)i, bufferSize, NULL, &size, &type, name);
		CN_TRACE(LogSysRender, "[%d]: %s '%s'   %d", i, cnRLL_GLTypeToString(type), name, size);
	}

	// Print the info log.
	GLint infoLogLength;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		if (MAX_INFO_LOG_LENGTH < infoLogLength) {
			CN_TRACE(LogSysRender, "Info log is too small to hold all output");
		}
		char infoLog[MAX_INFO_LOG_LENGTH];
		glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);
		CN_TRACE(LogSysRender, "Validation log: %s\n", infoLog);
	}
}

/**
 * The renderer may have requested a specific version of OpenGL, but this
 * program provides the current version being used.
 */
void cnRLL_PrintGLVersion(void)
{
	CN_ASSERT_NO_GL_ERROR();
	SDL_GL_MakeCurrent(window, gl);
	GLint majorVersion, minorVersion;
	glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
	CN_TRACE(LogSysRender, "Using OpenGL %i.%i", majorVersion, minorVersion);
	CN_TRACE(LogSysRender, "    Vendor: %s", glGetString(GL_VENDOR));
	CN_TRACE(LogSysRender, "    Renderer: %s", glGetString(GL_RENDERER));
	CN_TRACE(LogSysRender, "    Version: %s", glGetString(GL_VERSION));
	CN_TRACE(LogSysRender, "    GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	CN_ASSERT_NO_GL_ERROR();
}

/**
 * An openGL orthographic matrix.  Note that DirectX will need a related, but
 * slightly different version.
 *
 * Orthographic projection matrix to match the same dimensions as the window
 * size.  Scaling based on the same aspect ratio but big enough to see might be
 * a better option.
 */
static CnFloat4x4 cnRLL_OrthoProjection(const uint32_t width, const uint32_t height)
{
	const float far = -100;
	const float near = 100;
	const float w = (float)width;
	const float h = (float)height;
	const CnFloat4x4 scale = cnFloat4x4_NonUniformScale(2.0f / w, 2.0f / h, 2.0f / (far - near));
	const CnFloat4x4 trans = cnFloat4x4_Translate(-w / 2.0f, -h / 2.0f, -(far + near) / 2.0f);
	return cnFloat4x4_Multiply(trans, scale);
}

static bool cnRLL_CreateShader(GLuint* shader, const char* source, const uint32_t sourceLength)
{
	const GLchar* sources[] = { source };
	const GLint sizes[] = { sourceLength };
	glShaderSource(*shader, 1, sources, sizes);
	glCompileShader(*shader);

	GLint compileResult = GL_FALSE;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &compileResult);
	if (!compileResult) {
		CN_ERROR(LogSysRender, "Unable to compile shader: %s", source);
		return false;
	}

	GLint infoLogLength;
	glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		if (MAX_INFO_LOG_LENGTH < infoLogLength) {
			CN_ERROR(LogSysRender, "Info log buffer is too small to hold all output");
			return false;
		}
		char infoLog[MAX_INFO_LOG_LENGTH];
		glGetShaderInfoLog(*shader, MAX_INFO_LOG_LENGTH, NULL, infoLog);
		CN_TRACE(LogSysRender, "Compilation results for %s", infoLog);
	}
	return true;
}

void cnRLL_InitGL(void)
{
	cnLog_RegisterSystem(&LogSysRender, "Render", CnLogVerbosityTrace);

	// TODO: Settle on an appropriate OpenGL version to use.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	gl = SDL_GL_CreateContext(window);
	if (gl == NULL) {
		CN_FATAL_ERROR("Unable to create OpenGL context: %s", SDL_GetError());
	}

#ifdef _WIN32
	// Use GLEW on Windows to load function pointers to advanced OpenGL functions.
	glewInit();
#endif

	CN_TRACE(LogSysRender, "OpenGL renderer initialized");
	cnRLL_PrintGLVersion();
}

void cnRLL_ConfigureVSync(void)
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
void cnRLL_InitDummyVAO(void)
{
	static GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	CN_ASSERT_NO_GL_ERROR();
}

void cnRLL_InitVertexFormats(void)
{
	// Invalid all attributes on all vertex formats.
	for (uint32_t i=0; i < CnVertexFormatMax; ++i) {
		for (uint32_t j = 0; j < CN_RLL_MAX_VERTEX_FORMAT_ATTRIBUTES; ++j) {
			vertexFormats[i].attributes[j].semanticName = CnAttributeSemanticNameUnknown;
		}
	}

	// TODO: Load these from file.
	{
		CnVertexFormat* v = &vertexFormats[CnVertexFormatP4];
		CnVertexFormatAttribute* a = &v->attributes[CnAttributeSemanticNamePosition4];
		a->semanticName = CnAttributeSemanticNamePosition4;
		a->componentType = GL_FLOAT;
		a->numComponents = 4;
		a->normalized = GL_FALSE;
		a->stride = 0;
		a->offset = 0;
	}

	{
		CnVertexFormat* v = &vertexFormats[CnVertexFormatP2];
		CnVertexFormatAttribute* a = &v->attributes[CnAttributeSemanticNamePosition2];
		a->semanticName = CnAttributeSemanticNamePosition2;
		a->componentType = GL_FLOAT;
		a->numComponents = 2;
		a->normalized = GL_FALSE;
		a->stride = 0;
		a->offset = 0;
	}

	{
		CnVertexFormat* v = &vertexFormats[CnVertexFormatP2T2Interleaved];
		CnVertexFormatAttribute* p2 = &v->attributes[CnAttributeSemanticNamePosition2];
		p2->semanticName = CnAttributeSemanticNamePosition2;
		p2->componentType = GL_FLOAT;
		p2->numComponents = 2;
		p2->normalized = GL_FALSE;
		p2->stride = 4 * sizeof(float);
		p2->offset = 0;

		CnVertexFormatAttribute* t2 = &v->attributes[CnAttributeSemanticNameTexCoord2];
		t2->semanticName = CnAttributeSemanticNameTexCoord2;
		t2->componentType = GL_FLOAT;
		t2->numComponents = 2;
		t2->normalized = GL_FALSE;
		t2->stride = 4 * sizeof(float);
		t2->offset = 2 * sizeof(float);
	}

	{
		CnVertexFormat*v = &glyphFormat;
		CnVertexFormatAttribute* p2 = &v->attributes[CnAttributeSemanticNamePosition2];
		p2->semanticName = CnAttributeSemanticNamePosition2;
		p2->componentType = GL_FLOAT;
		p2->numComponents = 2;
		p2->normalized = GL_FALSE;
		p2->stride = 0;
		p2->offset = 0;

		CnVertexFormatAttribute* t2 = &v->attributes[CnAttributeSemanticNameTexCoord2];
		t2->semanticName = CnAttributeSemanticNameTexCoord2;
		t2->componentType = GL_FLOAT;
		t2->numComponents = 2;
		t2->normalized = GL_FALSE;
		t2->stride = 0;
		t2->offset = sizeof(float) * 2 * RLL_MAX_GLYPHS_PER_DRAW * RLL_VERTICES_PER_GLYPH;
	}
}

void cnRLL_FillFullScreenQuadBuffer(void)
{
	// OpenGL ndc is a cube from -1 to 1
	CnFloat2 vertices[] = {
		cnFloat2_Make(-1.0f, -1.0f),
		cnFloat2_Make(-1.0f, 1.0f),
		cnFloat2_Make(1.0f, -1.0f),
		cnFloat2_Make(1.0f, 1.0f)
	};
	glGenBuffers(1, &fullScreenQuadBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, fullScreenQuadBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	CN_ASSERT(fullScreenQuadBuffer, "Cannot allocate a buffer for the full screen quad");
	CN_ASSERT_NO_GL_ERROR();
}

void cnRLL_FillSpriteBuffer(void)
{
	typedef struct {
		CnFloat2 position;
		CnFloat2 texCoord2;
	} CnVertexP2T2;

	CnVertexP2T2 vertices[8];
	vertices[0] = (CnVertexP2T2) {
		cnFloat2_Make(0.0f, 0.0f),
		cnFloat2_Make(0.0f, 0.0f)
	};
	vertices[1] = (CnVertexP2T2) {
		cnFloat2_Make(0.0f, 1.0f),
		cnFloat2_Make(0.0f, 1.0f)
	};
	vertices[2] = (CnVertexP2T2) {
		cnFloat2_Make(1.0f, 0.0f),
		cnFloat2_Make(1.0f, 0.0f)
	};
	vertices[3] = (CnVertexP2T2) {
		cnFloat2_Make(1.0f, 1.0f),
		cnFloat2_Make(1.0f, 1.0f)
	};

	glGenBuffers(1, &spriteBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, spriteBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	CN_ASSERT(spriteBuffer, "Cannot allocate a buffer for the sprite buffer");
	CN_ASSERT(glIsBuffer(spriteBuffer), "Could not create sprite buffer");
	CN_ASSERT_NO_GL_ERROR();
}

void cnRLL_FillDebugQuadBuffer(void)
{
	// Doesn't matter, will be overwritten.
	CnFloat4 vertices[RLL_MAX_DEBUG_POINTS];
	memset(&vertices[0], 0, RLL_MAX_DEBUG_POINTS * sizeof(CnFloat4));
	glGenBuffers(1, &debugDrawBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, debugDrawBuffer);

	// Allocate the maximum used spaced and then override it.
	glBufferData(GL_ARRAY_BUFFER, RLL_MAX_DEBUG_POINTS * 4 * sizeof(float), vertices, GL_DYNAMIC_DRAW);

	CN_ASSERT(debugDrawBuffer, "Cannot allocate a buffer for the debug drawing");
	CN_ASSERT_NO_GL_ERROR();
}

void cnRLL_FillGlyphBuffer(void)
{
	CN_ASSERT_NO_GL_ERROR();
	glGenBuffers(1, &glyphBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, glyphBuffer);
	glBufferData(GL_ARRAY_BUFFER, RLL_GLYPH_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);
	CN_ASSERT_NO_GL_ERROR();
}

void cnRLL_FillBuffers(void)
{
	cnRLL_FillSpriteBuffer();
	cnRLL_FillFullScreenQuadBuffer();
	cnRLL_FillDebugQuadBuffer();
	cnRLL_FillGlyphBuffer();
}

void cnRLL_InitSprites(void)
{
	cnRLL_SpriteInit();
	cnRLL_FontInit();
}

void cnRLL_LoadSimpleShader(const char* vertexShaderFileName,
	const char* fragmentShaderFileName, uint32_t programIndex)
{
	CnPathBuffer fragmentShaderPath;
	CnPathBuffer vertexShaderPath;
	CnDynamicBuffer fragmentShaderBuffer;
	CnDynamicBuffer vertexShaderBuffer;

	// Find the fragment shader.
	if (!cnAssets_PathBufferFor(fragmentShaderFileName, &fragmentShaderPath)) {
		CN_ERROR(LogSysRender, "Unable to find asset for fragment shader: %s",
			fragmentShaderFileName);
	}

	if (!cnPath_IsFile(fragmentShaderPath.str)) {
		CN_ERROR(LogSysRender, "Fragment shader is not a file: %s",
			fragmentShaderPath.str);
	}

	if (!cnAssets_ReadFile(fragmentShaderPath.str, CnFileTypeText, &fragmentShaderBuffer)) {
		CN_ERROR(LogSysRender, "Unable to read fragment shader text");
	}

	// Find the vertex shader.
	if (!cnAssets_PathBufferFor(vertexShaderFileName, &vertexShaderPath)) {
		CN_ERROR(LogSysRender, "Unable to find asset for vertex shader: %s",
			vertexShaderFileName);
	}

	if (!cnPath_IsFile(vertexShaderPath.str)) {
		CN_ERROR(LogSysRender, "Vertex shader is not a file: %s",
			vertexShaderPath.str);
	}

	if (!cnAssets_ReadFile(vertexShaderPath.str, CnFileTypeText, &vertexShaderBuffer)) {
		CN_ERROR(LogSysRender, "Unable to read vertex shader text");
	}

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	if (!glIsShader(vertexShader)) {
		CN_ERROR(LogSysRender, "Unable to allocate space for vertex shader");
	}

	if (!glIsShader(fragmentShader)) {
		CN_ERROR(LogSysRender, "Unable to allocate space for fragment shader");
	}

	cnRLL_CreateShader(&fragmentShader, fragmentShaderBuffer.contents, fragmentShaderBuffer.size);
	cnRLL_CreateShader(&vertexShader, vertexShaderBuffer.contents, vertexShaderBuffer.size);

	GLuint program;
	if (!cnRLL_CreateProgram(vertexShader, fragmentShader, &program, programIndex)) {
		CN_TRACE(LogSysRender, "Fragment shader %s", fragmentShaderBuffer.contents);
		CN_TRACE(LogSysRender, "Vertex shader %s", vertexShaderBuffer.contents);
		CN_ERROR(LogSysRender, "Unable to create shader program");
	}
	cnMem_Free(&vertexShaderBuffer);
	cnMem_Free(&fragmentShaderBuffer);
}

void cnRLL_LoadShaders(void)
{
	cnRLL_LoadSimpleShader("shaders/fullscreen_textured_quad.vert",
		"shaders/uv_as_red_green.frag", CnProgramIndexFullScreen);
	cnRLL_LoadSimpleShader("shaders/solid_polygon.vert",
		"shaders/solid_polygon.frag", CnProgramIndexSolidPolygon);
	cnRLL_LoadSimpleShader("shaders/atlas_sprite.vert",
		"shaders/atlas_sprite.frag", CnProgramIndexSprite);
}

bool cnRLL_CreateProgram(GLuint vertexShader, GLuint fragmentShader, GLuint* program,
	uint32_t programIndex)
{
	CN_ASSERT_NO_GL_ERROR();

	if (!program) {
		CN_ERROR(LogSysRender, "program is a NULL ptr");
		return false;
	}

	if (!glIsShader(vertexShader)) {
		CN_ERROR(LogSysRender, "Vertex shader doesn't exist");
		return false;
	}

	if (!glIsShader(fragmentShader)) {
		CN_ERROR(LogSysRender, "Fragment shader doesn't exist");
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
			CN_ERROR(LogSysRender, "Info log is too small to hold all output");
		}
		char infoLog[MAX_INFO_LOG_LENGTH];
		glGetProgramInfoLog(*program, infoLogLength, NULL, infoLog);
		CN_TRACE(LogSysRender, "Link log: %s\n", infoLog);
	}

	if (linkResult == GL_FALSE) {
		CN_ERROR(LogSysRender, "Unable to link program");
		return false;
	}

	glDetachShader(*program, vertexShader);
	glDetachShader(*program, fragmentShader);

#if CN_DEBUG
	cnRLL_PrintProgram(*program);
#endif

	CN_ASSERT_NO_GL_ERROR();

	if (linkResult == GL_TRUE) {
		cnRLL_RegisterProgram(programIndex, *program);
	}
	CN_ASSERT_NO_GL_ERROR();

	return linkResult == GL_TRUE;
}

void cnRLL_Init(uint32_t width, uint32_t height)
{
	cnRLL_InitGL();
	cnRLL_ConfigureVSync();
	cnRLL_InitDummyVAO();
	cnRLL_InitVertexFormats();
	cnRLL_FillBuffers();
	cnRLL_InitSprites();
	cnRLL_LoadShaders();

	windowWidth = (GLsizei)width;
	windowHeight = (GLsizei)height;
	uniformStorage[CnUniformNameProjection].f44 = cnRLL_OrthoProjection(width, height);
}

void cnRLL_Shutdown(void)
{

}

void cnRLL_StartFrame(void)
{
	SDL_GL_MakeCurrent(window, gl);
	CN_ASSERT_NO_GL_ERROR();
}

void cnRLL_EndFrame(void)
{
	CN_ASSERT_NO_GL_ERROR();
	SDL_GL_SwapWindow(window);
}

void cnRLL_Clear(CnRGBA8u color)
{
	glClearColor(color.red, color.green, color.blue, color.alpha);
	glClear(GL_COLOR_BUFFER_BIT);
}

void cnRLL_SetFullScreenViewport(void)
{
	glViewport(0, 0, windowWidth, windowHeight);
}

bool cnRLL_LoadSprite(CnSpriteId id, const char* path)
{
	CN_ASSERT_NO_GL_ERROR();

	glGenTextures(1, &spriteTextures[id]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, spriteTextures[id]);

	CnImageRGBA8 image;
	if (!cnImageRGBA8_Allocate(&image, path)) {
		return false;
	}

	// Don't mipmap for now.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	// TODO: Use proxy textures to test to see if sufficient space exists.
	// TODO: Should this be GL_RGBA8?
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width, image.height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, image.pixels.contents);

	// Set the texture parameters.
	// https://stackoverflow.com/questions/3643932/what-is-the-scope-of-gltexparameters-in-opengl
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	CN_ASSERT(glIsTexture(spriteTextures[id]), "Unable to reserve texture for "
		"sprite loading from path: %s", path);

	cnImageRGBA8_Free(&image);

	CN_ASSERT_NO_GL_ERROR();
	return true;
}

void cnRLL_DrawSprite(CnSpriteId id, CnFloat2 position, CnDimension2f size)
{
	CN_ASSERT_NO_GL_ERROR();

	cnRLL_SetFullScreenViewport();

	GLuint texture = spriteTextures[id];
	CN_ASSERT(glIsTexture(texture), "Sprite %" PRIu32 " does not have a valid"
		"texture", id);
	cnRLL_ReadyTexture2(0, spriteTextures[id]);

	uniformStorage[CnUniformNameModelView].f44 = cnFloat4x4_Multiply(
		cnFloat4x4_NonUniformScale(size.width, size.height, 1.0f),
		cnFloat4x4_Translate(position.x, position.y, 0.0f));;

	glBindBuffer(GL_ARRAY_BUFFER, spriteBuffer);
	CN_ASSERT_NO_GL_ERROR();
	cnRLL_EnableProgramForVertexFormat(CnProgramIndexSprite, &vertexFormats[CnVertexFormatP2T2Interleaved]);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	cnRLL_DisableProgram(CnProgramIndexSprite);

	CN_ASSERT_NO_GL_ERROR();
}

/**
 * Loads a PSF2 font from a given font into the specific id.
 */
bool cnRLL_LoadPSF2Font(CnFontId id, const char* path)
{
	// TODO: Check to determine if the font id has already been used.

	CN_ASSERT(path != NULL, "Cannot load a font from a null path");
	CN_ASSERT(cnPath_IsFile(path), "PSF2 font does not exist");

	CN_ASSERT_NO_GL_ERROR();

	CnFontPSF2* font = &fonts[id];
	cnFont_PSF2Allocate(&fonts[id], path);

	glGenTextures(1, &fontTextures[id]);
	CN_ASSERT(fontTextures[id] != 0, "Could not allocate a texture name for the font.");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fontTextures[id]);

	// Don't mipmap for now.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	cnImageRGBA8_Flip(&font->atlas.image);
	CN_ASSERT(font->atlas.image.pixels.size == font->atlas.backingSizePixels.width * font->atlas.backingSizePixels.height * 4,
		"Backing size doesn't match pixel size.");

	// TODO: Use proxy textures to test to see if sufficient space exists.
	glTexImage2D(GL_TEXTURE_2D,
		0, // mipmap level
		GL_RGB,
		font->atlas.backingSizePixels.width,
		font->atlas.backingSizePixels.height,
		0, // border
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		font->atlas.image.pixels.contents);

	CN_ASSERT_NO_GL_ERROR();

	// Set the texture parameters.
	// https://stackoverflow.com/questions/3643932/what-is-the-scope-of-gltexparameters-in-opengl
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	CN_ASSERT(glIsTexture(fontTextures[id]), "Unable to reserve texture for "
		"font loading from path: %s", path);

	CN_ASSERT_NO_GL_ERROR();
	return true;
}

static void cnRLL_AddToGlyphBatch(CnFloat2 position, CnDimension2f size, CnFloat2* texCoords)
{
	const uint32_t glyphOffset = usedGlyphs * RLL_VERTICES_PER_GLYPH;
	glyphTexCoords[glyphOffset] = texCoords[0];
	glyphTexCoords[glyphOffset + 1] = texCoords[1];
	glyphTexCoords[glyphOffset + 2] = texCoords[2];

	glyphTexCoords[glyphOffset + 3] = texCoords[1];
	glyphTexCoords[glyphOffset + 4] = texCoords[3];
	glyphTexCoords[glyphOffset + 5] = texCoords[2];

	glyphVertices[glyphOffset + 0] = position;
	glyphVertices[glyphOffset + 1] = cnFloat2_Add(position, cnFloat2_Make(size.width, 0.0f));
	glyphVertices[glyphOffset + 2] = cnFloat2_Add(position, cnFloat2_Make(0.0f, size.height));

	glyphVertices[glyphOffset + 3] = cnFloat2_Add(position, cnFloat2_Make(size.width, 0.0f));
	glyphVertices[glyphOffset + 4] = cnFloat2_Add(position, cnFloat2_Make(size.width, size.height));
	glyphVertices[glyphOffset + 5] = cnFloat2_Add(position, cnFloat2_Make(0.0f, size.height));
	++usedGlyphs;
}

static void cnRLL_AppendGlyph(CnFontId id, CnFloat2 position, CnGlyphIndex glyphIndex)
{
	CnFontPSF2* font = &fonts[id];
	CN_ASSERT(glyphIndex != CN_GRAPHEME_INDEX_INVALID, "Cannot draw an invalid glyph");

	// Get the glyph size, should go in printing parameters.
	// TODO: Use aspect ratio of the glyph.
	const CnDimension2f glyphSize = (CnDimension2f) { .width = 30.0f, .height = 50.0f };

	CnFloat2 texCoords[4];
	cnTextureAtlas_TexCoordForSubImage(&font->atlas, &texCoords[0], glyphIndex);
	cnRLL_AddToGlyphBatch(position, glyphSize, texCoords);
}

/**
 * The final draw call to write text once all the glyphs have been assembled.
 */
static void cnRLL_DrawGlyphs(CnFontId id)
{
	CN_ASSERT_NO_GL_ERROR();
	cnRLL_SetFullScreenViewport();
	const GLuint texture = fontTextures[id];
	CN_ASSERT(glIsTexture(texture), "Sprite %" PRIu32 " does not have a valid"
		"texture", texture);
	cnRLL_ReadyTexture2(0, fontTextures[id]);

	uniformStorage[CnUniformNameModelView].f44 = cnFloat4x4_Identity();
	glBindBuffer(GL_ARRAY_BUFFER, glyphBuffer);

	const size_t verticesSize = sizeof(float) * 2 * RLL_MAX_GLYPH_VERTICES_PER_DRAW;
	const size_t texCoordsSize = sizeof(float) * 2 * RLL_MAX_GLYPH_VERTICES_PER_DRAW;
	CN_ASSERT(verticesSize + texCoordsSize == RLL_GLYPH_BUFFER_SIZE, "Insufficient size"
		" for vertices and texture coordinates: %zu and %zu -> %zu",
		verticesSize, texCoordsSize,  RLL_GLYPH_BUFFER_SIZE);

	glBufferSubData(GL_ARRAY_BUFFER, 0, verticesSize, glyphVertices);
	glBufferSubData(GL_ARRAY_BUFFER, verticesSize, texCoordsSize, glyphTexCoords);
	cnRLL_EnableProgramForVertexFormat(CnProgramIndexSprite, &glyphFormat);
	glDrawArrays(GL_TRIANGLES, 0, 6 * usedGlyphs);

	usedGlyphs = 0;
	cnRLL_DisableProgram(CnProgramIndexSprite);
	CN_ASSERT_NO_GL_ERROR();
}

/**
 * @param id
 * @param textPosition
 * @param text a null-terminated, utf-8 string
 */
void cnRLL_DrawSimpleText(CnFontId id, CnTextDrawParams* params, const char* text)
{
	CnFontPSF2* font = &fonts[id];
	// TODO: Check to ensure the id is valid.
	CN_ASSERT(params != NULL, "Cannot draw with null parameters.");
	CN_ASSERT(params->layout == CnLayoutHorizontal,
		"Only horizontal layouts are currently supported.");
	CN_ASSERT(params->printDirection == CnPrintDirectionLeftToRight,
		"Only left-to-right print direction is currently supported.");
	CN_ASSERT(text != NULL, "Cannot draw a null text");

	// When printing characters, we need to know:
	// 1. where we are in the string.
	// 2. where to draw the next glyph.
	// 3. the distance between glyphs.
	const uint8_t* cursor = (const uint8_t*)text;
	CnFloat2 glyphPosition = params->position;
	float scale = 3.0f;
	CnFloat2 glyphAdvance = cnFloat2_Make(font->glyphSize.width * scale, 0.0f);

	// Text is a utf-8 string, so its byte length is not necessarily its glyph length.
	const size_t textLengthInBytes = strlen(text);
	const char* textAfterLastByte = text + textLengthInBytes;

	while (cursor < (const uint8_t*)textAfterLastByte) {
		// The next grapheme might be longer than a single code point.  We don't
		// know how long the grapheme is until we match it.
		uint32_t graphemeByteSize = cnUtf8_NumBytesInCodePoint(*cursor);
		for (uint32_t graphemeLength = 1; graphemeLength < CN_MAX_CODE_POINTS_IN_GRAPHEME; ++graphemeLength) {
			const CnGlyphIndex graphemeIndex = cnGraphemeMap_GraphemeIndexForCodePoints(&font->map, (uint8_t*) cursor,
																						graphemeLength);
			if (graphemeIndex != CN_GRAPHEME_INDEX_INVALID) {
				cnRLL_AppendGlyph(id, glyphPosition, font->map.glyphs[graphemeIndex]);
				graphemeByteSize = font->map.graphemes[graphemeIndex].byteLength;
				break;
			}
		}
		glyphPosition = cnFloat2_Add(glyphPosition, glyphAdvance);

		cursor = cnUtf8_StringNext(cursor);
	}
	cnRLL_DrawGlyphs(id);
	CN_ASSERT_NO_GL_ERROR();
}

/**
 * Draw a fullscreen debug rect.
 */
void cnRLL_DrawDebugFullScreenRect(void)
{
	CN_ASSERT_NO_GL_ERROR();

	cnRLL_SetFullScreenViewport();

	glBindBuffer(GL_ARRAY_BUFFER, fullScreenQuadBuffer);

	cnRLL_EnableProgramForVertexFormat(CnProgramIndexFullScreen, &vertexFormats[CnVertexFormatP2]);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	cnRLL_DisableProgram(CnProgramIndexFullScreen);

	CN_ASSERT_NO_GL_ERROR();
}

/**
 * Draws a rectangle at a given center point with known dimensions.
 */
void cnRLL_DrawDebugRect(CnFloat2 center, CnDimension2f dimensions, CnFloat4 color)
{
	cnRLL_SetFullScreenViewport();

	glBindBuffer(GL_ARRAY_BUFFER, debugDrawBuffer);

	uniformStorage[CnUniformNameViewModel].f44 = cnFloat4x4_Identity();
	uniformStorage[CnUniformNamePolygonColor].f4 = color;

	cnRLL_EnableProgramForVertexFormat(CnProgramIndexSolidPolygon, &vertexFormats[CnVertexFormatP2]);

	CnFloat2 vertices[4];
	vertices[0] = cnFloat2_Make(-dimensions.width / 2.0f, -dimensions.height / 2.0f);
	vertices[1] = cnFloat2_Make(dimensions.width / 2.0f, -dimensions.height / 2.0f);
	vertices[2] = cnFloat2_Make(-dimensions.width / 2.0f, dimensions.height / 2.0f);
	vertices[3] = cnFloat2_Make(dimensions.width / 2.0f, dimensions.height / 2.0f);

	for (uint32_t i = 0; i < 4; ++i) {
		vertices[i].x += center.x;
		vertices[i].y += center.y;
	}

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	cnRLL_DisableProgram(CnProgramIndexSolidPolygon);

	CN_ASSERT_NO_GL_ERROR();
}

void cnRLL_DrawDebugLine(float x1, float y1, float x2, float y2, CnRGB8u color)
{
	// TODO: Probably shouldn't reset viewport.
	cnRLL_SetFullScreenViewport();

	glBindBuffer(GL_ARRAY_BUFFER, debugDrawBuffer);

	uniformStorage[CnUniformNameViewModel].f44 = cnFloat4x4_Identity();
	uniformStorage[CnUniformNamePolygonColor].f4 = cnFloat4_Make(
		(float) color.r / 255.0f, (float) color.g / 255.0f, (float) color.b / 255.0f, 1.0f);

	cnRLL_EnableProgramForVertexFormat(CnProgramIndexSolidPolygon, &vertexFormats[CnVertexFormatP2]);

	CnFloat2 vertices[2];
	vertices[0] = cnFloat2_Make(x1, y1);
	vertices[1] = cnFloat2_Make(x2, y2);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glDrawArrays(GL_LINES, 0, 2);

	cnRLL_DisableProgram(CnProgramIndexSolidPolygon);
	CN_ASSERT_NO_GL_ERROR();
}

void cnRLL_DrawDebugLineStrip(CnFloat2* points, uint32_t numPoints, CnRGB8u color)
{
	CN_ASSERT(numPoints < RLL_MAX_DEBUG_POINTS, "Exceeded number of debug points "
		"to draw: %" PRIu32 " (%" PRIu32 " max)", numPoints, RLL_MAX_DEBUG_POINTS);

	// TODO: Probably shouldn't reset viewport.
	cnRLL_SetFullScreenViewport();

	glBindBuffer(GL_ARRAY_BUFFER, debugDrawBuffer);

	uniformStorage[CnUniformNameViewModel].f44 = cnFloat4x4_Identity();
	uniformStorage[CnUniformNamePolygonColor].f4 = cnFloat4_Make(
		(float) color.r / 255.0f, (float) color.g / 255.0f, (float) color.b / 255.0f, 1.0f);

	cnRLL_EnableProgramForVertexFormat(CnProgramIndexSolidPolygon, &vertexFormats[CnVertexFormatP2]);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(CnFloat2) * numPoints, points);
	glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)numPoints);

	cnRLL_DisableProgram(CnProgramIndexSolidPolygon);

	CN_ASSERT_NO_GL_ERROR();
}

void cnRLL_DrawDebugFont(CnFontId id, CnFloat2 center, CnDimension2f size)
{
	CN_ASSERT_NO_GL_ERROR();

	cnRLL_SetFullScreenViewport();

	const GLuint texture = fontTextures[id];
	CN_ASSERT(glIsTexture(texture), "Font %" PRIu32 " does not have a valid"
		"texture", id);
	cnRLL_ReadyTexture2(0, texture);

	uniformStorage[CnUniformNameModelView].f44 = cnFloat4x4_Multiply(
		cnFloat4x4_NonUniformScale(size.width, size.height, 1.0f),
		cnFloat4x4_Translate(center.x, center.y, 0.0f));;

	glBindBuffer(GL_ARRAY_BUFFER, spriteBuffer);
	CN_ASSERT_NO_GL_ERROR();
	cnRLL_EnableProgramForVertexFormat(CnProgramIndexSprite, &vertexFormats[CnVertexFormatP2T2Interleaved]);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	cnRLL_DisableProgram(CnProgramIndexSprite);

	CN_ASSERT_NO_GL_ERROR();
}

/**
 * Creates a line of points to form circle in a counter clockwise winding.
 */
static void cnRLL_CreateCircle(CnFloat2* vertices, uint32_t numVertices, float radius)
{
	CN_ASSERT(vertices != NULL, "Cannot write vertices into a null pointer");
	CN_ASSERT(radius > 0.0f, "Radius must positive: %f provided", radius);
	const float arcAngle = 2 * 3.14159f / (float)(numVertices);
	for (uint32_t i = 0; i < numVertices; ++i) {
		vertices[i] = cnFloat2_Make(
			radius * cosf(i * arcAngle),
			radius * sinf(i * arcAngle));
	}
}

#define RLL_MAX_CIRCLE_POINTS 30

void cnRLL_OutlineCircle(CnFloat2 center, float radius, CnRGB8u color, uint32_t numSegments)
{
	const uint32_t numPoints = numSegments + 1;
	CN_ASSERT(numSegments < RLL_MAX_CIRCLE_POINTS, "Exceeded maximum number of circle"
		"draw points: %" PRIu32 " of %" PRIu32, numSegments - 1, numPoints);

	static CnFloat2 points[RLL_MAX_CIRCLE_POINTS];
	cnRLL_SetFullScreenViewport();

	glBindBuffer(GL_ARRAY_BUFFER, debugDrawBuffer);

	uniformStorage[CnUniformNameViewModel].f44 = cnFloat4x4_Translate(center.x, center.y, 0.0f);
	uniformStorage[CnUniformNamePolygonColor].f4 = cnFloat4_Make(
		(float) color.r / 255.0f, (float) color.g / 255.0f, (float) color.b / 255.0f, 1.0f);

	cnRLL_EnableProgramForVertexFormat(CnProgramIndexSolidPolygon, &vertexFormats[CnVertexFormatP2]);

	cnRLL_CreateCircle(&points[0], numPoints, radius);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(CnFloat2) * numPoints, points);
	glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)numPoints);

	cnRLL_DisableProgram(CnProgramIndexSolidPolygon);

	CN_ASSERT_NO_GL_ERROR();
}
