#include <orion/orion.hpp>

#include <orion/core/logger.hpp>
#include <orion/core/window.hpp>
#include <orion/imgui/imgui.hpp>
#include <orion/loaders/texture.hpp>
#include <orion/renderer/renderer.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <filesystem>
#include <numeric>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

class Sandbox : public orion::Application
{
public:
    explicit Sandbox(orion::Engine& engine, fastgltf::Asset asset, std::filesystem::path asset_directory)
        : m_window(orion::WindowDesc{"Sandbox", 1'280, 720})
        , m_renderer(engine.renderer())
        , m_swapchain(m_renderer.create_swapchain(m_window))
        , m_imgui(m_renderer, m_window)
        , m_asset(std::move(asset))
        , m_asset_directory(std::move(asset_directory))
        , m_default_material(m_renderer.create_material({}))
    {
        load_materials();
        load_meshes();
        load_scene();
        m_window.set_cursor_mode(orion::CursorMode::Disabled);
        m_last_mouse_position = m_window.mouse_position();
    }

    ~Sandbox() override
    {
        for (const auto& primitives : m_meshes) {
            for (const auto& primitive : primitives) { m_renderer.destroy_mesh(primitive.mesh); }
        }
        for (const auto material : m_materials) { m_renderer.destroy_material(material); }
        m_renderer.destroy_material(m_default_material);
        for (const auto& [image_index, texture] : m_textures) {
            if (texture.handle != orion::Texture{}.handle) { m_renderer.destroy_texture(texture); }
        }
    }

private:
    struct MeshPrimitive {
        orion::Mesh mesh;
        orion::Material material;
    };

    std::expected<orion::LoadImageResult, orion::LoadImageError> load_image(std::size_t image_index)
    {
        return std::visit([this](const auto& source) -> std::expected<orion::LoadImageResult, orion::LoadImageError> {
            using Source = std::decay_t<decltype(source)>;
            if constexpr (std::is_same_v<Source, fastgltf::sources::URI>) {
                if (!source.uri.isLocalPath() || source.fileByteOffset != 0) {
                    return std::unexpected(orion::LoadImageError::DecodeFailed);
                }
                return orion::load_image(m_asset_directory / source.uri.fspath());
            } else {
                std::span<const std::byte> bytes;
                if constexpr (std::is_same_v<Source, fastgltf::sources::BufferView>) {
                    const auto& view = m_asset.bufferViews[source.bufferViewIndex];
                    bytes = std::visit([](const auto& buffer) -> std::span<const std::byte> {
                        if constexpr (requires { buffer.bytes; }) {
                            return {buffer.bytes.data(), buffer.bytes.size()};
                        } else {
                            return {};
                        }
                    }, m_asset.buffers[view.bufferIndex].data);
                    if (view.byteOffset > bytes.size() || view.byteLength > bytes.size() - view.byteOffset) {
                        return std::unexpected(orion::LoadImageError::DecodeFailed);
                    }
                    bytes = bytes.subspan(view.byteOffset, view.byteLength);
                } else if constexpr (requires { source.bytes; }) {
                    // Includes data-URI images decoded to bytes by fastgltf.
                    bytes = {source.bytes.data(), source.bytes.size()};
                } else {
                    return std::unexpected(orion::LoadImageError::DecodeFailed);
                }
                return orion::decode_image({reinterpret_cast<const std::uint8_t*>(bytes.data()), bytes.size()});
            }
        }, m_asset.images[image_index].data);
    }

    orion::Texture load_texture(const fastgltf::TextureInfo& info)
    {
        if (info.texCoordIndex != 0 || info.transform) {
            ORION_LOG_WARN(orion::LogChannel::Application, "Skipping albedo texture with unsupported UV set or transform");
            return {};
        }
        const auto& texture = m_asset.textures[info.textureIndex];
        if (!texture.imageIndex.has_value()) {
            ORION_LOG_WARN(orion::LogChannel::Application, "Albedo texture {} has no supported image source", info.textureIndex);
            return {};
        }
        const auto image_index = texture.imageIndex.value();
        auto [entry, inserted] = m_textures.try_emplace(image_index);
        if (!inserted) { return entry->second; }

        const auto image = load_image(image_index);
        if (!image) {
            ORION_LOG_WARN(orion::LogChannel::Application, "Failed to load image {} (error {}); using base color only",
                          image_index, std::to_underlying(image.error()));
            return {};
        }
        entry->second = m_renderer.create_texture({
            .width = image->width,
            .height = image->height,
            .data = {image->pixels.data(), image->pixels.size()},
        });
        ORION_LOG_INFO(orion::LogChannel::Application, "Loaded albedo image {} ({} x {})", image_index, image->width, image->height);
        return entry->second;
    }

    void load_materials()
    {
        m_materials.reserve(m_asset.materials.size());
        for (const auto& material : m_asset.materials) {
            const auto& color = material.pbrData.baseColorFactor;
            m_materials.push_back(m_renderer.create_material({
                .base_color = glm::vec4(color[0], color[1], color[2], color[3]),
                .base_color_texture = material.pbrData.baseColorTexture.has_value()
                    ? load_texture(material.pbrData.baseColorTexture.value())
                    : orion::Texture{},
            }));
        }
    }

    void load_meshes()
    {
        m_meshes.resize(m_asset.meshes.size());
        for (std::size_t mesh_index = 0; mesh_index < m_asset.meshes.size(); ++mesh_index) {
            for (const auto& primitive : m_asset.meshes[mesh_index].primitives) {
                if (primitive.type != fastgltf::PrimitiveType::Triangles) {
                    ORION_LOG_WARN(orion::LogChannel::Application, "Skipping non-triangle primitive in mesh {}", mesh_index);
                    continue;
                }

                const auto position_attribute = primitive.findAttribute("POSITION");
                if (position_attribute == primitive.attributes.end()) {
                    ORION_LOG_WARN(orion::LogChannel::Application, "Skipping primitive without positions in mesh {}", mesh_index);
                    continue;
                }
                const auto& position_accessor = m_asset.accessors[position_attribute->accessorIndex];
                std::vector<glm::vec3> positions(position_accessor.count);
                fastgltf::copyFromAccessor<glm::vec3>(m_asset, position_accessor, positions.data());

                std::vector<glm::vec2> uvs;
                if (const auto attribute = primitive.findAttribute("TEXCOORD_0"); attribute != primitive.attributes.end()) {
                    const auto& accessor = m_asset.accessors[attribute->accessorIndex];
                    if (accessor.count != positions.size()) {
                        ORION_LOG_WARN(orion::LogChannel::Application, "Skipping primitive with mismatched UV count in mesh {}", mesh_index);
                        continue;
                    }
                    uvs.resize(accessor.count);
                    fastgltf::copyFromAccessor<glm::vec2>(m_asset, accessor, uvs.data());
                }
                
                std::vector<glm::vec3> normals;
                if (const auto attribute = primitive.findAttribute("NORMAL"); attribute != primitive.attributes.end()) {
                    const auto& accessor = m_asset.accessors[attribute->accessorIndex];
                    if (accessor.count != positions.size()) {
                        ORION_LOG_WARN(orion::LogChannel::Application,
                                       "Skipping primitive with mismatched NORMAL count in mesh {}",
                                       mesh_index);
                        continue;
                    }
                    normals.resize(accessor.count);
                    fastgltf::copyFromAccessor<glm::vec3>(m_asset, accessor, normals.data());
                }

                std::vector<std::uint32_t> indices;
                if (primitive.indicesAccessor.has_value()) {
                    const auto& accessor = m_asset.accessors[primitive.indicesAccessor.value()];
                    indices.resize(accessor.count);
                    fastgltf::copyFromAccessor<std::uint32_t>(m_asset, accessor, indices.data());
                } else {
                    indices.resize(positions.size());
                    std::iota(indices.begin(), indices.end(), 0u);
                }
                if (positions.empty() || indices.empty() || indices.size() % 3 != 0) {
                    ORION_LOG_WARN(orion::LogChannel::Application, "Skipping empty or incomplete triangle primitive in mesh {}", mesh_index);
                    continue;
                }
                if (std::ranges::any_of(indices, [&](auto index) { return index >= positions.size(); })) {
                    ORION_LOG_WARN(orion::LogChannel::Application, "Skipping primitive with out-of-range indices in mesh {}", mesh_index);
                    continue;
                }

                const auto mesh = m_renderer.create_mesh({
                    .positions = positions,
                    .uvs = uvs,
                    .normals = normals,
                    .indices = indices,
                    .vertex_count = static_cast<std::uint32_t>(positions.size()),
                });
                const auto material = primitive.materialIndex.has_value()
                    ? m_materials[primitive.materialIndex.value()]
                    : m_default_material;
                m_meshes[mesh_index].push_back({mesh, material});
            }
        }
    }

    void load_scene()
    {
        if (m_asset.scenes.empty()) {
            ORION_LOG_WARN(orion::LogChannel::Application, "Asset has no scenes to render");
            return;
        }
        const auto scene_index = m_asset.defaultScene.value_or(0);
        fastgltf::iterateSceneNodes(m_asset, scene_index, fastgltf::math::fmat4x4(),
            [this](const fastgltf::Node& node, const fastgltf::math::fmat4x4& world) {
                if (!node.meshIndex.has_value()) { return; }
                // Both matrix types use column-major storage; copy the accumulated world transform.
                glm::mat4 model(1.0f);
                for (auto column = 0; column < 4; ++column) {
                    for (auto row = 0; row < 4; ++row) { model[column][row] = world[column][row]; }
                }
                for (const auto& primitive : m_meshes[node.meshIndex.value()]) {
                    m_instances.push_back({primitive.mesh, primitive.material, model});
                }
            });
        ORION_LOG_INFO(orion::LogChannel::Application, "Created {} render instances from scene {}", m_instances.size(), scene_index);
    }

    void on_update(float delta_time) override
    {
        // Handle camera movement based on keyboard input
        float xdelta = 0.0f;
        if (m_window.is_key_pressed(orion::Keycode::A)) { xdelta -= 1.0f; }
        if (m_window.is_key_pressed(orion::Keycode::D)) { xdelta += 1.0f; }
        float ydelta = 0.0f;
        if (m_window.is_key_pressed(orion::Keycode::W)) { ydelta = 1.0f; }
        if (m_window.is_key_pressed(orion::Keycode::S)) { ydelta -= 1.0f; }

        // Handle camera rotation based on mouse movement
        if (m_cursor_locked) {
            const auto mouse_position = m_window.mouse_position();
            const auto mouse_delta    = mouse_position - m_last_mouse_position;
            m_last_mouse_position     = mouse_position;
            m_camera_pitch -= mouse_delta.y * m_camera_sensitivity;
            m_camera_yaw += mouse_delta.x * m_camera_sensitivity;
        }

        // Clamp pitch & wrap yaw
        m_camera_pitch = glm::clamp(m_camera_pitch, -89.0f, 89.0f);
        m_camera_yaw   = glm::mod(m_camera_yaw, 360.0f);

        // Calculate camera front & right vectors based on yaw and pitch
        m_camera_front.x = cos(glm::radians(m_camera_yaw)) * cos(glm::radians(m_camera_pitch));
        m_camera_front.y = sin(glm::radians(m_camera_pitch));
        m_camera_front.z = sin(glm::radians(m_camera_yaw)) * cos(glm::radians(m_camera_pitch));
        m_camera_front   = glm::normalize(m_camera_front);
        m_camera_right   = glm::normalize(glm::cross(m_camera_front, glm::vec3(0.0f, 1.0f, 0.0f)));
        m_camera_up      = glm::normalize(glm::cross(m_camera_right, m_camera_front));

        // Update camera position based on input
        m_camera_position += m_camera_right * xdelta * m_camera_speed * delta_time;
        m_camera_position += m_camera_front * ydelta * m_camera_speed * delta_time;
    }

    void on_event(const orion::Event& event) override
    {
        if (const auto keypress = std::get_if<orion::OnKeyPress>(&event)) {
            if (m_imgui.wants_capture_keyboard()) { return; }
            if (keypress->key == orion::Keycode::Escape) {
                m_window.set_cursor_mode(orion::CursorMode::Normal);
                m_cursor_locked = false;
            }
        }
        if (const auto mouse_click = std::get_if<orion::OnMouseButtonPress>(&event)) {
            if (m_imgui.wants_capture_mouse()) { return; }
            if (mouse_click->button == orion::MouseButton::Left && !m_cursor_locked) {
                m_window.set_cursor_mode(orion::CursorMode::Disabled);
                m_last_mouse_position = m_window.mouse_position();
                m_cursor_locked       = true;
            }
        }
    }

    void on_render() override
    {
        m_imgui.begin_frame();
        ImGui::Begin("Camera Properties");
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", m_camera_position.x, m_camera_position.y, m_camera_position.z);
        ImGui::Text("Front: (%.2f, %.2f, %.2f)", m_camera_front.x, m_camera_front.y, m_camera_front.z);
        ImGui::Text("Up: (%.2f, %.2f, %.2f)", m_camera_up.x, m_camera_up.y, m_camera_up.z);
        ImGui::Text("Right: (%.2f, %.2f, %.2f)", m_camera_right.x, m_camera_right.y, m_camera_right.z);
        ImGui::SliderFloat("Pitch", &m_camera_pitch, -89.0f, 89.0f);
        ImGui::SliderFloat("Yaw", &m_camera_yaw, -180.0f, 180.0f);
        ImGui::SliderFloat("Speed", &m_camera_speed, 0.1f, 10.0f);
        ImGui::SliderFloat("Sensitivity", &m_camera_sensitivity, 0.01f, 1.0f);
        ImGui::End();

        if (ImGui::Begin("Frame Stats")) {
            const auto& stats = m_renderer.frame_stats();
            ImGui::Text("Last completed frame: %" PRIu64, stats.frame_index);

            ImGui::SeparatorText("CPU timings");
            ImGui::Text("Frame wait: %.3f ms", stats.frame_wait_ms);
            ImGui::Text("Swapchain acquire: %.3f ms", stats.swapchain_acquire_ms);
            ImGui::Text("Scene recording: %.3f ms", stats.scene_record_ms);
            ImGui::Text("UI recording: %.3f ms", stats.ui_record_ms);
            ImGui::Text("Upload flush: %.3f ms", stats.upload_flush_ms);
            ImGui::Text("Command submit: %.3f ms", stats.command_submit_ms);
            ImGui::Text("Swapchain present: %.3f ms", stats.swapchain_present_ms);

            ImGui::SeparatorText("Scene geometry");
            ImGui::Text("Instances: %" PRIu32, stats.instance_count);
            ImGui::Text("Draw calls: %" PRIu32, stats.draw_calls);
            ImGui::Text("Indices: %" PRIu32, stats.index_count);
            ImGui::Text("Triangles: %" PRIu32, stats.triangle_count);
        }
        ImGui::End();

        if (ImGui::Begin("Renderer Stats")) {
            const auto stats = m_renderer.renderer_stats();
            ImGui::SeparatorText("Resource counts");
            ImGui::Text("Meshes: %" PRIu32, stats.mesh_count);
            ImGui::Text("Materials: %" PRIu32, stats.material_count);
            ImGui::Text("Textures: %" PRIu32, stats.texture_count);

            constexpr auto bytes_per_mib = 1024.0 * 1024.0;
            ImGui::SeparatorText("Allocated memory");
            ImGui::Text("Meshes: %.3f MiB", static_cast<double>(stats.mesh_allocation_bytes) / bytes_per_mib);
            ImGui::Text("Materials: %.3f MiB", static_cast<double>(stats.material_allocation_bytes) / bytes_per_mib);
            ImGui::Text("Textures: %.3f MiB", static_cast<double>(stats.texture_allocation_bytes) / bytes_per_mib);
            ImGui::TextDisabled("Buffer/image allocations, including unused capacity.");
            ImGui::TextDisabled("Includes resources pending destruction.");
        }
        ImGui::End();
        m_imgui.end_frame();

        if (m_window.width() == 0 || m_window.height() == 0) { return; }
        if (auto frame = m_renderer.begin_frame(m_swapchain)) {
            m_renderer.render_scene({
                .view       = glm::lookAtRH(m_camera_position, m_camera_position + m_camera_front, m_camera_up),
                .projection = glm::perspectiveRH_ZO(glm::radians(45.0f), m_window.aspect_ratio(), 0.1f, 100.0f),
                .scene      = {.instances = m_instances},
            });
            m_renderer.render_ui(ImGui::GetDrawData());
            m_renderer.end_frame();
        }

        m_imgui.update_platform_windows();
    }

    bool should_exit() const override { return m_window.should_close(); }

    orion::Window m_window;
    orion::Renderer& m_renderer;
    orion::gpu::Swapchain* m_swapchain;
    orion::ImGuiContext m_imgui;
    fastgltf::Asset m_asset;
    std::filesystem::path m_asset_directory;
    orion::Material m_default_material;
    std::unordered_map<std::size_t, orion::Texture> m_textures;
    std::vector<orion::Material> m_materials;
    std::vector<std::vector<MeshPrimitive>> m_meshes;
    std::vector<orion::RenderInstance> m_instances;

    orion::MousePosition m_last_mouse_position{0.0f, 0.0f};
    glm::vec3 m_camera_position{0.0f, 0.0f, -5.0f};
    glm::vec3 m_camera_front{0.0f, 0.0f, 1.0f};
    glm::vec3 m_camera_up{0.0f, 1.0f, 0.0f};
    glm::vec3 m_camera_right{-1.0f, 0.0f, 0.0f};
    float m_camera_pitch{0.0f};
    float m_camera_yaw{90.0f};
    bool m_cursor_locked{true};
    float m_camera_sensitivity{0.1f};
    float m_camera_speed{5.0f};
};

int main(int argc, char* argv[])
{
    auto engine = orion::Engine{};

    if (argc != 2) {
        ORION_LOG_ERROR(orion::LogChannel::Application, "Usage: {} <asset.gltf|asset.glb>", argv[0]);
        return 1;
    }

    const auto asset_path = std::filesystem::path(argv[1]);
    auto data             = fastgltf::GltfDataBuffer::FromPath(asset_path);
    if (data.error() != fastgltf::Error::None) {
        ORION_LOG_ERROR(orion::LogChannel::Application,
                        "Failed to read {}: {}",
                        asset_path.string(), fastgltf::getErrorMessage(data.error()));
        return 1;
    }

    auto parser = fastgltf::Parser{};
    // Load buffers for meshes and embedded images; external images are loaded on demand.
    auto asset = parser.loadGltf(data.get(), asset_path.parent_path(), fastgltf::Options::LoadExternalBuffers);
    if (asset.error() != fastgltf::Error::None) {
        ORION_LOG_ERROR(orion::LogChannel::Application,
                        "Failed to parse {}: {}",
                        asset_path.string(), fastgltf::getErrorMessage(asset.error()));
        return 1;
    }
    if (const auto error = fastgltf::validate(asset.get()); error != fastgltf::Error::None) {
        ORION_LOG_ERROR(orion::LogChannel::Application,
                        "Invalid asset {}: {}", asset_path.string(), fastgltf::getErrorMessage(error));
        return 1;
    }

    ORION_LOG_INFO(
        orion::LogChannel::Application,
        "Loaded {}: {} scenes, {} nodes, {} meshes, {} textures",
        asset_path.string(), asset->scenes.size(), asset->nodes.size(), asset->meshes.size(), asset->textures.size());

    engine.run(std::make_unique<Sandbox>(engine, std::move(asset.get()), asset_path.parent_path()));
    return 0;
}
