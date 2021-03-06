#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wswitch-default"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <dear/imgui.h>
#include <dear/imgui_impl_glfw.h>
#include <dear/imgui_impl_opengl3.h>

#pragma GCC diagnostic pop

#include "common_constants.h"
#include "graphicsmanager.h"
#include "mainmenustate.h"


bool window_init(GLFWwindow*&);


int main(void) {
    GLFWwindow* window;
    if (!window_init(window)) {
        return -1;
    }

    GraphicsManager gfx(window);

    std::unique_ptr<GameState> current_state = std::make_unique<MainMenuState>();
    current_state->set_graphics(&gfx);
    current_state->set_csp(&current_state);

    // ImGui init

    const char* glsl_version = "#version 330";

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGui::StyleColorsDark();

    // It's convenient to keep the demo code in here,
    // for when we want to explore ImGui features
    bool show_demo_window = false;

    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);

    glfwSwapInterval(0);

    while(!glfwWindowShouldClose(window)) {
        auto time_0 = std::chrono::high_resolution_clock::now();

        glfwPollEvents();

        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.2, 0, 0.3, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (!current_state) {
            break;
        }

        current_state->check_for_quit();
        current_state->main_loop();

        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);

        auto time_1 = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(20) - (time_1 - time_0));
    }

    glfwTerminate();
    return 0;
}

bool window_init(GLFWwindow*& window) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, false);

    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sokoban 3D", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    return true;
}
