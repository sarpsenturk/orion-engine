#include "window.hpp"

#include "app.hpp"

#include <GLFW/glfw3.h>

namespace orion
{
    void WindowPlugin::build(App& app)
    {
        app
            .add_system(App::Schedule::Startup, [&app, config = std::move(config)](World& world) {
                // Initialize GLFW
                glfwInit();

                // Create GLFWwindow* without OpenGL context
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
                auto* window = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);

                // Set GLFW callbacks
                glfwSetWindowUserPointer(window, &app);
                glfwSetWindowCloseCallback(window, [](GLFWwindow* _window) {
                    auto* _app = static_cast<App*>(glfwGetWindowUserPointer(_window));
                    _app->exit();
                });

                // Use framebuffer size to get the actual size of the window in pixels
                int width, height;
                glfwGetFramebufferSize(window, &width, &height);

                world.insert_resource(Window{window, width, height});
            })
            .add_system(App::Schedule::Shutdown, [](World& world) {
                auto window = world.remove_resource<Window>();
                glfwDestroyWindow(static_cast<GLFWwindow*>(window->handle));
                glfwTerminate();
            })
            .set_runner([](App& app) {
                while (!app.should_exit()) {
                    glfwPollEvents();
                    app.do_frame();
                }
                return 0;
            });
    }
} // namespace orion
