#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Button_UI {
	glm::vec2 anchor;		// upper-left corner
	glm::vec2 dimension;	// width and height
	std::string text;

    unsigned int TextureID;
    unsigned int program;
    unsigned int VAO, VBO;

	enum call_back {
		BUY_FOOD = 0,
		SELL_MATURE = 1,
		BUY_EGG = 2
	};
	call_back trigger_event;

	Button_UI(glm::vec2 _anchor, glm::vec2 _dimension, std::string _text, call_back _trigger_event);

	void draw_button();
    void interact(int32_t x, int32_t y, bool is_down);
};