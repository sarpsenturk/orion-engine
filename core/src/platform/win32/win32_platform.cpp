#include "orion-core/platform.h"

#include "orion-core/platform/win32/win32_platform.h"

namespace orion
{
    spdlog::logger* win32::logger()
    {
        static const auto win32_logger = create_logger("orion-win32", static_cast<spdlog::level::level_enum>(ORION_WIN32_LOG_LEVEL));
        return win32_logger.get();
    }

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

    std::string win32_format_last_error(DWORD last_error)
    {
        char* buffer = nullptr;
        auto result = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            last_error,
            0,
            (LPSTR)&buffer,
            0,
            nullptr);
        // TODO: Check if result is 0 recursively?

        // We have to do this dance since the buffer allocated with
        // LocalAlloc has to be freed with LocalFree
        try {
            std::string message(buffer, result);
            LocalFree(buffer);
            return message;
        } catch (...) {
            LocalFree(buffer);
            throw;
        }
    }
} // namespace orion
