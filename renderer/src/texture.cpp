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
        Format get_texture_format(const TextureInfo& info)
        {
            return Format::B8G8R8A8_Srgb;
        }
    } // namespace

    Texture::Texture(UniqueImage image, UniqueImageView image_view, TextureInfo info)
        : image_(std::move(image))
        , image_view_(std::move(image_view))
        , info_(info)
    {
    }

    TextureManager::TextureManager(RenderDevice* device)
        : device_(device)
        , descriptor_pool_(create_descriptor_pool())
        , descriptor_layout_(create_descriptor_layout())
        , descriptor_(create_descriptor())
        , command_allocator_(device->create_command_allocator({.queue_type = CommandQueueType::Graphics, .reset_command_buffer = false}))
        , command_list_(command_allocator_->create_command_list())
        , default_sampler_(create_default_sampler())
    {
        // Create 1x1 missing texture
        const auto missing_data = std::array{
            std::uint8_t{0x00},
            std::uint8_t{0x00},
            std::uint8_t{0x00},
            std::uint8_t{0xFF},
            std::uint8_t{0xFC},
            std::uint8_t{0x00},
            std::uint8_t{0xFF},
            std::uint8_t{0xFF},
            std::uint8_t{0xFC},
            std::uint8_t{0x00},
            std::uint8_t{0xFF},
            std::uint8_t{0xFF},
            std::uint8_t{0x00},
            std::uint8_t{0x00},
            std::uint8_t{0x00},
            std::uint8_t{0xFF},
        };
        const auto& missing = textures_.emplace_back(make_texture(missing_data.data(), {.width = 2, .height = 2, .channels = 4}));
        // Set all textures to missing initially
        {
            std::vector<ImageDescriptorDesc> image_writes(max_textures);
            for (auto& image : image_writes) {
                image.image_view_handle = missing.image_view();
                image.image_layout = ImageLayout::ShaderReadOnly;
            }
            const auto write = DescriptorWrite{
                .binding = texture_binding,
                .descriptor_type = DescriptorType::SampledImage,
                .array_start = 0,
                .images = image_writes,
            };
            device->write_descriptor(descriptor_, write);
        }

        // Add 1x1 white texture
        const auto white = std::array{std::uint8_t{0xFF}, std::uint8_t{0xFF}, std::uint8_t{0xFF}, std::uint8_t{0xFF}};
        add_texture(white.data(), TextureInfo{.width = 1, .height = 1, .channels = 4});

        // Set sampler to default sampler
        set_sampler_default();
    }

    TextureCreateResult TextureManager::add_texture(const unsigned char* data, const TextureInfo& info)
    {
        const auto index = static_cast<texture_index_t>(textures_.size());
        const auto* texture = &(textures_.emplace_back(make_texture(data, info)));
        set_texture(index, *texture);
        return TextureCreateSuccess{texture, index};
    }

    TextureCreateResult TextureManager::load_from_file(const FilePath& path)
    {
        SPDLOG_LOGGER_TRACE(logger(), "Creating texture from file: {}...", path);
        if (!fs::exists(path)) {
            SPDLOG_LOGGER_ERROR(logger(), "File does not exist");
            return unexpected{"texture file does not exist"};
        }

        int width, height, channels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (data == nullptr) {
            const char* fail_reason = stbi_failure_reason();
            SPDLOG_LOGGER_ERROR(logger(), "stbi_load failed: {}", fail_reason);
            return unexpected{fail_reason};
        }
        const auto stbi_free = finally([data]() { stbi_image_free(data); });

        SPDLOG_LOGGER_TRACE(logger(), "width: {}, height: {}, channels: {}", width, height, channels);

        // TODO: Do proper handling of channels
        return add_texture(data, {width, height, 4});
    }

    const Texture* TextureManager::get(texture_index_t index) const
    {
        ORION_ASSERT(index >= 0 && index < max_textures);
        return &(textures_[index]);
    }

    void TextureManager::set_sampler(SamplerHandle sampler_handle)
    {
        const auto sampler_descriptor = ImageDescriptorDesc{.sampler_handle = sampler_handle};
        const auto descriptor_write = DescriptorWrite{
            .binding = sampler_binding,
            .descriptor_type = DescriptorType::Sampler,
            .images = {&sampler_descriptor, 1},
        };
        device_->write_descriptor(descriptor_, descriptor_write);
    }

    void TextureManager::set_sampler_default()
    {
        set_sampler(default_sampler_.get());
    }

    UniqueDescriptorPool TextureManager::create_descriptor_pool() const
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

    UniqueDescriptorLayout TextureManager::create_descriptor_layout() const
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

    DescriptorHandle TextureManager::create_descriptor() const
    {
        return device_->create_descriptor(descriptor_layout_.get(), descriptor_pool_.get());
    }

    UniqueSampler TextureManager::create_default_sampler() const
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

    Texture TextureManager::make_texture(const unsigned char* data, TextureInfo info)
    {
        // Create staging buffer for image
        const auto image_size = info.size_bytes();
        const auto buffer = device_->make_unique<GPUBufferHandle_tag>(GPUBufferDesc{
            .size = image_size,
            .usage = GPUBufferUsageFlags::TransferSrc,
            .host_visible = true,
        });

        // Upload image data to staging buffer
        void* ptr = device_->map(buffer.get());
        std::memcpy(ptr, data, image_size);
        device_->unmap(buffer.get());

        // Create the image itself
        const auto image_extent = Vector{static_cast<std::uint32_t>(info.width), static_cast<std::uint32_t>(info.height), 1u};
        const auto image_format = get_texture_format(info);
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

        return Texture{device_->to_unique(image), device_->to_unique(image_view), info};
    }

    void TextureManager::set_texture(texture_index_t index, const Texture& texture)
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

    spdlog::logger* TextureManager::logger()
    {
        static const auto logger = create_logger("orion-texture", ORION_TEXTURE_MANAGER_LOG_LEVEL);
        return logger.get();
    }
} // namespace orion
