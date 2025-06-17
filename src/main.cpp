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

#include <cstdio>
#include <cstdlib>

int key_pressed_this_tick = GLFW_KEY_UNKNOWN;
std::string key_to_string(int key) {
    static const std::unordered_map<int, std::string> key_map = {
        {GLFW_KEY_A, "a"}, {GLFW_KEY_B, "b"},    {GLFW_KEY_C, "c"}, {GLFW_KEY_D, "d"}, {GLFW_KEY_E, "e"},
        {GLFW_KEY_F, "f"}, {GLFW_KEY_G, "g"},    {GLFW_KEY_H, "h"}, {GLFW_KEY_I, "i"}, {GLFW_KEY_J, "j"},
        {GLFW_KEY_K, "k"}, {GLFW_KEY_L, "l"},    {GLFW_KEY_M, "m"}, {GLFW_KEY_N, "n"}, {GLFW_KEY_O, "o"},
        {GLFW_KEY_P, "p"}, {GLFW_KEY_Q, "q"},    {GLFW_KEY_R, "r"}, {GLFW_KEY_S, "s"}, {GLFW_KEY_T, "t"},
        {GLFW_KEY_U, "u"}, {GLFW_KEY_V, "v"},    {GLFW_KEY_W, "w"}, {GLFW_KEY_X, "x"}, {GLFW_KEY_Y, "y"},
        {GLFW_KEY_Z, "z"}, {GLFW_KEY_0, "0"},    {GLFW_KEY_1, "1"}, {GLFW_KEY_2, "2"}, {GLFW_KEY_3, "3"},
        {GLFW_KEY_4, "4"}, {GLFW_KEY_5, "5"},    {GLFW_KEY_6, "6"}, {GLFW_KEY_7, "7"}, {GLFW_KEY_8, "8"},
        {GLFW_KEY_9, "9"}, {GLFW_KEY_SPACE, " "} // Add more keys if needed, like special characters or function keys
    };

    // Find the key in the map and return the corresponding string, or empty if not found
    auto it = key_map.find(key);
    return (it != key_map.end()) ? it->second : "";
}

void process_key_pressed_this_tick(UI &ui) {
    if (key_pressed_this_tick != GLFW_KEY_UNKNOWN) {
        std::string key_string = key_to_string(key_pressed_this_tick);
        if (!key_string.empty()) {
            std::cout << "process key press: " << key_string << std::endl;
            ui.process_key_press(key_string);
        }
        if (key_pressed_this_tick == GLFW_KEY_BACKSPACE) {
            ui.process_delete_action();
        }
    }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {}

unsigned int SCREEN_WIDTH = 640;
unsigned int SCREEN_HEIGHT = 480;

bool mouse_just_clicked = false;

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouse_just_clicked = true;
    }
}

double mouse_position_x, mouse_position_y;

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
    mouse_position_x = xpos;
    mouse_position_y = ypos;
}

glm::vec2 get_ndc_mouse_pos(GLFWwindow *window, double xpos, double ypos) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    return {(2.0f * xpos) / width - 1.0f, 1.0f - (2.0f * ypos) / height};
}

int main() {

    UIState curr_state = UIState::MAIN_MENU;

    std::unordered_map<SoundType, std::string> sound_type_to_file = {
        {SoundType::UI_HOVER, "assets/sounds/hover.wav"},
        {SoundType::UI_CLICK, "assets/sounds/click.wav"},
        {SoundType::UI_SUCCESS, "assets/sounds/success.wav"},
    };

    SoundSystem sound_system(100, sound_type_to_file);

    Window window;

    std::function<void(unsigned int)> char_callback = [](unsigned int codepoint) {};
    std::function<void(int, int, int, int)> key_callback = [](int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) {
            key_pressed_this_tick = key; // Store the key pressed this tick
        }
    };
    std::function<void(double, double)> mouse_pos_callback = [](double xpos, double ypos) {
        mouse_position_x = xpos;
        mouse_position_y = ypos;
    };
    std::function<void(int, int, int)> mouse_button_callback = [](int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            mouse_just_clicked = true;
        }
    };
    std::function<void(int, int)> frame_buffer_size_callback = [&](int width, int height) {
        // this gets called whenever the window changes size, because the framebuffer automatically
        // changes size, that is all done in glfw's context, then we need to update opengl's size.
        std::cout << "framebuffersize callback called, width" << width << "height: " << height << std::endl;
        glViewport(0, 0, width, height);
        window.width_px = width;
        window.height_px = height;
    };
    GLFWLambdaCallbackManager glcm(window.glfw_window, char_callback, key_callback, mouse_pos_callback,
                                   mouse_button_callback, frame_buffer_size_callback);

    // using this to save our configuration for the future.
    Configuration configuration("assets/config/user_cfg.ini", {}, false);

    configuration.register_config_handler("graphics", "resolution",
                                          [&](const std::string resolution) { window.set_resolution(resolution); });

    configuration.register_config_handler("graphics", "fullscreen",
                                          [&](const std::string value) { window.set_fullscreen_by_on_off(value); });

    configuration.apply_config_logic();

    bool start_in_fullscreen = false;

    std::vector<ShaderType> requested_shaders = {ShaderType::ABSOLUTE_POSITION_WITH_COLORED_VERTEX};

    ShaderCache shader_cache(requested_shaders);
    Batcher batcher(shader_cache);

    glm::mat4 projection = glm::mat4(1);
    auto text_color = glm::vec3(0.5, 0.5, 1);
    float char_width = 0.5;
    float edge_transition = 0.1;

    // glDisable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader_cache.set_uniform(ShaderType::ABSOLUTE_POSITION_WITH_COLORED_VERTEX, ShaderUniformVariable::ASPECT_RATIO,
                             glm::vec2(1, 1));

    UIRenderSuiteImpl ui_render_suite(batcher);

    InputGraphicsSoundMenu input_graphics_sound_menu(curr_state, window.glfw_window, batcher, sound_system,
                                                     configuration);

    std::map<UIState, UI> game_state_to_ui = {
        {UIState::MAIN_MENU, input_graphics_sound_menu.main_menu_ui},
        {UIState::IN_GAME, input_graphics_sound_menu.in_game_ui},
        {UIState::SETTINGS_MENU, input_graphics_sound_menu.settings_menu_ui},
        {UIState::PLAYER_SETTINGS, input_graphics_sound_menu.player_settings_ui},
        {UIState::INPUT_SETTINGS, input_graphics_sound_menu.input_settings_ui},
        {UIState::SOUND_SETTINGS, input_graphics_sound_menu.sound_settings_ui},
        {UIState::GRAPHICS_SETTINGS, input_graphics_sound_menu.graphics_settings_ui},
        {UIState::ADVANCED_SETTINGS, input_graphics_sound_menu.advanced_settings_ui},
    };

    // TODO: was debugging this I believe instead we have to use the window.width stuff here instead

    while (!glfwWindowShouldClose(window.glfw_window)) {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto ndc_mouse_pos = get_ndc_mouse_pos(window.glfw_window, mouse_position_x, mouse_position_y);

        std::vector<UIState> uis_to_render = {curr_state};
        for (const auto &ui_state : input_graphics_sound_menu.get_ui_dependencies(curr_state)) {
            uis_to_render.push_back(ui_state);
        }

        for (const auto &ui_state : uis_to_render) {
            if (game_state_to_ui.find(ui_state) != game_state_to_ui.end()) {
                UI &selected_ui = game_state_to_ui.at(ui_state);

                process_key_pressed_this_tick(selected_ui); // because I yet use the
                                                            // InputState

                process_and_queue_render_ui(ndc_mouse_pos, selected_ui, ui_render_suite, {}, false, false,
                                            mouse_just_clicked);
            }
        }

        batcher.absolute_position_with_colored_vertex_shader_batcher.draw_everything();

        sound_system.play_all_sounds();

        mouse_just_clicked = false;

        key_pressed_this_tick = GLFW_KEY_UNKNOWN; // Clear the key at the end of
                                                  // the tick
        TemporalBinarySignal::process_all();
        glfwSwapBuffers(window.glfw_window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window.glfw_window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
