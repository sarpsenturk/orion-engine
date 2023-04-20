#include "orion-renderapi/buffer.h"

namespace orion
{
    GPUBuffer::GPUBuffer(GPUBufferHandle handle, GPUBufferDesc desc)
        : handle_(handle)
        , desc_(desc)
    {
    }
} // namespace orion
