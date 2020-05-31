#ifndef CN_RENDER_LL_GL_H
#define CN_RENDER_LL_GL_H

#include <calendon/cn.h>

#include <calendon/compat-gl.h>
#include <calendon/render-ll.h>

/**
 * Limit the number of vertex format attributes to something reasonable for a 2D
 * game engine.
 */
#define CN_RLL_MAX_VERTEX_FORMAT_ATTRIBUTES 16

/**
 * Define the format, amount, location, layout and transformation needed to
 * understand the format of one attribute of a vertex.
 */
typedef struct {
// "What is this thing is supposed to represent?"

	/**
	 * Abstract name of the type of this data.  It is positional data, texture
	 * coordinates, or something else?
	 *
	 * If no appropriate semantic name is assigned, then that means this is
	 * invalid data.
	 */
	uint32_t semanticName;

// Element format.

	/**
	 * The format type of each component.
	 *
	 * e.g. GL_BYTE, GL_FLOAT, GL_INT
	 */
	GLenum componentType;

	/**
	 * A vertex attribute might have multiple components.
	 *
	 * e.g. a 2D position would be 2 (x,y), a 3D position would be 3 (x,y,z),
	 * 2D texture coordinates would be 2 (u, v).
	 */
	uint8_t numComponents;

// Structure, layout and location of the array of data.

	/**
	 * The distance from the start of one attribute to the the start of the
	 * next same attribute.  0 is a special value here, meaning "I'm too lazy to count
	 * the size of each vertex, the values immediately follow each other."
	 */
	uint32_t stride;

	/**
	 * Is this value stored in an integer format and need to be transformed to
	 * an appropriate floating point range [-1, 1] for signed, [0, 1] for
	 * unsigned?
	 */
	bool normalized;

	/**
	 * The byte offset from the start of overall vertex data to the start of the
	 * first value of the attribute.
	 */
	size_t offset;
} CnVertexFormatAttribute;

/**
 * Vertex data is just a blob of binary data unless you know the format of it
 * and how to interpret it.
 *
 * Vertices might have any of a number of sorts of data assigned to each vertex,
 * such as a position, normal vector, or 2D texture coordinates.  A shader
 * program might or might not use each attribute in a set of vertex data, but it
 * needs to know how to use the attributes which are available.
 */
typedef struct {
	/**
	 * To speed lookup and application of vertex formats, attributes are indexed
	 * according to semantic name.
	 */
	CnVertexFormatAttribute attributes[CN_RLL_MAX_VERTEX_FORMAT_ATTRIBUTES];
} CnVertexFormat;

/**
 * Support a limited number of vertex attributes to maintain the best
 * compatibility.
 */
#define CN_RLL_MAX_ATTRIBUTES 8
CN_STATIC_ASSERT(CN_RLL_MAX_ATTRIBUTES <= GL_MAX_VERTEX_ATTRIBS,
	"RLL supports more active attributes than the API allows");
#define CN_RLL_MAX_UNIFORMS 32

/*
 * Statically define the maximum attribute name length to prevent from having to
 * dynamically allocate memory for attribute or uniform names.  The flexibility
 * lost in name length is more than made up for in simplicity.  The value used
 * is a balance between allowing reasonably sized names and not using excessive
 * amounts of storage.
 */
#define CN_RLL_MAX_ATTRIBUTE_NAME_LENGTH 64
#define CN_RLL_MAX_UNIFORM_NAME_LENGTH 64

/**
 * Attributes determine which data gets sent to the vertex shader.  Caching the
 * attributes used by a program prevent from having to query for it every time
 * that program is used.
 *
 * Normalized attributed are not currently supported.
 */
typedef struct {
	/**
	 * The name of the attribute as it appears in the shader source.
	 */
	char name[CN_RLL_MAX_ATTRIBUTE_NAME_LENGTH];

	/** Corresponding semantic type to use at this index. */
	uint32_t semanticName;

	/**
	 * The layout location of where the attribute will go.  This is in
	 * relation to the shader, and unrelated to the vertex format.
	 */
	GLint location;

	/**
	 * The size of the attribute, in terms of the given type.
	 */
	GLint size;

	/**
	 * Not necessarily the actual type to use for the vertex pointer.  Note that
	 * the type returned by glGetActiveAttrib IS NOT the same type as used by
	 * glVertexAttribPointer.
	 *
	 * e.g. `size` might be 1 and `type` `GL_FLOAT_VEC4` whereas
	 * `glVertexAttribPointer` is expecting to be given 4 and type `GL_FLOAT`.
	 */
	GLenum type;
} CnAttribute;

/**
 * Uniforms used during the vertex or fragment shaders.
 */
typedef struct {
	/**
	 * The name of the uniform, as it appears in the shader source.
	 */
	char name[CN_RLL_MAX_UNIFORM_NAME_LENGTH];

	/**
	 * The location to apply the uniform at, as returned by `glGetActiveUniform`.
	 */
	GLuint location;

	/**
	 * Points to the storage used to apply this uniform.
	 */
	uint32_t storageLocation;
	GLint size;
	GLenum type;
} CnUniform;

/**
 * Cache attribute and uniform locations and types used by a program to make
 * setting these things faster without needing lookups.
 */
typedef struct {
	GLuint id;
	CnAttribute attributes[CN_RLL_MAX_ATTRIBUTES];
	CnUniform uniforms[CN_RLL_MAX_UNIFORMS];
	uint32_t numAttributes;
	uint32_t numUniforms;
} CnProgram;

#endif /* CN_RENDER_LL_GL_H */
