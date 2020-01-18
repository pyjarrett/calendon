#include "render-ll.h"

#include "kn.h"

#include "assets.h"
#include "assets-fileio.h"
#include "compat-sdl.h"
#include "color.h"
#include "log.h"
#include "math4.h"
#include "memory.h"

#include <spa_fu/spa_fu.h>

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
 * Orthographic projection matrix to match the same dimensions as the window
 * size.  Scaling based on the same aspect ratio but big enough to see might be
 * a better option.
 */
static float4x4 projection;

static GLuint spriteProgram;
static GLuint fullScreenDebugProgram;
static GLuint fullScreenQuadBuffer;

/**
 * Vertex `GL_ARRAY_BUFFER` containing vertex information for drawing debug shapes.
 */
static GLuint debugQuadBuffer;

/**
 * Program to use for drawing debug shapes.
 */
static GLuint solidPolygonProgram;

#define RLL_NUM_SPRITES 128
uint32_t spritesUsed;
float4x4 spriteTransforms[RLL_NUM_SPRITES];
float4 spriteVertexBuffer[RLL_NUM_SPRITES];
float4 spriteTint[RLL_NUM_SPRITES];

#define MAX_INFO_LOG_LENGTH 4096

/**
 * Logging ID for renderer logging.
 */
uint32_t LogSysRender;

bool RLL_CreateProgram(GLuint vertexShader, GLuint fragmentShader, GLuint* program);
void RLL_FillBuffers();
void RLL_LoadShaders();

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
void RLL_PrintProgram(const GLuint program)
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
void RLL_PrintGLVersion()
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

#ifdef _WIN32
	// Use GLEW on Windows to load function pointers to advanced OpenGL functions.
	glewInit();
#endif

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
    KN_ASSERT_NO_GL_ERROR();
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

void RLL_FillDebugQuadBuffer()
{
	// Doesn't matter, will be overwritten.
	float4 vertices[] = {
		float4_Make(-1.0f, -1.0f, 0.0f, 0.0f),
		float4_Make(-1.0f, 1.0f, 0.0f, 0.0f),
		float4_Make(1.0f, -1.0f, 0.0f, 0.0f),
		float4_Make(1.0f, 1.0f, 0.0f, 0.0f)
	};
	glGenBuffers(1, &debugQuadBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, debugQuadBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	KN_ASSERT(debugQuadBuffer, "Cannot allocate a buffer for the debug quad buffer");
	KN_ASSERT_NO_GL_ERROR();
}

void RLL_FillBuffers()
{
	RLL_FillSpriteBuffer();
	RLL_FillFullScreenQuadBuffer();
	RLL_FillDebugQuadBuffer();
}

void RLL_LoadFullScreenDebugShader()
{
	const int maxShaderTextLength = 1024;
	char fragmentShaderPath[1024];
	char vertexShaderPath[1024];
	DynamicBuffer fragmentShaderBuffer;
	DynamicBuffer vertexShaderBuffer;

	// Read fragment shader
	if (Assets_PathFor("shaders/uv_as_red_green.frag", fragmentShaderPath, maxShaderTextLength)) {
		if (SPA_IsFile(fragmentShaderPath)) {
			KN_TRACE(LogSysRender, "\e[32m" "%s found" "\e[39m", fragmentShaderPath);
		}
		else {
			KN_TRACE(LogSysRender, "%s not found", fragmentShaderPath);
		}
	}

	if (!Assets_ReadFile(fragmentShaderPath, KN_FILE_TYPE_TEXT, &fragmentShaderBuffer)) {
		KN_ERROR(LogSysRender, "Unable to read fragment shader text");
	}

	// Read vertex shader
	if (Assets_PathFor("shaders/fullscreen_textured_quad.vert", vertexShaderPath, maxShaderTextLength)) {
		if (SPA_IsFile(vertexShaderPath)) {
			KN_TRACE(LogSysRender, "\e[32m" "%s found" "\e[39m", vertexShaderPath);
		}
		else {
			KN_TRACE(LogSysRender, "%s not found", vertexShaderPath);
		}
	}

	if (!Assets_ReadFile(vertexShaderPath, KN_FILE_TYPE_TEXT, &vertexShaderBuffer)) {
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

	RLL_CreateProgram(vertexShader, fragmentShader, &fullScreenDebugProgram);
	Mem_Free(&vertexShaderBuffer);
	Mem_Free(&fragmentShaderBuffer);
}

void RLL_LoadSolidPolygonShader()
{
	const int maxShaderTextLength = 1024;
	char fragmentShaderPath[1024];
	char vertexShaderPath[1024];
	DynamicBuffer fragmentShaderBuffer;
	DynamicBuffer vertexShaderBuffer;

	// Read fragment shader
	if (Assets_PathFor("shaders/solid_polygon.frag", fragmentShaderPath, maxShaderTextLength)) {
		if (SPA_IsFile(fragmentShaderPath)) {
			KN_TRACE(LogSysRender, "\e[32m" "%s found" "\e[39m", fragmentShaderPath);
		}
		else {
			KN_TRACE(LogSysRender, "%s not found", fragmentShaderPath);
		}
	}

	if (!Assets_ReadFile(fragmentShaderPath, KN_FILE_TYPE_TEXT, &fragmentShaderBuffer)) {
		KN_ERROR(LogSysRender, "Unable to read fragment shader text");
	}

	// Read vertex shader
	if (Assets_PathFor("shaders/solid_polygon.vert", vertexShaderPath, maxShaderTextLength)) {
		if (SPA_IsFile(vertexShaderPath)) {
			KN_TRACE(LogSysRender, "\e[32m" "%s found" "\e[39m", vertexShaderPath);
		}
		else {
			KN_TRACE(LogSysRender, "%s not found", vertexShaderPath);
		}
	}

	if (!Assets_ReadFile(vertexShaderPath, KN_FILE_TYPE_TEXT, &vertexShaderBuffer)) {
		KN_ERROR(LogSysRender, "Unable to read vertex shader text");
	}

	KN_TRACE(LogSysRender, "Fragment shader %s", fragmentShaderBuffer.contents);
	KN_TRACE(LogSysRender, "Vertex shader %s", vertexShaderBuffer.contents);

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

	RLL_CreateProgram(vertexShader, fragmentShader, &solidPolygonProgram);
	Mem_Free(&vertexShaderBuffer);
	Mem_Free(&fragmentShaderBuffer);
}

void RLL_LoadShaders()
{
	RLL_LoadSolidPolygonShader();
	RLL_LoadFullScreenDebugShader();
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
	return linkResult == GL_TRUE;
}

void RLL_Init(const uint32_t width, const uint32_t height)
{
	RLL_InitGL();
	RLL_ConfigureVSync();
	RLL_InitDummyVAO();
	RLL_FillBuffers();
	RLL_LoadShaders();

	windowWidth = (GLsizei)width;
	windowHeight = (GLsizei)height;
	projection = RLL_OrthoProjection(width, height);
}

void RLL_StartFrame(void)
{
	SDL_GL_MakeCurrent(window, gl);
	spritesUsed = 0;
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
void RLL_DrawDebugRect(const float4 position, const dimension2f dimensions,
	const float4 color)
{
	RLL_SetFullScreenViewport();

	glUseProgram(solidPolygonProgram);
	glBindBuffer(GL_ARRAY_BUFFER, debugQuadBuffer);

	const GLint uniformProjection = glGetUniformLocation(solidPolygonProgram, "Projection");
	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, &projection.m[0][0]);

	const float4x4 identity = float4x4_Identity();
	const GLint uniformViewModel = glGetUniformLocation(solidPolygonProgram, "ViewModel");
	glUniformMatrix4fv(uniformViewModel, 1, GL_FALSE, &identity.m[0][0]);

	const GLint uniformColor = glGetUniformLocation(solidPolygonProgram, "PolygonColor");
	glUniform4f(uniformColor, color.x, color.y, color.z, color.w);

	const GLint positionAttrib = glGetAttribLocation(solidPolygonProgram, "Position");
	KN_ASSERT(positionAttrib >= 0, "Position attribute does not exist");
	glEnableVertexAttribArray((GLuint)positionAttrib);
	KN_ASSERT_NO_GL_ERROR();
	glVertexAttribPointer(
		(GLuint)positionAttrib,
		4,
		GL_FLOAT,
		GL_FALSE,
		4 * sizeof(float),
		(void *)0
	);

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

	static uint32_t flag = 0;
	if (!flag) {
		for (uint32_t i = 0; i < 4; ++i) {
			float4_DebugPrint(stdout, vertices[i]);
		}

		printf("Transformed:\n");
		for (uint32_t i = 0; i < 4; ++i) {
			float4_DebugPrint(stdout, float4_Multiply(vertices[i], projection));
		}
		flag = 1;
		printf("\n");
	}

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(positionAttrib);

	KN_ASSERT_NO_GL_ERROR();
}