#include "GPUTripleVertexArray.h"

void GPUTripleVertexArray::create()
{
	glGenVertexArrays(3, m_VAOs);
}

void GPUTripleVertexArray::bindSlot(int slot) const
{
	glBindVertexArray(m_VAOs[slot]);
}

void GPUTripleVertexArray::bindCurrent() const
{
	glBindVertexArray(m_VAOs[sm_current]);
}
