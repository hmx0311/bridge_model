#include "GPUTripleBuffer.h"

std::vector<GPUTripleBuffer*> s_buffers;

void GPUTripleBuffer::Swap()
{
	for (GPUTripleBuffer* buffer : s_buffers)
	{
		buffer->swap();
	}
}

void GPUTripleBuffer::create(GLsizeiptr size)
{
	if (m_size != 0 || size == 0)
	{
		return;
	}
	glCreateBuffers(3, m_buffers);
	for (int i = 0; i < 3; i++)
	{
		glNamedBufferStorage(m_buffers[i], size, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		m_mapped_data[i] = glMapNamedBufferRange(m_buffers[i], 0, size, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	}
	m_size = size;
	s_buffers.push_back(this);
}

void GPUTripleBuffer::bindRange(GLenum target, GLuint binding_point, GLintptr offset, GLsizeiptr size)
{
	m_bindings.push_back({ target, binding_point, offset, size });
	glBindBufferRange(target, binding_point, m_buffers[sm_current], offset, size);
}

void GPUTripleBuffer::bindRangeCurrent(GLenum target, GLuint binding_point, GLintptr offset, GLsizeiptr size) const
{
	glBindBufferRange(target, binding_point, m_buffers[sm_current], offset, size);
}

void GPUTripleBuffer::bindBase(GLenum target, GLuint binding_point)
{
	m_bindings.push_back({ target, binding_point, 0, 0 });
	glBindBufferBase(target, binding_point, m_buffers[sm_current]);
}

void GPUTripleBuffer::bindBaseCurrent(GLenum target, GLuint binding_point) const
{
	glBindBufferBase(target, binding_point, m_buffers[sm_current]);
}

void GPUTripleBuffer::bindSlot(GLenum target, int slot) const
{
	glBindBuffer(target, m_buffers[slot]);
}

void GPUTripleBuffer::bindCurrent(GLenum target) const
{
	glBindBuffer(target, m_buffers[sm_current]);
}

void GPUTripleBuffer::updateData(GLintptr offset, GLsizeiptr size, const void* data) const
{
	memcpy(static_cast<char*>(m_mapped_data[sm_current]) + offset, data, size);
}

void GPUTripleBuffer::swap()
{
	for (Binding& binding : m_bindings)
	{
		if (binding.size == 0)
		{
			glBindBufferBase(binding.target, binding.binding_point, m_buffers[sm_current]);
		}
		else
		{
			glBindBufferRange(binding.target, binding.binding_point, m_buffers[sm_current], binding.offset, binding.size);
		}
	}
}