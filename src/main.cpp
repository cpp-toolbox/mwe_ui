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
#include "utility/config_file_parser/config_file_parser.hpp"

#include <cstdio>
#include <cstdlib>

Colors colors;

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

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        key_pressed_this_tick = key; // Store the key pressed this tick
    }
}

unsigned int SCREEN_WIDTH = 640;
unsigned int SCREEN_HEIGHT = 480;

#include <iostream>

struct VideoMode {
    int width;
    int height;
    int refresh_rate;

    friend std::ostream &operator<<(std::ostream &os, const VideoMode &vm) {
        os << vm.width << "x" << vm.height << " @ " << vm.refresh_rate << "Hz";
        return os;
    }
};

std::vector<VideoMode> get_available_video_modes(GLFWmonitor *monitor) {
    std::vector<VideoMode> modes_out;

    if (!monitor) {
        std::cerr << "Invalid monitor pointer.\n";
        return modes_out;
    }

    int count;
    const GLFWvidmode *modes = glfwGetVideoModes(monitor, &count);

    for (int i = 0; i < count; ++i) {
        modes_out.push_back({modes[i].width, modes[i].height, modes[i].refreshRate});
    }

    return modes_out;
}

std::vector<std::string> video_modes_to_resolutions(const std::vector<VideoMode> &video_modes) {
    std::vector<std::string> resolutions;
    std::unordered_set<std::string> seen;

    for (const auto &mode : video_modes) {
        std::string res = std::to_string(mode.width) + "x" + std::to_string(mode.height);
        if (seen.insert(res).second) { // Insert if not already seen
            resolutions.push_back(res);
        }
    }

    return resolutions;
}

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

enum GameState { MAIN_MENU, IN_GAME, SETTINGS, CREDITS };

#include <iostream> // Needed for std::cout

class UIRenderSuiteImpl : public IUIRenderSuite {
  public:
    Batcher &batcher;

    explicit UIRenderSuiteImpl(Batcher &batcher) : batcher(batcher) {}

    void render_colored_box(const UIRect &cb) override {
        batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
            cb.ivpsc.id, cb.ivpsc.indices, cb.ivpsc.xyz_positions, cb.ivpsc.rgb_colors,
            cb.modified_signal.has_just_changed());
    }

    void render_text_box(const UITextBox &tb) override {
        batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
            tb.text_drawing_ivpsc.id, tb.text_drawing_ivpsc.indices, tb.text_drawing_ivpsc.xyz_positions,
            tb.text_drawing_ivpsc.rgb_colors, tb.modified_signal.has_just_changed());

        batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
            tb.background_ivpsc.id, tb.background_ivpsc.indices, tb.background_ivpsc.xyz_positions,
            tb.background_ivpsc.rgb_colors, tb.modified_signal.has_just_changed());
    }

    void render_clickable_text_box(const UIClickableTextBox &cr) override {
        batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
            cr.text_drawing_ivpsc.id, cr.text_drawing_ivpsc.indices, cr.text_drawing_ivpsc.xyz_positions,
            cr.text_drawing_ivpsc.rgb_colors);

        batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
            cr.ivpsc.id, cr.ivpsc.indices, cr.ivpsc.xyz_positions, cr.ivpsc.rgb_colors,
            cr.modified_signal.has_just_changed());
    }

    void render_input_box(const UIInputBox &ib) override {
        batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
            ib.text_drawing_ivpsc.id, ib.text_drawing_ivpsc.indices, ib.text_drawing_ivpsc.xyz_positions,
            ib.text_drawing_ivpsc.rgb_colors, ib.modified_signal.has_just_changed());

        batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
            ib.background_ivpsc.id, ib.background_ivpsc.indices, ib.background_ivpsc.xyz_positions,
            ib.background_ivpsc.rgb_colors, ib.modified_signal.has_just_changed());
    }

    void render_dropdown(const UIDropdown &dd) override {
        batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
            dd.dropdown_text_ivpsc.id, dd.dropdown_text_ivpsc.indices, dd.dropdown_text_ivpsc.xyz_positions,
            dd.dropdown_text_ivpsc.rgb_colors, dd.modified_signal.has_just_changed());

        batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
            dd.dropdown_background.id, dd.dropdown_background.indices, dd.dropdown_background.xyz_positions,
            dd.dropdown_background.rgb_colors, dd.modified_signal.has_just_changed());
    }

    void render_dropdown_option(const UIDropdown &dd, const draw_info::IVPSolidColor &ivpsc,
                                const draw_info::IVPSolidColor &text_ivpsc, unsigned int doid) override {

        batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
            text_ivpsc.id, text_ivpsc.indices, text_ivpsc.xyz_positions, text_ivpsc.rgb_colors,
            dd.modified_signal.has_just_changed());

        batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(
            ivpsc.id, ivpsc.indices, ivpsc.xyz_positions, ivpsc.rgb_colors, dd.modified_signal.has_just_changed());
    }
};

class MenuSystem {
  private:
    vertex_geometry::Rectangle settings_menu_rect = vertex_geometry::Rectangle(glm::vec3(0, 0, 0), 1.2, 1.2);
    std::vector<vertex_geometry::Rectangle> settings_menu = weighted_subdivision(settings_menu_rect, {1, 3}, true);

    Batcher &batcher;
    Configuration &configuration;
    GLFWwindow *window;
    std::vector<TreeState> &curr_state;

  public:
    UI main_menu_ui, in_game_ui, settings_menu_ui, player_settings_ui, input_settings_ui, sound_settings_ui,
        graphics_settings_ui, advanced_settings_ui;
    MenuSystem(std::vector<TreeState> &curr_state, GLFWwindow *window, Batcher &batcher, Configuration &configuration)
        : curr_state(curr_state), batcher(batcher), configuration(configuration), main_menu_ui(create_main_menu_ui()),
          in_game_ui(create_in_game_ui()), settings_menu_ui(create_settings_menu_ui()),
          player_settings_ui(create_player_settings_ui()), input_settings_ui(create_input_settings_ui()),
          sound_settings_ui(create_sound_settings_ui()), graphics_settings_ui(create_graphics_settings_ui()),
          advanced_settings_ui(create_advanced_settings_ui()) {};

    UI create_main_menu_ui() {

        std::function<void()> on_game_start = [&]() { curr_state = {TreeState::IN_GAME}; };
        std::function<void()> on_click_settings = [&]() { curr_state = {TreeState::SETTINGS, TreeState::PLAYER}; };
        std::function<void()> on_game_quit = [&]() { glfwSetWindowShouldClose(window, GLFW_TRUE); };
        std::function<void()> on_back_clicked = [&]() { curr_state = {TreeState::MAIN_MENU}; };
        std::function<void()> on_hover = [&]() {};

        UIRenderSuiteImpl ui_render_suite(batcher);

        // main menu ui
        UI main_menu_ui(0, batcher.absolute_position_with_colored_vertex_shader_batcher.object_id_generator);
        main_menu_ui.add_textbox("Welcome to the program.", 0, 0.75, 1, 0.25, colors.grey);

        vertex_geometry::Grid grid(4, 1, 0.5, 0.5);
        auto frag_time_rect = grid.get_at(0, 0);
        main_menu_ui.add_clickable_textbox(on_game_start, on_hover, "RUN", frag_time_rect, colors.darkgreen,
                                           colors.green);

        auto settings_rect = grid.get_at(0, 1);
        main_menu_ui.add_clickable_textbox(on_click_settings, on_hover, "SETTINGS", settings_rect, colors.darkblue,
                                           colors.blue);

        auto credits_rect = grid.get_at(0, 2);
        main_menu_ui.add_clickable_textbox(on_game_quit, on_hover, "CREDITS", credits_rect, colors.darkblue,
                                           colors.blue);

        auto exit_rect = grid.get_at(0, 3);
        main_menu_ui.add_clickable_textbox(on_game_quit, on_hover, "QUIT", exit_rect, colors.darkred, colors.red);

        return main_menu_ui;
    }

    UI create_in_game_ui() {

        std::function<void()> on_back_clicked = [&]() { curr_state = {TreeState::MAIN_MENU}; };
        std::function<void()> on_hover = [&]() {};

        UI in_game_ui(0, batcher.absolute_position_with_colored_vertex_shader_batcher.object_id_generator);
        std::function<void(std::string)> on_confirm = [&](std::string contents) { std::cout << contents << std::endl; };
        in_game_ui.add_input_box(on_confirm, "password", 0, 0.25, 1, 0.25, colors.grey, colors.lightgrey);
        in_game_ui.add_clickable_textbox(on_back_clicked, on_hover, "back to main menu", -0.65, -0.65, 0.5, 0.5,
                                         colors.seagreen, colors.grey);

        return in_game_ui;
    }

    UI create_settings_menu_ui() {
        UI settings_menu_ui(0, batcher.absolute_position_with_colored_vertex_shader_batcher.object_id_generator);

        vertex_geometry::Grid top_row_grid(1, 5, settings_menu.at(0));

        std::function<void()> on_back_clicked = [&]() { curr_state = {TreeState::MAIN_MENU}; };
        std::function<void()> on_apply_clicked = [&]() { configuration.apply_config_logic(); };
        std::function<void()> on_save_clicked = [&]() { configuration.save_to_file(); };
        std::function<void()> on_hover = [&]() {};
        std::function<void()> settings_on_click = []() {};
        std::function<void()> settings_on_hover = []() {};

        std::function<void()> player_on_click = [&]() { curr_state = {TreeState::SETTINGS, TreeState::PLAYER}; };
        auto player_rect = top_row_grid.get_at(0, 0);
        settings_menu_ui.add_clickable_textbox(player_on_click, settings_on_hover, "player", player_rect,
                                               colors.darkblue, colors.blue);

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

        vertex_geometry::Rectangle main_settings_rect = settings_menu.at(1);
        settings_menu_ui.add_colored_rectangle(main_settings_rect, colors.grey);

        vertex_geometry::Rectangle go_back_rect = vertex_geometry::create_rectangle_from_corners(
            glm::vec3(-1, -0.75, 0), glm::vec3(-0.75, -0.75, 0), glm::vec3(-1, -1, 0), glm::vec3(-0.75, -1, 0));
        settings_menu_ui.add_clickable_textbox(on_back_clicked, on_hover, "BACK", go_back_rect, colors.darkred,
                                               colors.red);

        vertex_geometry::Rectangle apply_rect = vertex_geometry::create_rectangle_from_corners(
            glm::vec3(1, -0.75, 0), glm::vec3(0.75, -0.75, 0), glm::vec3(1, -1, 0), glm::vec3(0.75, -1, 0));
        settings_menu_ui.add_clickable_textbox(on_apply_clicked, on_hover, "APPLY", apply_rect, colors.darkgreen,
                                               colors.green);

        vertex_geometry::Rectangle save_rect = vertex_geometry::slide_rectangle(apply_rect, -1, 0);

        settings_menu_ui.add_clickable_textbox(on_save_clicked, on_hover, "SAVE", save_rect, colors.darkgreen,
                                               colors.green);

        vertex_geometry::Grid main_settings_grid(7, 3, main_settings_rect);

        return settings_menu_ui;
    }

    UI create_player_settings_ui() {

        std::function<void(std::string)> on_confirm = [&](std::string contents) { std::cout << contents << std::endl; };

        UI player_settings_ui(-0.1, batcher.absolute_position_with_colored_vertex_shader_batcher.object_id_generator);

        vertex_geometry::Rectangle main_settings_rect = settings_menu.at(1);
        vertex_geometry::Grid main_settings_grid(7, 3, main_settings_rect);

        std::function<void(std::string)> username_on_confirm = [](std::string s) {};
        player_settings_ui.add_textbox("username", main_settings_grid.get_at(0, 0), colors.maroon);
        player_settings_ui.add_input_box(on_confirm, "username", main_settings_grid.get_at(2, 0), colors.orange,
                                         colors.orangered);
        player_settings_ui.add_textbox("crosshair", main_settings_grid.get_at(0, 1), colors.maroon);

        vertex_geometry::Grid input_settings_grid(10, 3, main_settings_rect);

        return player_settings_ui;
    }

    UI create_input_settings_ui() {

        vertex_geometry::Rectangle main_settings_rect = settings_menu.at(1);
        vertex_geometry::Grid main_settings_grid(7, 3, main_settings_rect);

        vertex_geometry::Grid input_settings_grid(10, 3, main_settings_rect);
        UI input_settings_ui(-0.1, batcher.absolute_position_with_colored_vertex_shader_batcher.object_id_generator);
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

        return input_settings_ui;
    }

    UI create_sound_settings_ui() {

        vertex_geometry::Rectangle main_settings_rect = settings_menu.at(1);
        vertex_geometry::Grid main_settings_grid(7, 3, main_settings_rect);

        vertex_geometry::Grid sound_settings_grid(1, 3, main_settings_rect);
        UI sound_settings_ui(-0.1, batcher.absolute_position_with_colored_vertex_shader_batcher.object_id_generator);
        sound_settings_ui.add_textbox("volume", sound_settings_grid.get_at(0, 0), colors.maroon);
        return sound_settings_ui;
    }

    UI create_graphics_settings_ui() {

        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        auto video_modes = get_available_video_modes(monitor);
        std::vector<std::string> resolutions = video_modes_to_resolutions(video_modes);

        std::function<void(std::string)> on_confirm = [&](std::string contents) { std::cout << contents << std::endl; };
        std::function<void()> on_hover = [&]() {};

        vertex_geometry::Rectangle main_settings_rect = settings_menu.at(1);
        vertex_geometry::Grid main_settings_grid(7, 3, main_settings_rect);

        std::function<void()> on_click_settings = [&]() { curr_state = {TreeState::SETTINGS, TreeState::PLAYER}; };

        std::vector<std::string> yes_no_options = {"on", "off"};

        vertex_geometry::Grid graphics_settings_grid(10, 3, main_settings_rect);
        UI graphics_settings_ui(-0.1, batcher.absolute_position_with_colored_vertex_shader_batcher.object_id_generator);

        std::vector<std::string> options = resolutions;
        std::vector<std::function<void()>> option_on_clicks(options.size(), []() {});

        std::function<void(std::string)> empty_on_click = [](std::string option) { std::cout << option << std::endl; };

        std::function<void(std::string)> resolution_dropdown_on_click = [this](std::string option) {
            size_t x_pos = option.find('x');
            unsigned int width, height;
            if (x_pos != std::string::npos) {
                width = std::stoi(option.substr(0, x_pos));
                height = std::stoi(option.substr(x_pos + 1));
                // the above verifies that indeed the things are numbers which means its valid I think... probably not
                // needed since the options are already
                configuration.set_value("graphics", "resolution", option);
            } else {
                throw std::invalid_argument("Input string is not in the correct format (e.g. 1280x960)");
            }
        };

        graphics_settings_ui.add_textbox("resolution", graphics_settings_grid.get_at(0, 0), colors.maroon);
        graphics_settings_ui.add_dropdown(on_click_settings, on_hover, graphics_settings_grid.get_at(2, 0),
                                          colors.orange, colors.orangered, options, resolution_dropdown_on_click);

        std::function<void(std::string)> fullscreen_on_click = [this](std::string option) {
            configuration.set_value("graphics", "fullscreen", option);
        };

        graphics_settings_ui.add_textbox("fullscreen", graphics_settings_grid.get_at(0, 1), colors.maroon);
        graphics_settings_ui.add_dropdown(on_click_settings, on_hover, graphics_settings_grid.get_at(2, 1),
                                          colors.orange, colors.orangered, yes_no_options, fullscreen_on_click);

        std::vector<std::string> lighting_options = {"none", "early 2000s"};
        std::vector<std::function<void()>> lighting_option_on_clicks = {[]() {}, []() {}};
        graphics_settings_ui.add_textbox("lighting", graphics_settings_grid.get_at(0, 2), colors.maroon);
        graphics_settings_ui.add_dropdown(on_click_settings, on_hover, graphics_settings_grid.get_at(2, 2),
                                          colors.orange, colors.orangered, lighting_options, empty_on_click);

        graphics_settings_ui.add_textbox("fov", graphics_settings_grid.get_at(0, 3), colors.maroon);
        graphics_settings_ui.add_input_box(on_confirm, "enter fov", graphics_settings_grid.get_at(2, 3), colors.grey,
                                           colors.lightgrey);

        std::vector<std::function<void()>> viewmodel_options_on_click = {[]() {}, []() {}};
        graphics_settings_ui.add_textbox("enable view model", graphics_settings_grid.get_at(0, 4), colors.maroon);
        graphics_settings_ui.add_dropdown(on_click_settings, on_hover, graphics_settings_grid.get_at(2, 4),
                                          colors.orange, colors.orangered, yes_no_options, empty_on_click);

        std::vector<std::function<void()>> fps_options_on_click = {[]() {}, []() {}};
        graphics_settings_ui.add_textbox("show fps", graphics_settings_grid.get_at(0, 5), colors.maroon);
        graphics_settings_ui.add_dropdown(on_click_settings, on_hover, graphics_settings_grid.get_at(2, 5),
                                          colors.orange, colors.orangered, yes_no_options, empty_on_click);

        return graphics_settings_ui;
    }

    UI create_advanced_settings_ui() {

        vertex_geometry::Rectangle main_settings_rect = settings_menu.at(1);
        vertex_geometry::Grid main_settings_grid(7, 3, main_settings_rect);

        vertex_geometry::Grid advanced_settings_grid(3, 3, main_settings_rect);
        UI advanced_settings_ui(-0.1, batcher.absolute_position_with_colored_vertex_shader_batcher.object_id_generator);
        advanced_settings_ui.add_textbox("display tick time expendature", advanced_settings_grid.get_at(0, 0),
                                         colors.maroon);
        advanced_settings_ui.add_textbox("display current ping", advanced_settings_grid.get_at(0, 1), colors.maroon);
        advanced_settings_ui.add_textbox("display movement dial", advanced_settings_grid.get_at(0, 2), colors.maroon);

        return advanced_settings_ui;
    }
};

int main() {

    // TODO I don't think I even need to use tree states here, just regular states and do the ui map approach
    // defined later down
    std::vector<TreeState> curr_state = {TreeState::MAIN_MENU};

    Window window;

    // using this to save our configuration for the future.
    Configuration configuration("assets/config/user_cfg.ini");

    configuration.register_config_handler("graphics", "resolution", [&](const std::string resolution) {
        size_t x_pos = resolution.find('x');
        unsigned int width, height;
        if (x_pos != std::string::npos) {
            width = std::stoi(resolution.substr(0, x_pos));
            height = std::stoi(resolution.substr(x_pos + 1));
            glfwSetWindowSize(window.glfw_window, width, height);
            // the above verifies that indeed the things are numbers which means its valid I think... probably not
            // needed since the options are already
            // configuration.set_value("graphics", "resolution", option);
        } else {
            throw std::invalid_argument("Input string is not in the correct format (e.g. 1280x960)");
        }
    });

    configuration.register_config_handler("graphics", "fullscreen", [&](const std::string value) {
        // TODO: if value is on / off we call window.enable/disable_fullscreen accordingly.

        configuration.register_config_handler("graphics", "fullscreen", [&](const std::string &value) {
            if (value == "on") {
                window.enable_fullscreen();
            } else if (value == "off") {
                window.disable_fullscreen();
            } else {
                std::cout << "Invalid value for fullscreen: {}" << value << std::endl;
            }
        });
    });

    configuration.apply_config_logic();

    bool start_in_fullscreen = false;
    glfwSetMouseButtonCallback(window.glfw_window, mouse_button_callback);
    glfwSetKeyCallback(window.glfw_window, key_callback);
    glfwSetCursorPosCallback(window.glfw_window, cursor_position_callback);

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

    std::function<void()> on_game_start = [&]() { curr_state = {TreeState::IN_GAME}; };
    std::function<void()> on_click_settings = [&]() { curr_state = {TreeState::SETTINGS, TreeState::PLAYER}; };
    std::function<void()> on_game_quit = [&]() { glfwSetWindowShouldClose(window.glfw_window, GLFW_TRUE); };
    std::function<void()> on_back_clicked = [&]() { curr_state = {TreeState::MAIN_MENU}; };

    std::function<void()> on_hover = [&]() {};

    UIRenderSuiteImpl ui_render_suite(batcher);

    MenuSystem menu_system(curr_state, window.glfw_window, batcher, configuration);

    std::map<StatePath, UI> game_state_to_ui = {
        {{TreeState::MAIN_MENU}, menu_system.main_menu_ui},
        {{TreeState::IN_GAME}, menu_system.in_game_ui},
        {{TreeState::SETTINGS}, menu_system.settings_menu_ui},
        {{TreeState::SETTINGS, TreeState::PLAYER}, menu_system.player_settings_ui},
        {{TreeState::SETTINGS, TreeState::INPUT}, menu_system.input_settings_ui},
        {{TreeState::SETTINGS, TreeState::SOUND}, menu_system.sound_settings_ui},
        {{TreeState::SETTINGS, TreeState::GRAPHICS}, menu_system.graphics_settings_ui},
        {{TreeState::SETTINGS, TreeState::ADVANCED}, menu_system.advanced_settings_ui},
    };

    int width, height;

    while (!glfwWindowShouldClose(window.glfw_window)) {

        glfwGetFramebufferSize(window.glfw_window, &width, &height);

        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto ndc_mouse_pos = get_ndc_mouse_pos(window.glfw_window, mouse_position_x, mouse_position_y);

        // here we render all ui's and ui's that are
        // ancestor ones
        std::vector<TreeState> incremental_path;
        for (const auto ts : curr_state) {
            incremental_path.push_back(ts);
            if (game_state_to_ui.find(incremental_path) != game_state_to_ui.end()) {
                UI &ancestor_ui = game_state_to_ui.at(incremental_path);

                // TODO: use input state thing and then
                // actually fill this out with non mock data

                process_key_pressed_this_tick(ancestor_ui); // because I yet use the
                                                            // InputState

                process_and_queue_render_ui(ndc_mouse_pos, ancestor_ui, ui_render_suite, {}, false, false,
                                            mouse_just_clicked);
            }
        }

        batcher.absolute_position_with_colored_vertex_shader_batcher.draw_everything();

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
