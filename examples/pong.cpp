#include <orion/application.hpp>
#include <orion/window.hpp>

#include <orion/renderapi/render_backend.hpp>

#include <orion/renderer/renderer.hpp>
#include <orion/renderer/sprite_renderer.hpp>

#include <spdlog/spdlog.h>

using namespace orion;

class Pong final : public Application
{
public:
    Pong()
        : window_({.title = "Pong", .width = 800, .height = 600})
        , renderer_({.render_backend = RenderBackend::create_builtin_vulkan(), .width = 800, .height = 600})
        , sprite_renderer_({.render_device = renderer_.render_device(), .screen_width = 800, .screen_height = 600, .camera_size = 100})
    {
        // Get render device from renderer
        auto* render_device = renderer_.render_device();

        // Get command queue from renderer
        auto* graphics_queue = renderer_.graphics_queue();

        // Create swapchain
        swapchain_ = render_device->create_swapchain({
            .window = &window_,
            .queue = graphics_queue,
            .width = 800,
            .height = 600,
            .image_count = 2,
            .image_format = Format::B8G8R8A8_Unorm,
        });

        // Set camera background to black
        camera_.set_clear_color(colors::black);
    }

private:
    void on_update(duration dt) override
    {
        WindowEvent event;
        while ((event = window_.poll_event())) {
            if (event.is<OnWindowClose>()) {
                orion_exit();
            }
        }

        auto clamp_paddle = [](Vector3f& paddle) {
            paddle[1] = std::clamp(paddle[1], -3.f + paddle_scale[1] / 2, 3.f - paddle_scale[1] / 2);
        };

        // Get user input
        if (window_.keyboard().is_key_down(Keycode::KeyW)) {
            left_paddle_pos_[1] += paddle_speed * dt.count();
        }
        if (window_.keyboard().is_key_down(Keycode::KeyS)) {
            left_paddle_pos_[1] -= paddle_speed * dt.count();
        }
        clamp_paddle(left_paddle_pos_);

        if (window_.keyboard().is_key_down(Keycode::UpArrow)) {
            right_paddle_pos_[1] += paddle_speed * dt.count();
        }
        if (window_.keyboard().is_key_down(Keycode::DownArrow)) {
            right_paddle_pos_[1] -= paddle_speed * dt.count();
        }
        clamp_paddle(right_paddle_pos_);

        // Update ball position
        ball_pos_ += ball_velocity_ * dt.count();

        // Check for collisions with left & right walls
        if (ball_pos_[0] < -4 || ball_pos_[0] > 4) {
            ball_pos_ = Vector3f{0, 0, 0};
        }

        // Check for collisions with top & bottom walls
        if (ball_pos_[1] < -3 || ball_pos_[1] > 3) {
            ball_velocity_[1] *= -1;
        }

        // Check for collisions with left paddle
        auto paddle_collision = [ball = ball_pos_](const Vector3f& paddle) {
            const auto left = paddle[0] - paddle_scale[0] / 2;
            const auto right = paddle[0] + paddle_scale[0] / 2;
            const auto x = ball[0] > left && ball[0] < right;
            const auto top = paddle[1] + paddle_scale[1] / 2;
            const auto bottom = paddle[1] - paddle_scale[1] / 2;
            const auto y = ball[1] < top && ball[1] > bottom;
            return x && y;
        };
        if (paddle_collision(left_paddle_pos_)) {
            const auto yoffset = (left_paddle_pos_[1] - ball_pos_[1]) / (paddle_scale[1] / 2);
            SPDLOG_TRACE("yoffset = {}", yoffset);
            const auto ball_velocity_magnitude = ball_velocity_.magnitude();
            SPDLOG_TRACE("magnitude = {}", ball_velocity_magnitude);
            const auto vertical_speed = ball_velocity_magnitude * yoffset;
            const auto horizontal_speed = ball_velocity_magnitude * (1 - yoffset);
            ball_velocity_ = {horizontal_speed, -vertical_speed, 0};
        }
        if (paddle_collision(right_paddle_pos_)) {
            const auto yoffset = (right_paddle_pos_[1] - ball_pos_[1]) / (paddle_scale[1] / 2);
            SPDLOG_TRACE("yoffset = {}", yoffset);
            const auto ball_velocity_magnitude = ball_velocity_.magnitude();
            SPDLOG_TRACE("magnitude = {}", ball_velocity_magnitude);
            const auto vertical_speed = ball_velocity_magnitude * yoffset;
            const auto horizontal_speed = ball_velocity_magnitude * (1 - yoffset);
            ball_velocity_ = {-horizontal_speed, -vertical_speed, 0};
        }
    }

    void on_render() override
    {
        // Begin a new frame
        renderer_.begin_frame(camera_);

        // Draw left paddle
        sprite_renderer_.draw(left_paddle_pos_, paddle_scale);

        // Draw right paddle
        sprite_renderer_.draw(right_paddle_pos_, paddle_scale);

        // Draw the ball
        sprite_renderer_.draw(ball_pos_, ball_scale);

        // Submit sprites
        sprite_renderer_.submit(&renderer_);

        // End frame
        renderer_.end_frame();

        // Present results to swapchain
        renderer_.present(swapchain_.get(), 800, 600);
    }

    Window window_;
    Renderer renderer_;
    Camera camera_;
    SpriteRenderer sprite_renderer_;
    std::unique_ptr<Swapchain> swapchain_;

    static constexpr auto paddle_x_offset = 3.5f;
    static constexpr auto paddle_scale = Vector2f{0.25f, 1.25f};
    static constexpr auto paddle_speed = 3.f;

    static constexpr auto ball_scale = Vector2f{0.2f, 0.2f};

    Vector3f left_paddle_pos_ = Vector3f{-paddle_x_offset, 0, 0};
    Vector3f right_paddle_pos_ = Vector3f{paddle_x_offset, 0, 0};

    Vector3f ball_pos_ = Vector3f{0.f, 0.f, 0.f};
    Vector3f ball_velocity_ = Vector3f{-5.f, 0.f, 0.f};
};

std::unique_ptr<Application> orion_main(std::span<const char* const> args)
{
    return std::make_unique<Pong>();
}
