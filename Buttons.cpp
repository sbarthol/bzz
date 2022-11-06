#include "GL.hpp"
#include "gl_compile_program.hpp"

#include <string>
#include <vector>

#include "Buttons.hpp"

void Button_UI::Button_UI()	:
    anchor(_anchor), dimension(_dimension), text(_text), trigger_event(_trigger_event) {
    
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

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    glUseProgram(program);
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUseProgram(0);

    // set up the texture
    const int32_t height = 500;
    const int32_t width = 500;
    // set the color to white (255, 255, 255)
    // for now it doesn't matter since fragment shader
    // doesn't sample from the texture
    pure_color[height][width][3] = {(int8_t)-1};
    glGenTextures(1, &TextureID);
    glBindTexture(GL_TEXTURE_2D, TextureID);
    glTexImage2D {
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        width,
        height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        pure_color
    };
    // texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

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

    GL_ERRORS();
}

void Button_UI::draw_button() {
    glUseProgram(program);

    glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);
    glUniform3f(glGetUniformLocation(program, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    float vertices[6][4] = {
        { anchor.x,               anchor.y,               0.0f, 0.0f }, // lower-left         
        { anchor.x,               anchor.y + dimension.y, 0.0f, 1.0f }, // upper-left
        { anchor.x + dimension.x, anchor.y + dimension.y, 1.0f, 1.0f },

        { anchor.x,               anchor.y,               0.0f, 0.0f },
        { anchor.x + dimension.x, anchor.y + dimension.y, 1.0f, 1.0f },
        { anchor.x + dimension.x, anchor.y,               1.0f, 0.0f }  // lower-right
    };
    glBindTexture(GL_TEXTURE_2D, TextureID);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindVertexArray(0);
    glUseProgram(0);

    GL_ERRORS();
}

void Button_UI::interact(int32_t x, int32_t y, bool is_down) {}