#include "orion-renderapi/types.h"

#include "orion-utils/assertion.h"
#include "orion-utils/type.h"

#include <numeric>

namespace orion
{
    std::string orion::format_as(ShaderStageFlags shader_stages)
    {
        if (!shader_stages) {
            return {};
        }

        std::string result{};
        for (auto stage : BitwiseRange{shader_stages}) {
            if (!stage) {
                continue;
            }
            switch (stage) {
                case ShaderStageFlags::Vertex:
                    result += "Vertex | ";
                    break;
                case ShaderStageFlags::Pixel:
                    result += "Pixel | ";
                    break;
            }
        }
        return result.substr(0, result.size() - 3);
    }
} // namespace orion
