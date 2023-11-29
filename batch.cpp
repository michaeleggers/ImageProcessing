#include "batch.h"
#include "render_common.h"

#include <SDL2/SDL.h>
#include <glad/glad.h>

Batch::Batch(uint32_t numVerts, uint32_t numIndices)
{
	m_NumVerts = numVerts;
	m_NumIndices = numIndices;

	m_VertOffsetIndex = 0;
	m_IndexOffsetIndex = 0;

	glGenVertexArrays(1, &m_VertexArray);
	glBindVertexArray(m_VertexArray);

	glGenBuffers(1, &m_VertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vertex), nullptr, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // pos
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(1); // color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)));
	glEnableVertexAttribArray(2); // uv
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(glm::vec3)));

	glGenBuffers(1, &m_IndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(uint32_t), nullptr, GL_STATIC_DRAW);
}

Batch::~Batch() {
	glDeleteBuffers(1, &m_VertexBuffer);
	glDeleteBuffers(1, &m_IndexBuffer);
	glDeleteVertexArrays(1, &m_VertexArray);
}

void Batch::Add(Vertex* vertices, const uint32_t numVerts, uint32_t* indices, const uint32_t numIndices)
{
	glBindVertexArray(m_VertexArray);
	
	//glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, m_VertOffsetIndex * sizeof(Vertex), numVerts * sizeof(Vertex), vertices);
	//glBindBuffer(GL_ARRAY_BUFFER, m_IndexBuffer);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, m_IndexOffsetIndex * sizeof(uint32_t), numIndices * sizeof(uint32_t), indices);	
	
	m_VertOffsetIndex += numVerts;
	m_IndexOffsetIndex += numIndices;
}

void Batch::Bind()
{
	glBindVertexArray(m_VertexArray);
}
