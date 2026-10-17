#pragma once
#include "GPUTriple.h"
#include <vector>
#include "glad/glad.h"

class GPUTripleBuffer :
	public GPUTriple
{
	friend void GPUTriple::Swap();

private:
	struct Binding
	{
		GLenum target;
		GLuint binding_point;
		GLintptr offset;
		GLsizeiptr size;
	};

	GLuint m_buffers[3]{};
	void* m_mapped_data[3]{};
	GLsizeiptr m_size = 0;
	std::vector<Binding> m_bindings;

	static void Swap();

public:
	void create(GLsizeiptr size);
	void bindRange(GLenum target, GLuint binding_point, GLintptr offset, GLsizeiptr size);
	void bindRangeCurrent(GLenum target, GLuint binding_point, GLintptr offset, GLsizeiptr size) const;
	void bindBase(GLenum target, GLuint binding_point);
	void bindBaseCurrent(GLenum target, GLuint binding_point) const;
	void bindSlot(GLenum target, int slot) const;
	void bindCurrent(GLenum target) const;
	void updateData(GLintptr offset, GLsizeiptr size, const void* data) const;

private:
	void swap();
};

