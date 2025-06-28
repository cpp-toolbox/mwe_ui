// aboutodo was working on dropdown in ui.hpp
// it's currenlty broken because I'm not done there.
#include <future>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "graphics/font_atlas/font_atlas.hpp"
#include "graphics/shader_standard/shader_standard.hpp"
#include "graphics/ui/ui.hpp"
#include "graphics/window/window.hpp"
#include "graphics/shader_cache/shader_cache.hpp"
#include "graphics/batcher/generated/batcher.hpp"
#include "graphics/colors/colors.hpp"
#include "graphics/vertex_geometry/vertex_geometry.hpp"
#include "graphics/input_graphics_sound_menu/input_graphics_sound_menu.hpp"

#include "graphics/ui_render_suite_implementation/ui_render_suite_implementation.hpp"

#include "sound/sound_system/sound_system.hpp"
#include "sound/sound_types/sound_types.hpp"

#include "utility/config_file_parser/config_file_parser.hpp"

#include "input/glfw_lambda_callback_manager/glfw_lambda_callback_manager.hpp"
#include "input/input_state/input_state.hpp"

#include <cstdio>
#include <cstdlib>
#include <glm/fwd.hpp>

int main() {

    std::unordered_map<SoundType, std::string> sound_type_to_file = {
        {SoundType::UI_HOVER, "assets/sounds/hover.wav"},
        {SoundType::UI_CLICK, "assets/sounds/click.wav"},
        {SoundType::UI_SUCCESS, "assets/sounds/success.wav"},
    };

    SoundSystem sound_system(100, sound_type_to_file);

    Window window;

    InputState input_state;

    // using this to save our configuration for the future.
    Configuration configuration("assets/config/user_cfg.ini", {}, false);

    std::vector<ShaderType> requested_shaders = {ShaderType::ABSOLUTE_POSITION_WITH_COLORED_VERTEX};
    ShaderCache shader_cache(requested_shaders);
    Batcher batcher(shader_cache);

    std::function<void(unsigned int)> char_callback = [](unsigned int codepoint) {};
    std::function<void(int, int, int, int)> key_callback = [&](int key, int scancode, int action, int mods) {
        input_state.glfw_key_callback(key, scancode, action, mods);
    };
    std::function<void(double, double)> mouse_pos_callback = [&](double xpos, double ypos) {
        input_state.glfw_cursor_pos_callback(xpos, ypos);
    };
    std::function<void(int, int, int)> mouse_button_callback = [&](int button, int action, int mods) {
        input_state.glfw_mouse_button_callback(button, action, mods);
    };
    std::function<void(int, int)> frame_buffer_size_callback = [&](int width, int height) {
        // this gets called whenever the window changes size, because the framebuffer automatically
        // changes size, that is all done in glfw's context, then we need to update opengl's size.
        std::cout << "framebuffersize callback called, width" << width << "height: " << height << std::endl;
        glViewport(0, 0, width, height);
        window.width_px = width;
        window.height_px = height;

        shader_cache.set_uniform(ShaderType::ABSOLUTE_POSITION_WITH_COLORED_VERTEX, ShaderUniformVariable::ASPECT_RATIO,
                                 glm::vec2(height / (float)width, 1));
    };

    GLFWLambdaCallbackManager glcm(window.glfw_window, char_callback, key_callback, mouse_pos_callback,
                                   mouse_button_callback, frame_buffer_size_callback);

    glEnable(GL_DEPTH_TEST);

    shader_cache.set_uniform(ShaderType::ABSOLUTE_POSITION_WITH_COLORED_VERTEX, ShaderUniformVariable::ASPECT_RATIO,
                             glm::vec2(1, 1));

    UIRenderSuiteImpl ui_render_suite(batcher);

    InputGraphicsSoundMenu input_graphics_sound_menu(window, input_state, batcher, sound_system, configuration);

    while (!glfwWindowShouldClose(window.glfw_window)) {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        input_graphics_sound_menu.process_and_queue_render_menu(window, input_state, ui_render_suite);

        batcher.absolute_position_with_colored_vertex_shader_batcher.draw_everything();

        sound_system.play_all_sounds();

        TemporalBinarySignal::process_all();
        glfwSwapBuffers(window.glfw_window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window.glfw_window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
