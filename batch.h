#ifndef _BATCH_H_
#define _BATCH_H_

#include <stdint.h>

#include <SDL2/SDL.h>
#include <glad/glad.h>

#include "render_common.h"

class Batch {
public:
	Batch(uint32_t numVerts, uint32_t numIndices);
	~Batch();

	void Add(Vertex* vertices, const uint32_t numVerts, uint32_t* indices, const uint32_t numIndices);
	void Bind();
	void Kill();

	uint32_t VertCount() const {
		return m_VertOffsetIndex;
	}
	uint32_t IndexCount() const {
		return m_IndexOffsetIndex;
	}


private:
	uint32_t m_NumVerts;
	uint32_t m_NumIndices;

	uint32_t m_VertOffsetIndex;
	uint32_t m_IndexOffsetIndex;

	GLuint m_VertexBuffer;
	GLuint m_IndexBuffer;
	GLuint m_VertexArray;
};

#endif

