#include "orion-renderer/texture.h"

#include "orion-renderapi/render_device.h"

#include "orion-utils/finally.h"

#include "stb_image.h"

#ifndef ORION_TEXTURE_MANAGER_LOG_LEVEL
    #define ORION_TEXTURE_MANAGER_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h>

// Issues:
//  This implementation is really basic.
//  We always create an rgba texture without asking the user.

namespace orion
{
    namespace
    {
        spdlog::logger* logger()
        {
            static const auto logger = create_logger("orion-texture", ORION_TEXTURE_MANAGER_LOG_LEVEL);
            return logger.get();
        }

        Format get_texture_format(const TextureDesc&)
        {
            return Format::B8G8R8A8_Srgb;
        }
    } // namespace

    Texture::Texture(UniqueImage image, UniqueImageView image_view)
        : image_(std::move(image))
        , image_view_(std::move(image_view))
    {
    }

    TextureLoader::TextureLoader(RenderDevice* device)
        : device_(device)
        , command_allocator_(device->create_command_allocator({.queue_type = CommandQueueType::Graphics, .reset_command_buffer = false})) // TODO: Use transfer queue
        , command_list_(command_allocator_->create_command_list())
    {
    }

    TextureLoadResult TextureLoader::load_from_memory(const TextureDesc& desc)
    {
        // Create staging buffer for image
        const auto image_size = desc.size_bytes();
        const auto buffer = device_->make_unique<GPUBufferHandle_tag>(GPUBufferDesc{
            .size = image_size,
            .usage = GPUBufferUsageFlags::TransferSrc,
            .host_visible = true,
        });

        // Upload image data to staging buffer
        void* ptr = device_->map(buffer.get());
        std::memcpy(ptr, desc.data, image_size);
        device_->unmap(buffer.get());

        // Create the image itself
        const auto image_extent = Vector{static_cast<std::uint32_t>(desc.width), static_cast<std::uint32_t>(desc.height), 1u};
        const auto image_format = get_texture_format(desc);
        const auto image = device_->create_image({
            .type = ImageType::Image2D,
            .format = image_format,
            .size = image_extent,
            .tiling = ImageTiling::Optimal,
            .usage = ImageUsageFlags::TransferDst | ImageUsageFlags::SampledImage,
            .host_visible = false,
        });

        // Transfer image data from staging buffer to image
        command_allocator_->reset();
        command_list_->begin();
        command_list_->transition_barrier({.image = image, .old_layout = ImageLayout::Undefined, .new_layout = ImageLayout::TransferDst});
        command_list_->copy_buffer_to_image({
            .src_buffer = buffer.get(),
            .dst_image = image,
            .dst_layout = ImageLayout::TransferDst,
            .buffer_offset = 0,
            .image_offset = 0,
            .dst_size = image_extent,
        });
        command_list_->transition_barrier({.image = image, .old_layout = ImageLayout::TransferDst, .new_layout = ImageLayout::ShaderReadOnly});
        command_list_->end();
        device_->submit_immediate({.queue_type = CommandQueueType::Graphics, .command_lists = {{command_list_.get()}}});

        // Create image view
        const auto image_view = device_->create_image_view({
            .image = image,
            .type = ImageViewType::View2D,
            .format = image_format,
        });

        return Texture{device_->to_unique(image), device_->to_unique(image_view)};
    }

    TextureLoadResult TextureLoader::load_from_file(const FilePath& path)
    {
        SPDLOG_LOGGER_TRACE(logger(), "Creating texture from file: {}...", path);
        if (!fs::exists(path)) {
            SPDLOG_LOGGER_ERROR(logger(), "File does not exist");
            return unexpected{"texture file does not exist"};
        }

        int width, height, real_channels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &real_channels, STBI_rgb_alpha);
        if (data == nullptr) {
            const char* fail_reason = stbi_failure_reason();
            SPDLOG_LOGGER_ERROR(logger(), "stbi_load failed: {}", fail_reason);
            return unexpected{fail_reason};
        }
        const auto stbi_free = finally([data]() { stbi_image_free(data); });

        SPDLOG_LOGGER_TRACE(logger(), "width: {}, height: {}, channels: {}", width, height, real_channels);

        // TODO: Do proper handling of channels
        const auto channels = 4;
        return load_from_memory({data, width, height, channels});
    }

    TextureArray::TextureArray(RenderDevice* device)
        : device_(device)
        , descriptor_pool_(create_descriptor_pool())
        , descriptor_layout_(create_descriptor_layout())
        , descriptor_(create_descriptor())
        , default_sampler_(create_default_sampler())
    {
        set_sampler_default();
    }

    texture_index_t TextureArray::add(const Texture& texture)
    {
        const auto index = texture_index_++;
        set(index, texture);
        return index;
    }

    void TextureArray::set_all(const Texture& texture)
    {
        std::vector<ImageDescriptorDesc> image_writes(max_textures);
        for (auto& image : image_writes) {
            image.image_view_handle = texture.image_view();
            image.image_layout = ImageLayout::ShaderReadOnly;
        }
        const auto write = DescriptorWrite{
            .binding = texture_binding,
            .descriptor_type = DescriptorType::SampledImage,
            .array_start = 0,
            .images = image_writes,
        };
        device_->write_descriptor(descriptor_, write);
    }

    void TextureArray::set_sampler(SamplerHandle sampler_handle)
    {
        const auto sampler_descriptor = ImageDescriptorDesc{.sampler_handle = sampler_handle};
        const auto descriptor_write = DescriptorWrite{
            .binding = sampler_binding,
            .descriptor_type = DescriptorType::Sampler,
            .images = {&sampler_descriptor, 1},
        };
        device_->write_descriptor(descriptor_, descriptor_write);
    }

    void TextureArray::set_sampler_default()
    {
        set_sampler(default_sampler_.get());
    }

    UniqueDescriptorPool TextureArray::create_descriptor_pool() const
    {
        return device_->make_unique<DescriptorPoolHandle_tag>(DescriptorPoolDesc{
            .max_descriptors = 1,
            .flags = {},
            .sizes = {{
                DescriptorPoolSize{
                    .type = DescriptorType::Sampler,
                    .count = max_samplers,
                },
                DescriptorPoolSize{
                    .type = DescriptorType::SampledImage,
                    .count = max_textures,
                },
            }},
        });
    }

    UniqueDescriptorLayout TextureArray::create_descriptor_layout() const
    {
        return device_->make_unique<DescriptorLayoutHandle_tag>(DescriptorLayoutDesc{
            .bindings = {{
                DescriptorBindingDesc{
                    .type = DescriptorType::Sampler,
                    .shader_stages = ShaderStageFlags::All,
                    .count = 1,
                },
                DescriptorBindingDesc{
                    .type = DescriptorType::SampledImage,
                    .shader_stages = ShaderStageFlags::All,
                    .count = max_textures,
                },
            }},
        });
    }

    DescriptorHandle TextureArray::create_descriptor() const
    {
        return device_->create_descriptor(descriptor_layout_.get(), descriptor_pool_.get());
    }

    UniqueSampler TextureArray::create_default_sampler() const
    {
        return device_->make_unique<SamplerHandle_tag>(SamplerDesc{
            .filter = Filter::Nearest,
            .address_mode_u = AddressMode::Repeat,
            .address_mode_v = AddressMode::Repeat,
            .address_mode_w = AddressMode::Repeat,
            .mip_load_bias = 0.f,
            .max_anisotropy = 1.f,
            .compare_func = CompareFunc::Never,
            .min_lod = 0.f,
            .max_lod = 0.f,
        });
    }

    void TextureArray::set(texture_index_t index, const Texture& texture)
    {
        const auto image_descriptor = ImageDescriptorDesc{
            .image_view_handle = texture.image_view(),
            .image_layout = ImageLayout::ShaderReadOnly,
        };
        const auto descriptor_write = DescriptorWrite{
            .binding = texture_binding,
            .descriptor_type = DescriptorType::SampledImage,
            .array_start = static_cast<std::uint32_t>(index),
            .images = {&image_descriptor, 1},
        };
        device_->write_descriptor(descriptor_, descriptor_write);
    }
} // namespace orion
