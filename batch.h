#ifndef _BATCH_H_
#define _BATCH_H_

#include <stdint.h>

#include <SDL2/SDL.h>
#include <glad/glad.h>

class Batch {
public:
	Batch(uint32_t numVerts, uint32_t numIndices);
	~Batch() {}

private:
	uint32_t m_NumVerts;
	uint32_t m_NumIndices;

	GLuint m_VertexBuffer;
	GLuint m_IndexBuffer;
	GLuint m_VertexArray;
};

#endif

