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

#include "utility/state_tree/state_tree.hpp"
#include "utility/tree_states/tree_states.hpp"

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
            std::cout << "process key press: " << key_string << std::endl;
            ui.process_key_press(key_string);
        }
        if (key_pressed_this_tick == GLFW_KEY_BACKSPACE) {
            ui.process_delete_action();
        }
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

enum GameState { MAIN_MENU, IN_GAME, SETTINGS, CREDITS };

void process_and_queue_render_ui(glm::vec2 ndc_mouse_pos, UI &curr_ui, Batcher &batcher) {
    curr_ui.process_mouse_position(ndc_mouse_pos);

    if (mouse_just_clicked) {
        curr_ui.process_mouse_just_clicked(ndc_mouse_pos);
    }

    process_key_pressed_this_tick(curr_ui);

    for (auto &cb : curr_ui.get_colored_boxes()) {
        batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
            cb.id, cb.ivpsc.indices, cb.ivpsc.xyz_positions, cb.ivpsc.rgb_colors,
            cb.modified_signal.has_just_changed());
    }

    for (auto &tb : curr_ui.get_text_boxes()) {
        batcher.transform_v_with_signed_distance_field_text_shader_batcher.queue_draw(
            tb.id, tb.text_drawing_data.indices, tb.text_drawing_data.xyz_positions,
            tb.text_drawing_data.texture_coordinates);
        batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
            tb.id, tb.background_ivpsc.indices, tb.background_ivpsc.xyz_positions, tb.background_ivpsc.rgb_colors,
            tb.modified_signal.has_just_changed());
    }

    for (auto &cr : curr_ui.get_clickable_text_boxes()) {
        batcher.transform_v_with_signed_distance_field_text_shader_batcher.queue_draw(
            cr.id, cr.text_drawing_data.indices, cr.text_drawing_data.xyz_positions,
            cr.text_drawing_data.texture_coordinates);

        batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
            cr.id, cr.ivpsc.indices, cr.ivpsc.xyz_positions, cr.ivpsc.rgb_colors,
            cr.modified_signal.has_just_changed());
    }

    for (auto &ib : curr_ui.get_input_boxes()) {

        batcher.transform_v_with_signed_distance_field_text_shader_batcher.queue_draw(
            ib.id, ib.text_drawing_data.indices, ib.text_drawing_data.xyz_positions,
            ib.text_drawing_data.texture_coordinates, ib.modified_signal.has_just_changed());

        batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
            ib.id, ib.background_ivpsc.indices, ib.background_ivpsc.xyz_positions, ib.background_ivpsc.rgb_colors,
            ib.modified_signal.has_just_changed());
    }

    for (auto &dd : curr_ui.get_dropdowns()) {

        batcher.transform_v_with_signed_distance_field_text_shader_batcher.queue_draw(
            dd.id, dd.dropdown_text_data.indices, dd.dropdown_text_data.xyz_positions,
            dd.dropdown_text_data.texture_coordinates, dd.modified_signal.has_just_changed());

        batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
            dd.id, dd.dropdown_background.indices, dd.dropdown_background.xyz_positions,
            dd.dropdown_background.rgb_colors, dd.modified_signal.has_just_changed());

        // render all the dropdowns if they're active
        if (dd.dropdown_open) {
            int num_dropdowns = dd.dropdown_option_rects.size();
            for (int i = 0; i < num_dropdowns; i++) {
                IVPSolidColor ivpsc = dd.dropdown_option_backgrounds[i];
                IVPTextured ivpt = dd.dropdown_option_text_data[i];
                unsigned int doid = dd.dropdown_doids[i];

                batcher.transform_v_with_signed_distance_field_text_shader_batcher.queue_draw(
                    doid, ivpt.indices, ivpt.xyz_positions, ivpt.texture_coordinates,
                    dd.modified_signal.has_just_changed());

                batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
                    doid, ivpsc.indices, ivpsc.xyz_positions, ivpsc.rgb_colors, dd.modified_signal.has_just_changed());
            }
        }
    }
}

int main() {

    // TODO I don't think I even need to use tree states here, just regular states and do the ui map approach defined
    // later down
    std::vector<TreeState> curr_state = {TreeState::MAIN_MENU};

    // clang-format off
    /*StateTree state({*/
    /*    {TreeState::MAIN_MENU, StateTree({*/
    /*            {TreeState::SETTINGS, StateTree({*/
    /*                    {TreeState::PLAYER, StateTree({})},*/
    /*                    {TreeState::INPUT, StateTree({})},*/
    /*                    {TreeState::SOUND, StateTree({})},*/
    /*                    {TreeState::GRAPHICS, StateTree({})},*/
    /*                    {TreeState::NETWORK, StateTree({})}*/
    /*             })}*/
    /*    })}*/
    /*});*/
    // clang-format on

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("mwe_shader_cache_logs.txt", true);
    file_sink->set_level(spdlog::level::info);
    std::vector<spdlog::sink_ptr> sinks = {console_sink, file_sink};

    LiveInputState live_input_state;
    bool start_in_fullscreen = false;
    GLFWwindow *window = initialize_glfw_glad_and_return_window(&SCREEN_WIDTH, &SCREEN_HEIGHT, "glfw window",
                                                                start_in_fullscreen, false, false, &live_input_state);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);

    std::vector<ShaderType> requested_shaders = {ShaderType::ABSOLUTE_POSITION_WITH_COLORED_VERTEX,
                                                 ShaderType::TRANSFORM_V_WITH_SIGNED_DISTANCE_FIELD_TEXT};

    ShaderCache shader_cache(requested_shaders, sinks);
    Batcher batcher(shader_cache);
    FontAtlas font_atlas("assets/times_64_sdf_atlas_font_info.json", "assets/times_64_sdf_atlas.json",
                         "assets/times_64_sdf_atlas.png", SCREEN_WIDTH, false, true);

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

    std::function<void()> on_game_start = [&]() { curr_state = {TreeState::IN_GAME}; };
    std::function<void()> on_click_settings = [&]() { curr_state = {TreeState::SETTINGS, TreeState::PLAYER}; };
    std::function<void()> on_game_quit = [&]() { glfwSetWindowShouldClose(window, GLFW_TRUE); };
    std::function<void()> on_back_clicked = [&]() { curr_state = {TreeState::MAIN_MENU}; };

    std::function<void()> on_hover = [&]() {};

    // main menu ui
    UI main_menu_ui(font_atlas);
    main_menu_ui.add_textbox("WELCOME TO FRAG-Z", 0, 0.75, 1, 0.25, colors.grey);

    Grid grid(4, 1, 0.5, 0.5);
    auto frag_time_rect = grid.get_at(0, 0);
    main_menu_ui.add_clickable_textbox(on_game_start, on_hover, "FRAG TIME", frag_time_rect, colors.darkgreen,
                                       colors.green);

    auto settings_rect = grid.get_at(0, 1);
    main_menu_ui.add_clickable_textbox(on_click_settings, on_hover, "SETTINGS", settings_rect, colors.darkblue,
                                       colors.blue);

    auto credits_rect = grid.get_at(0, 2);
    main_menu_ui.add_clickable_textbox(on_game_quit, on_hover, "CREDITS", credits_rect, colors.darkblue, colors.blue);

    auto exit_rect = grid.get_at(0, 3);
    main_menu_ui.add_clickable_textbox(on_game_quit, on_hover, "QUIT", exit_rect, colors.darkred, colors.red);

    // in game ui
    UI in_game_ui(font_atlas);
    std::function<void(std::string)> on_confirm = [&](std::string contents) { std::cout << contents << std::endl; };
    in_game_ui.add_input_box(on_confirm, "password", 0, 0.25, 1, 0.25, colors.grey, colors.lightgrey);
    in_game_ui.add_clickable_textbox(on_back_clicked, on_hover, "back to main menu", -0.65, -0.65, 0.5, 0.5,
                                     colors.seagreen, colors.grey);

    // settings ui
    UI settings_menu_ui(font_atlas);
    /*settings_menu_ui.*/

    Rectangle settings_menu_rect(glm::vec3(0, 0, 0), 1.5, 1.5);
    std::vector<Rectangle> settings_menu = weighted_subdivision(settings_menu_rect, {1, 3}, true);
    /*Rectangle main_area_rect = create_rectangle_from_corners(*/

    Grid top_row_grid(1, 5, settings_menu.at(0));

    std::function<void()> settings_on_click = []() {};
    std::function<void()> settings_on_hover = []() {};

    std::function<void()> player_on_click = [&]() { curr_state = {TreeState::SETTINGS, TreeState::PLAYER}; };
    auto player_rect = top_row_grid.get_at(0, 0);
    settings_menu_ui.add_clickable_textbox(player_on_click, settings_on_hover, "player", player_rect, colors.darkblue,
                                           colors.blue);

    std::function<void()> input_on_click = [&]() { curr_state = {TreeState::SETTINGS, TreeState::INPUT}; };
    auto input_rect = top_row_grid.get_at(1, 0);
    settings_menu_ui.add_clickable_textbox(input_on_click, settings_on_hover, "input", input_rect, colors.darkblue,
                                           colors.blue);

    std::function<void()> sound_on_click = [&]() { curr_state = {TreeState::SETTINGS, TreeState::SOUND}; };
    auto sound_rect = top_row_grid.get_at(2, 0);
    settings_menu_ui.add_clickable_textbox(sound_on_click, settings_on_hover, "sound", sound_rect, colors.darkblue,
                                           colors.blue);

    std::function<void()> graphics_on_click = [&]() { curr_state = {TreeState::SETTINGS, TreeState::GRAPHICS}; };
    auto graphics_rect = top_row_grid.get_at(3, 0);
    settings_menu_ui.add_clickable_textbox(graphics_on_click, settings_on_hover, "graphics", graphics_rect,
                                           colors.darkblue, colors.blue);

    std::function<void()> network_on_click = [&]() { curr_state = {TreeState::SETTINGS, TreeState::ADVANCED}; };
    auto network_rect = top_row_grid.get_at(4, 0);
    settings_menu_ui.add_clickable_textbox(network_on_click, settings_on_hover, "network", network_rect,
                                           colors.darkblue, colors.blue);

    Rectangle main_settings_rect = settings_menu.at(1);
    settings_menu_ui.add_colored_rectangle(main_settings_rect, colors.grey);

    Rectangle go_back_rect = create_rectangle_from_corners(glm::vec3(-1, -0.75, 0), glm::vec3(-0.75, -0.75, 0),
                                                           glm::vec3(-1, -1, 0), glm::vec3(-0.75, -1, 0));
    settings_menu_ui.add_clickable_textbox(on_back_clicked, on_hover, "BACK", go_back_rect, colors.darkred, colors.red);

    Grid main_settings_grid(7, 3, main_settings_rect);

    UI player_settings_ui(font_atlas);

    std::function<void(std::string)> username_on_confirm = [](std::string s) {};
    player_settings_ui.add_textbox("username", main_settings_grid.get_at(0, 0), colors.maroon);
    player_settings_ui.add_input_box(on_confirm, "username", main_settings_grid.get_at(2, 0), colors.orange,
                                     colors.orangered);
    player_settings_ui.add_textbox("crosshair", main_settings_grid.get_at(0, 1), colors.maroon);

    Grid input_settings_grid(10, 3, main_settings_rect);
    UI input_settings_ui(font_atlas);
    input_settings_ui.add_textbox("mouse sensitivity", input_settings_grid.get_at(0, 0), colors.maroon);
    input_settings_ui.add_textbox("fire", input_settings_grid.get_at(0, 1), colors.maroon);
    input_settings_ui.add_textbox("jump", input_settings_grid.get_at(0, 2), colors.maroon);
    input_settings_ui.add_textbox("move forward", input_settings_grid.get_at(0, 3), colors.maroon);
    input_settings_ui.add_textbox("move backward", input_settings_grid.get_at(0, 4), colors.maroon);
    input_settings_ui.add_textbox("move left", input_settings_grid.get_at(0, 5), colors.maroon);
    input_settings_ui.add_textbox("move right", input_settings_grid.get_at(0, 6), colors.maroon);
    input_settings_ui.add_textbox("select weapon 1", input_settings_grid.get_at(0, 7), colors.maroon);
    input_settings_ui.add_textbox("select weapon 2", input_settings_grid.get_at(0, 8), colors.maroon);
    input_settings_ui.add_textbox("select weapon 3", input_settings_grid.get_at(0, 9), colors.maroon);

    Grid sound_settings_grid(1, 3, main_settings_rect);
    UI sound_settings_ui(font_atlas);
    sound_settings_ui.add_textbox("volume", sound_settings_grid.get_at(0, 0), colors.maroon);

    Grid graphics_settings_grid(5, 3, main_settings_rect);
    UI graphics_settings_ui(font_atlas);

    std::vector<std::string> options = {"800x600", "1024x1024"};
    std::vector<std::function<void()>> option_on_clicks = {[]() {}, []() {}};
    graphics_settings_ui.add_textbox("resolution", graphics_settings_grid.get_at(0, 0), colors.maroon);
    graphics_settings_ui.add_dropdown(on_click_settings, on_hover, "1920x1080", graphics_settings_grid.get_at(2, 0),
                                      colors.orange, colors.orangered, options, option_on_clicks);

    std::vector<std::string> lighting_options = {"none", "early 2000s"};
    std::vector<std::function<void()>> lighting_option_on_clicks = {[]() {}, []() {}};
    graphics_settings_ui.add_textbox("lighting", graphics_settings_grid.get_at(0, 1), colors.maroon);
    graphics_settings_ui.add_dropdown(on_click_settings, on_hover, "lighting", graphics_settings_grid.get_at(2, 1),
                                      colors.orange, colors.orangered, lighting_options, lighting_option_on_clicks);

    graphics_settings_ui.add_textbox("fov", graphics_settings_grid.get_at(0, 2), colors.maroon);
    in_game_ui.add_input_box(on_confirm, "enter fov", graphics_settings_grid.get_at(2, 2), colors.grey,
                             colors.lightgrey);

    std::vector<std::string> yes_no_options = {"yes", "no"};
    std::vector<std::function<void()>> viewmodel_options_on_click = {[]() {}, []() {}};
    graphics_settings_ui.add_textbox("enable view model", graphics_settings_grid.get_at(0, 3), colors.maroon);
    graphics_settings_ui.add_dropdown(on_click_settings, on_hover, "yes", graphics_settings_grid.get_at(2, 3),
                                      colors.orange, colors.orangered, yes_no_options, viewmodel_options_on_click);

    std::vector<std::function<void()>> fps_options_on_click = {[]() {}, []() {}};
    graphics_settings_ui.add_textbox("show fps", graphics_settings_grid.get_at(0, 4), colors.maroon);
    graphics_settings_ui.add_dropdown(on_click_settings, on_hover, "yes", graphics_settings_grid.get_at(2, 4),
                                      colors.orange, colors.orangered, yes_no_options, fps_options_on_click);

    Grid advanced_settings_grid(3, 3, main_settings_rect);
    UI advanced_settings_ui(font_atlas);
    advanced_settings_ui.add_textbox("display tick time expendature", advanced_settings_grid.get_at(0, 0),
                                     colors.maroon);
    advanced_settings_ui.add_textbox("display current ping", advanced_settings_grid.get_at(0, 1), colors.maroon);
    advanced_settings_ui.add_textbox("display movement dial", advanced_settings_grid.get_at(0, 2), colors.maroon);

    std::map<StatePath, UI> game_state_to_ui = {
        {{TreeState::MAIN_MENU}, main_menu_ui},
        {{TreeState::IN_GAME}, in_game_ui},
        {{TreeState::SETTINGS}, settings_menu_ui},
        {{TreeState::SETTINGS, TreeState::PLAYER}, player_settings_ui},
        {{TreeState::SETTINGS, TreeState::INPUT}, input_settings_ui},
        {{TreeState::SETTINGS, TreeState::SOUND}, sound_settings_ui},
        {{TreeState::SETTINGS, TreeState::GRAPHICS}, graphics_settings_ui},
        {{TreeState::SETTINGS, TreeState::ADVANCED}, advanced_settings_ui},
    };

    int width, height;

    while (!glfwWindowShouldClose(window)) {

        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto ndc_mouse_pos =
            get_ndc_mouse_pose(window, live_input_state.mouse_position_x, live_input_state.mouse_position_y);

        // here we render all ui's and ui's that are ancestor ones
        std::vector<TreeState> incremental_path;
        for (const auto ts : curr_state) {
            incremental_path.push_back(ts);
            if (game_state_to_ui.find(incremental_path) != game_state_to_ui.end()) {
                UI &ancestor_ui = game_state_to_ui.at(incremental_path);
                process_and_queue_render_ui(ndc_mouse_pos, ancestor_ui, batcher);
            }
        }

        batcher.absolute_position_with_colored_vertex_shader_batcher.draw_everything();
        batcher.transform_v_with_signed_distance_field_text_shader_batcher.draw_everything();

        mouse_just_clicked = false;

        key_pressed_this_tick = GLFW_KEY_UNKNOWN; // Clear the key at the end of the tick
        TemporalBinarySignal::process_all();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
