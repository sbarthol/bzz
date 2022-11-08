#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

struct PlayMode;

struct Button_UI {
	glm::vec2 anchor;		// upper-left corner
	glm::vec2 dimension;	// width and height
	std::string text;
    unsigned int TextureID;

	// button state
	bool highlight;			// On mouse hover
	bool active;			// Whether the button is clicable

	enum call_back {
		BUY_FOOD = 0,
		SELL_MATURE = 1,
		BUY_EGG = 2
	};
	call_back trigger_event;

	Button_UI(glm::vec2 _anchor, glm::vec2 _dimension, std::string _text, call_back _trigger_event);
};

struct UI {
    unsigned int program;
    unsigned int VAO, VBO;
	PlayMode* game;

	std::vector<Button_UI> buttons;

	UI(PlayMode* _game);
	void draw();
	void update(int32_t x, int32_t y, bool is_down);
};