#include "GPUTriple.h"
#include "GPUTripleBuffer.h"

static GLsync s_fences[3]{};

int GPUTriple::sm_current = 0;

void GPUTriple::Swap()
{
	sm_current = (sm_current + 1) % 3; 
    if (s_fences[sm_current] != nullptr)
    {
        glClientWaitSync(s_fences[sm_current], GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
        glDeleteSync(s_fences[sm_current]);
        s_fences[sm_current] = nullptr;
    }
	GPUTripleBuffer::Swap();
}

void GPUTriple::Lock()
{
	s_fences[sm_current] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}
