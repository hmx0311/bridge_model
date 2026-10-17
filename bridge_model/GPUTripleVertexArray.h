#pragma once
#include "GPUTriple.h"
#include "glad/glad.h"
class GPUTripleVertexArray :
    public GPUTriple
{
    friend void GPUTriple::Swap();

private:
    GLuint m_VAOs[3]{};

public:
    void create();
    void bindSlot(int slot) const;
    void bindCurrent() const;
};

