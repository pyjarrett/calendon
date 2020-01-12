#include "render-ll.h"

#include "kn.h"

#include "assets.h"
#include "color.h"
#include "fileio.h"
#include "log.h"
#include "math.h"
#include "memory.h"

#include <spa_fu/spa_fu.h>

#include <stdbool.h>


extern struct SDL_Window* window;
static uint32_t windowWidth, windowHeight;

static SDL_GLContext* gl;

static float4x4 projection;

static GLuint spriteProgram;
static GLuint fullScreenDebugProgram;
static GLuint fullScreenQuadBuffer;

#define RLL_NUM_SPRITES 128
uint32_t spritesUsed;
float4x4 spriteTransforms[RLL_NUM_SPRITES];
float4 spriteVertexBuffer[RLL_NUM_SPRITES];
float4 spriteTint[RLL_NUM_SPRITES];

uint32_t LogSysRender;

bool RLL_CreateProgram(GLuint vertexShader, GLuint fragmentShader, GLuint* program);

#if KN_DEBUG
void RLL_CheckGLError(const char* file, const int line)
{
	const GLenum glError = glGetError();
	switch (glError)
	{
		case GL_NO_ERROR:
			return;
#define label_print(label) case label: KN_TRACE(LogSysRender, "OpenGL Error: %s:%d " #label, file, line); break;
		label_print(GL_INVALID_ENUM)
		label_print(GL_INVALID_VALUE)
		label_print(GL_INVALID_OPERATION)
		label_print(GL_INVALID_FRAMEBUFFER_OPERATION)
		label_print(GL_OUT_OF_MEMORY)
		label_print(GL_STACK_UNDERFLOW)
		label_print(GL_STACK_OVERFLOW)
#undef label_print
		default:
			KN_TRACE(LogSysRender, "Unknown error: %s:%d %d", file, line, glError);
	}
	KN_DEBUG_BREAK();
}

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
		default: return "Unknown type";
	}
#undef strType
}

void printProgram(const GLuint program)
{
	// Print attributes.
	GLint numActiveAttributes;
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numActiveAttributes);
	KN_TRACE(LogSysRender, "Active Attributes: %d", numActiveAttributes);
	for (int i = 0; i < numActiveAttributes; ++i) {
		const GLsizei bufferSize = 1024;
		GLchar name[bufferSize];
		GLint size;
		GLenum type;
		glGetActiveAttrib(program, i, bufferSize, NULL, &size, &type, name);
		KN_TRACE(LogSysRender, "[%d]: %s '%s'   %d", i, RLL_GLTypeToString(type), name, size);
	}

	GLint numActiveUniforms;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numActiveUniforms);
	KN_TRACE(LogSysRender, "Active Uniforms: %d", numActiveUniforms);
	for (int i = 0; i < numActiveUniforms; ++i) {
		const GLsizei bufferSize = 1024;
		GLchar name[bufferSize];
		GLint size;
		GLenum type;
		glGetActiveUniform(program, i, bufferSize, NULL, &size, &type, name);
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
		const uint32_t MAX_INFO_LOG_LENGTH = 4096;
		if (MAX_INFO_LOG_LENGTH < infoLogLength) {
			KN_TRACE(LogSysRender, "Info log is too small to hold all output");
		}
		char infoLog[MAX_INFO_LOG_LENGTH];
		glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);
		KN_TRACE(LogSysRender, "Validation log: %s\n", infoLog);
	}
}

void printGLVersion()
{
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

static float4x4 RLL_OrthoProjection(const uint32_t width, const uint32_t height)
{
	const float far = 0;
	const float near = 100;
	const float4x4 scale = float4x4_nonuniformScale(2.0f / width, 2.0f / height, 2.0f / (far - near));
	const float4x4 trans = float4x4_translate(-width / 2.0f, -height / 2.0f, -(far + near) / 2.0f);
	return float4x4_multiply(trans, scale);
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
		const int MAX_INFO_LOG_LENGTH = 4096;
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

void RLL_InitGL()
{
	Log_RegisterSystem(&LogSysRender, "Render", KN_LOG_TRACE);

	// Get up and running quickly with OpenGL 3.1 with old-school functions.
	// TODO: Replace with Core profile once something is working.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	gl = SDL_GL_CreateContext(window);
	if (gl == NULL) {
		KN_FATAL_ERROR("Unable to create OpenGL context: %s", SDL_GetError());
	}
	KN_TRACE(LogSysRender, "OpenGL renderer initialized");
}

void RLL_ConfigureVSync()
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
void RLL_InitDummyVAO()
{
	static GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
}

void RLL_FillSpriteBuffer()
{
	for (uint32_t i=0; i<RLL_NUM_SPRITES; ++i) {
		spriteVertexBuffer[0] = float4_Make(0.0f, 0.0f, 0.0f, 0.0f);
		spriteVertexBuffer[1] = float4_Make(0.0f, 1.0f, 0.0f, 0.0f);
		spriteVertexBuffer[2] = float4_Make(1.0f, 0.0f, 0.0f, 0.0f);
		spriteVertexBuffer[3] = float4_Make(1.0f, 1.0f, 0.0f, 0.0f);
	}
}

void RLL_FillFullScreenQuadBuffer()
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

void RLL_LoadShaders()
{
	const int maxShaderTextLength = 1024;
	char fragmentShaderPath[maxShaderTextLength];
	char vertexShaderPath[maxShaderTextLength];
	DynamicBuffer fragmentShaderBuffer;
	DynamicBuffer vertexShaderBuffer;

	// Read fragment shader
	if (Assets_PathFor("shaders/uv_as_red_green.frag", fragmentShaderPath, maxShaderTextLength)) {
		if (SPA_IsFile(fragmentShaderPath)) {
			KN_TRACE(LogSysMain, "\e[32m" "%s found" "\e[39m", fragmentShaderPath);
		}
		else {
			KN_TRACE(LogSysMain, "%s not found", fragmentShaderPath);
		}
	}

	if (!File_Read(fragmentShaderPath, KN_FILE_TYPE_TEXT, &fragmentShaderBuffer)) {
		KN_ERROR(LogSysMain, "Unable to read fragment shader text");
	}

	// Read vertex shader
	if (Assets_PathFor("shaders/fullscreen_textured_quad.vert", vertexShaderPath, maxShaderTextLength)) {
		if (SPA_IsFile(vertexShaderPath)) {
			KN_TRACE(LogSysMain, "\e[32m" "%s found" "\e[39m", vertexShaderPath);
		}
		else {
			KN_TRACE(LogSysMain, "%s not found", vertexShaderPath);
		}
	}

	if (!File_Read(vertexShaderPath, KN_FILE_TYPE_TEXT, &vertexShaderBuffer)) {
		KN_ERROR(LogSysMain, "Unable to read vertex shader text");
	}

	//KN_TRACE(LogSysMain, "Fragment shader %s", fragmentShaderBuffer.contents);
	//KN_TRACE(LogSysMain, "Vertex shader %s", vertexShaderBuffer.contents);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	if (!glIsShader(vertexShader)) {
		KN_ERROR(LogSysMain, "Unable to allocate space for vertex shader");
	}

	if (!glIsShader(fragmentShader)) {
		KN_ERROR(LogSysMain, "Unable to allocate space for fragment shader");
	}

	RLL_CreateShader(&fragmentShader, fragmentShaderBuffer.contents, fragmentShaderBuffer.size);
	RLL_CreateShader(&vertexShader, vertexShaderBuffer.contents, vertexShaderBuffer.size);

	RLL_CreateProgram(vertexShader, fragmentShader, &fullScreenDebugProgram);
	Mem_Free(&vertexShaderBuffer);
	Mem_Free(&fragmentShaderBuffer);
}

bool RLL_CreateProgram(GLuint vertexShader, GLuint fragmentShader, GLuint* program)
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
		const uint32_t MAX_INFO_LOG_LENGTH = 4096;
		if (MAX_INFO_LOG_LENGTH < infoLogLength) {
			KN_ERROR(LogSysRender, "Info log is too small to hold all output");
		};
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
	printProgram(*program);
#endif

	KN_ASSERT_NO_GL_ERROR();
	return linkResult == GL_TRUE;
}

void RLL_Init(const uint32_t width, const uint32_t height)
{
	RLL_InitGL();
	RLL_ConfigureVSync();
	RLL_InitDummyVAO();
	RLL_FillSpriteBuffer();
	RLL_FillFullScreenQuadBuffer();
	RLL_LoadShaders();

	windowWidth = width;
	windowHeight = height;
	projection = RLL_OrthoProjection(width, height);
}

void RLL_StartFrame()
{
	SDL_GL_MakeCurrent(window, gl);
	spritesUsed = 0;
}

void RLL_EndFrame()
{
	SDL_GL_SwapWindow(window);
}

void RLL_Clear(rgba8i color)
{
	glClearColor(color.red, color.green, color.blue, color.alpha);
	glClear(GL_COLOR_BUFFER_BIT);
}

void RLL_SetFullScreenViewport()
{
	glViewport(0, 0, windowWidth, windowHeight);
}

/**
 * Draw a fullscreen debug rect.
 */
void RLL_DrawDebugFullScreenRect()
{
	KN_ASSERT_NO_GL_ERROR();

	RLL_SetFullScreenViewport();

	glUseProgram(fullScreenDebugProgram);

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

	glBindBuffer(GL_ARRAY_BUFFER, fullScreenQuadBuffer);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(0);

	KN_ASSERT_NO_GL_ERROR();
}
