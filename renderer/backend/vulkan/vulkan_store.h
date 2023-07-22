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
    struct DefaultStoreData {
    };

    template<typename HandleType, typename ResourceType, typename StoreData = DefaultStoreData>
    class VulkanStore
    {
    public:
        using handle_type = HandleType;
        using resource_type = ResourceType;
        using store_data_type = StoreData;

        struct ResourceData {
            resource_type resource;
            store_data_type data;
        };

        void add(handle_type handle, resource_type resource, store_data_type data = {})
        {
            resources_.insert(std::make_pair(handle, ResourceData{std::move(resource), std::move(data)}));
        }

        void add_or_assign(handle_type handle, resource_type resource, store_data_type data = {})
        {
            resources_.insert_or_assign(handle, ResourceData{std::move(resource), std::move(data)});
        }

        void remove(handle_type handle)
        {
            resources_.erase(handle);
        }

        auto find(handle_type handle) { return resources_.find(handle); }
        auto find(handle_type handle) const { return resources_.find(handle); }

        auto& at(handle_type handle) { return resources_.at(handle); }
        auto& at(handle_type handle) const { return resources_.at(handle); }

        auto handle_at(handle_type handle) { return resources_.at(handle).resource.get(); }
        auto handle_at(handle_type handle) const { return resources_.at(handle).resource.get(); }

        auto& data_at(handle_type handle) { return resources_.at(handle).data; }
        auto& data_at(handle_type handle) const { return resources_.at(handle).data; }

        auto begin() { return resources_.begin(); }
        auto begin() const { return resources_.begin(); }
        auto end() { return resources_.end(); }
        auto end() const { return resources_.end(); }

    private:
        std::unordered_map<handle_type, ResourceData> resources_;
    };
} // namespace orion::vulkan
