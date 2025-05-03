// COMP710 GP Framework 2024
#ifndef __VERTEXTARRAY_H_
#define __VERTEXTARRAY_H_

class VertexArray
{
	// Member methods:
public:
	VertexArray(const float* pVertices, unsigned int numVertices, const unsigned int* pIndices, unsigned int numIndicies);
	~VertexArray();

	void SetActive();

	unsigned int GetNumVertices() const;
	unsigned int GetNumIndices() const;

protected:

private:
	VertexArray(const VertexArray& vertexArray);
	VertexArray& operator=(const VertexArray& vertexArray);

	// Member data:
public:

protected:
	unsigned int m_numVertices;
	unsigned int m_numIndices;

	unsigned int m_glVertexBuffer;
	unsigned int m_glIndexBuffer;
	unsigned int m_glVertexArray;

private:

};

#endif // __VERTEXTARRAY_H_