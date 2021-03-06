#ifndef GRAPHICSMANAGER_H
#define GRAPHICSMANAGER_H

#include "color_constants.h"
#include "shader.h"

class GLFWwindow;

// Reading off the texture atlas starting from top, left to right
enum class Texture {
    LightEdges = 0,
    Corners = 1,
    BrokenEdges = 2,
    Edges = 3,
    Cross = 4,
    SwitchUp = 5,
    SwitchDown = 6,
    Blank = 7,
    AutoBlock = 8,
    Car = 12,
};

Texture operator |(Texture a, Texture b);

class GraphicsManager {
public:
    GraphicsManager(GLFWwindow*);
    GLFWwindow* window();

    void set_model(glm::mat4);
    void set_view(glm::mat4);
    void set_projection(glm::mat4);
    void set_color(Color4);
    void set_tex(Texture);

    void draw_cube();
    //void draw_trail();

private:
    GLFWwindow* window_;
    Shader shader_;

    glm::mat4 model_;
    glm::mat4 view_;
    glm::mat4 projection_;

    Color4 color_;
    Texture tex_;

    void init_vertex_attributes();
    void init_cube_buffer();
    void load_texture_atlas();
};

#endif // GRAPHICSMANAGER_H
