#include "orion-core/platform.h"

#include "orion-core/platform/win32/win32_headers.h"

namespace orion
{
    ProcessorFeatureSet get_processor_features()
    {
        return {
            .sse = IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) != 0,
            .sse2 = IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != 0,
            .sse3 = IsProcessorFeaturePresent(PF_SSE3_INSTRUCTIONS_AVAILABLE) != 0,
            .sse4_1 = IsProcessorFeaturePresent(PF_SSE4_1_INSTRUCTIONS_AVAILABLE) != 0,
            .sse4_2 = IsProcessorFeaturePresent(PF_SSE4_2_INSTRUCTIONS_AVAILABLE) != 0,
            .avx = IsProcessorFeaturePresent(PF_AVX_INSTRUCTIONS_AVAILABLE) != 0,
            .avx2 = IsProcessorFeaturePresent(PF_AVX2_INSTRUCTIONS_AVAILABLE) != 0,
            .avx512 = IsProcessorFeaturePresent(PF_AVX512F_INSTRUCTIONS_AVAILABLE) != 0,
        };
    }
} // namespace orion
