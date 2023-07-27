#pragma once

#include "vulkan_headers.h"
#include "vulkan_types.h"

#include "orion-renderapi/descriptor.h"
#include "orion-renderapi/handles.h"

#include "orion-utils/assertion.h"

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
        using vulkan_handle_type = typename resource_type::pointer;

        struct ResourceData {
            store_data_type data;
            resource_type resource;
        };

        void add(handle_type handle, resource_type resource, store_data_type data = {})
        {
            auto [_, success] = resources_.insert(std::make_pair(handle, ResourceData{std::move(data), std::move(resource)}));
            ORION_ENSURES(success);
        }

        void add_or_assign(handle_type handle, resource_type resource, store_data_type data = {})
        {
            resources_.insert_or_assign(handle, ResourceData{std::move(data), std::move(resource)});
        }

        void set_resource(handle_type handle, resource_type resource)
        {
            resources_.at(handle).resource = std::move(resource);
        }

        void set_data(handle_type handle, store_data_type data)
        {
            resources_.at(handle).data = std::move(data);
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

        auto handle_or_null(handle_type handle) -> vulkan_handle_type
        {
            if (auto iter = resources_.find(handle); iter != resources_.end()) {
                return iter->second.resource.get();
            }
            return VK_NULL_HANDLE;
        }
        auto handle_or_null(handle_type handle) const -> vulkan_handle_type
        {
            if (auto iter = resources_.find(handle); iter != resources_.end()) {
                return iter->second.resource.get();
            }
            return VK_NULL_HANDLE;
        }

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
