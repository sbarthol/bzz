#include "GL.hpp"
#include "gl_errors.hpp"
#include "gl_compile_program.hpp"

#include <string>
#include <vector>

#include "Buttons.hpp"
#include "PlayMode.hpp"

UI::UI(PlayMode* _game) : game(_game) {
    program = gl_compile_program(
        // vertex shader
        "#version 330\n"
        "layout (location = 0) in vec4 vertex;\n" // <vec2 pos, vec2 tex>
        "out vec2 TexCoords;\n"
        "\n"
        "uniform mat4 projection;\n"
        "\n"
        "void main() {\n"
        "    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
        "    TexCoords = vertex.zw;\n"
        "}\n",

        // fragment shader
        "#version 330\n"
        "in vec2 TexCoords;\n"
        "out vec4 color;\n"
        "\n"
        "uniform sampler2D text;\n"
        "uniform vec3 textColor;\n"
        "\n"
        "void main() {\n"
        "    color = vec4(textColor, 1.0);\n"
        "}\n"
    );

    // hard-coded SCR_WIDTH and SCR_HEIGHT
    unsigned int SCR_WIDTH = 1280;
    unsigned int SCR_HEIGHT = 720;
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), static_cast<float>(SCR_HEIGHT), 0.0f);
    glUseProgram(program);
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUseProgram(0);

    // set up VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // initialize button arrays
    buttons.emplace_back(glm::vec2(100,100), glm::vec2(100, 50), "Buy Food", Button_UI::BUY_FOOD);
	buttons.emplace_back(glm::vec2(100,200), glm::vec2(100, 50), "Buy Egg", Button_UI::BUY_EGG);
	buttons.emplace_back(glm::vec2(100,300), glm::vec2(100, 50), "Sell Mature", Button_UI::SELL_MATURE);

    GL_ERRORS();
}

Button_UI::Button_UI(glm::vec2 _anchor, glm::vec2 _dimension, std::string _text, call_back _trigger_event)	:
    anchor(_anchor), dimension(_dimension), text(_text), trigger_event(_trigger_event) {

    // set up the texture
    const int32_t height = 500;
    const int32_t width = 500;
    // set the color to white (255, 255, 255)
    // for now it doesn't matter since fragment shader
    // doesn't sample from the texture
    uint8_t pure_color[height][width][3] = {(uint8_t)-1};
    glGenTextures(1, &TextureID);
    glBindTexture(GL_TEXTURE_2D, TextureID);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        width,
        height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        pure_color
    );
    // texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    GL_ERRORS();
}

void UI::draw() {
    glUseProgram(program);

    glm::vec3 color = glm::vec3(0.91f, 0.93f, 0.79f);
    glUniform3f(glGetUniformLocation(program, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    for (auto &button : buttons) {
        glm::vec2 anchor = button.anchor;
        glm::vec2 dimension = button.dimension;
        unsigned int TextureID = button.TextureID;

        float vertices[6][4] = {
            { anchor.x,               anchor.y + dimension.y, 0.0f, 0.0f }, // lower-left         
            { anchor.x,               anchor.y,               0.0f, 1.0f }, // upper-left
            { anchor.x + dimension.x, anchor.y,               1.0f, 1.0f },

            { anchor.x,               anchor.y + dimension.y, 0.0f, 0.0f },
            { anchor.x + dimension.x, anchor.y,               1.0f, 1.0f },
            { anchor.x + dimension.x, anchor.y + dimension.y, 1.0f, 0.0f }  // lower-right
        };
        glBindTexture(GL_TEXTURE_2D, TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindTexture(GL_TEXTURE_2D, 0);

        // draw texts
    }
    glBindVertexArray(0);
    glUseProgram(0);

    GL_ERRORS();
}

void UI::update(int32_t x, int32_t y, bool is_down) {
    for (auto &button : buttons) {
        glm::vec2 x_range(button.anchor.x, button.anchor.x + button.dimension.x);
        glm::vec2 y_range(button.anchor.y, button.anchor.y + button.dimension.y);

        if (x >= x_range.x && x <= x_range.y &&
            y >= y_range.x && y <= y_range.y) {
                
            if (is_down)
                game->invoke_callback(button.trigger_event);
        }
    }
}