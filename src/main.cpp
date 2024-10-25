#include <future>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "font_atlas/font_atlas.hpp"
#include "shader_standard/shader_standard.hpp"
#include "ui/ui.hpp"
#include "window/window.hpp"
#include "shader_cache/shader_cache.hpp"
#include "batcher/generated/batcher.hpp"
#include "colors/colors.hpp"

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
        {GLFW_KEY_9, "9"}, {GLFW_KEY_SPACE, " "}
        // Add more keys if needed, like special characters or function keys
    };

    // Find the key in the map and return the corresponding string, or empty if not found
    auto it = key_map.find(key);
    return (it != key_map.end()) ? it->second : "";
}

void process_key_pressed_this_tick(UI &ui) {
    if (key_pressed_this_tick != GLFW_KEY_UNKNOWN) {
        std::string key_string = key_to_string(key_pressed_this_tick);
        if (!key_string.empty()) {
            ui.process_key_press(key_string);
        }
        if (key_pressed_this_tick == GLFW_KEY_BACKSPACE) {
            ui.process_delete_action();
        }
        key_pressed_this_tick = GLFW_KEY_UNKNOWN; // Clear the key at the end of the tick
    }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        key_pressed_this_tick = key; // Store the key pressed this tick
    }
}

unsigned int SCREEN_WIDTH = 640;
unsigned int SCREEN_HEIGHT = 480;

bool mouse_just_clicked = false;

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouse_just_clicked = true;
    }
}

glm::vec2 get_ndc_mouse_pose(GLFWwindow *window, double xpos, double ypos) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    return {(2.0f * xpos) / width - 1.0f, 1.0f - (2.0f * ypos) / height};
}

enum GameState { MAIN_MENU, IN_GAME };

int main() {
    GameState curr_state = MAIN_MENU;
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("mwe_shader_cache_logs.txt", true);
    file_sink->set_level(spdlog::level::info);
    std::vector<spdlog::sink_ptr> sinks = {console_sink, file_sink};

    LiveInputState live_input_state;
    GLFWwindow *window = initialize_glfw_glad_and_return_window(&SCREEN_WIDTH, &SCREEN_HEIGHT, "glfw window", false,
                                                                false, false, &live_input_state);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);

    std::vector<ShaderType> requested_shaders = {ShaderType::ABSOLUTE_POSITION_WITH_COLORED_VERTEX,
                                                 ShaderType::TRANSFORM_V_WITH_SIGNED_DISTANCE_FIELD_TEXT};

    ShaderCache shader_cache(requested_shaders, sinks);
    Batcher batcher(shader_cache);
    FontAtlas font_atlas("assets/times_64_sdf_atlas_font_info.json", "assets/times_64_sdf_atlas.json",
                         "assets/times_64_sdf_atlas.png", SCREEN_WIDTH, false, true);

    UI main_menu_ui(font_atlas);
    Colors colors;

    glm::mat4 projection = glm::mat4(1);
    auto text_color = glm::vec3(0.5, 0.5, 1);
    float char_width = 0.5;
    float edge_transition = 0.1;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader_cache.use_shader_program(ShaderType::TRANSFORM_V_WITH_SIGNED_DISTANCE_FIELD_TEXT);
    shader_cache.set_uniform(ShaderType::TRANSFORM_V_WITH_SIGNED_DISTANCE_FIELD_TEXT, ShaderUniformVariable::TRANSFORM,
                             projection);

    shader_cache.set_uniform(ShaderType::TRANSFORM_V_WITH_SIGNED_DISTANCE_FIELD_TEXT, ShaderUniformVariable::RGB_COLOR,
                             text_color);

    shader_cache.set_uniform(ShaderType::TRANSFORM_V_WITH_SIGNED_DISTANCE_FIELD_TEXT,
                             ShaderUniformVariable::CHARACTER_WIDTH, char_width);

    shader_cache.set_uniform(ShaderType::TRANSFORM_V_WITH_SIGNED_DISTANCE_FIELD_TEXT,
                             ShaderUniformVariable::EDGE_TRANSITION_WIDTH, edge_transition);

    /*ui.add_colored_rectangle(0.5, 0, 0.25, 0.25, colors.aliceblue);*/
    /*ui.add_colored_rectangle(-0.5, 0, 0.25, 0.25, colors.cornsilk);*/
    /*ui.add_colored_rectangle(0, -0.5, 0.25, 0.25, colors.gold);*/
    /*ui.add_colored_rectangle(0, +0.5, 0.25, 0.25, colors.magenta);*/
    /*ui.add_textbox("font_test", .25, .25, .25, .25, colors.whitesmoke);*/

    std::function<void()> on_game_start = [&]() { curr_state = IN_GAME; };
    std::function<void()> on_game_quit = [&]() { glfwSetWindowShouldClose(window, GLFW_TRUE); };
    std::function<void()> on_back_clicked = [&]() { curr_state = MAIN_MENU; };

    main_menu_ui.add_textbox("WELCOME TO FRAG-Z", 0, 0.75, 1, 0.25, colors.grey);
    main_menu_ui.add_clickable_textbox(on_game_start, "FRAG TIME", 0.65, -0.65, 0.5, 0.5, colors.darkgreen,
                                       colors.green);
    main_menu_ui.add_clickable_textbox(on_game_quit, "GO BACK TO SLEEP", -0.65, -0.65, 0.5, 0.5, colors.darkred,
                                       colors.red);

    UI in_game_ui(font_atlas);

    std::function<void(std::string)> on_confirm = [&](std::string contents) { std::cout << contents << std::endl; };
    in_game_ui.add_input_box(on_confirm, "password", 0, 0.25, 1, 0.25, colors.grey, colors.lightgrey);
    in_game_ui.add_clickable_textbox(on_back_clicked, "back to main menu", -0.65, -0.65, 0.5, 0.5, colors.seagreen,
                                     colors.grey);

    std::unordered_map<GameState, UI> game_state_to_ui = {
        {MAIN_MENU, main_menu_ui},
        {IN_GAME, in_game_ui},
    };

    int width, height;

    while (!glfwWindowShouldClose(window)) {

        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto ndc_mouse_pos =
            get_ndc_mouse_pose(window, live_input_state.mouse_position_x, live_input_state.mouse_position_y);

        auto &curr_ui = game_state_to_ui.at(curr_state);

        curr_ui.process_mouse_position(ndc_mouse_pos);

        if (mouse_just_clicked) {
            curr_ui.process_mouse_just_clicked(ndc_mouse_pos);
        }

        process_key_pressed_this_tick(curr_ui);

        for (auto &tb : curr_ui.get_text_boxes()) {
            batcher.transform_v_with_signed_distance_field_text_shader_batcher.queue_draw(
                tb.text_drawing_data.indices, tb.text_drawing_data.xyz_positions,
                tb.text_drawing_data.texture_coordinates);
            batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
                tb.background_ivpsc.indices, tb.background_ivpsc.xyz_positions, tb.background_ivpsc.rgb_colors);
        }

        for (auto &cr : curr_ui.get_clickable_text_boxes()) {
            batcher.transform_v_with_signed_distance_field_text_shader_batcher.queue_draw(
                cr.text_drawing_data.indices, cr.text_drawing_data.xyz_positions,
                cr.text_drawing_data.texture_coordinates);
            batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
                cr.ivpsc.indices, cr.ivpsc.xyz_positions, cr.ivpsc.rgb_colors);
        }

        for (auto &ib : curr_ui.get_input_boxes()) {
            batcher.transform_v_with_signed_distance_field_text_shader_batcher.queue_draw(
                ib.text_drawing_data.indices, ib.text_drawing_data.xyz_positions,
                ib.text_drawing_data.texture_coordinates);
            batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
                ib.background_ivpsc.indices, ib.background_ivpsc.xyz_positions, ib.background_ivpsc.rgb_colors);
        }

        batcher.absolute_position_with_colored_vertex_shader_batcher.draw_everything();
        batcher.transform_v_with_signed_distance_field_text_shader_batcher.draw_everything();

        mouse_just_clicked = false;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
