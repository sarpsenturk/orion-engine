#pragma once

#include "vulkan_buffer.h"
#include "vulkan_headers.h"
#include "vulkan_pipeline.h"
#include "vulkan_types.h"

#include "orion-renderapi/descriptor.h"
#include "orion-renderapi/handles.h"

#include <memory>

namespace orion::vulkan
{
    template<typename HandleType, typename ResourceType>
    class VulkanStore
    {
    public:
        using handle_type = HandleType;
        using resource_type = ResourceType;

        void add(handle_type handle, resource_type resource)
        {
            resources_.insert(std::make_pair(handle, std::move(resource)));
        }

        void add_or_assign(handle_type handle, resource_type resource)
        {
            resources_.insert_or_assign(handle, std::move(resource));
        }

        void remove(handle_type handle)
        {
            resources_.erase(handle);
        }

        auto find(handle_type handle) { return resources_.find(handle); }
        auto find(handle_type handle) const { return resources_.find(handle); }

        auto at(handle_type handle) { return resources_.at(handle).get(); }
        auto at(handle_type handle) const { return resources_.at(handle).get(); }

        auto begin() { return resources_.begin(); }
        auto begin() const { return resources_.begin(); }
        auto end() { return resources_.end(); }
        auto end() const { return resources_.end(); }

    private:
        std::unordered_map<handle_type, resource_type> resources_;
    };
} // namespace orion::vulkan
