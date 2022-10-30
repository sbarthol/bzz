#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"
#include "DrawLines.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct Button_UI {
	glm::vec2 anchor;		// upper-left corner
	glm::vec2 dimension;	// width and height
	std::string text;

	enum call_back {
		BUY_FOOD = 0,
		SELL_MATURE = 1,
		BUY_EGG
	};

	call_back trigger_event;

	Button_UI(glm::vec2 _anchor, glm::vec2 _dimension, std::string _text, call_back _trigger_event)
	 : anchor(_anchor), dimension(_dimension), text(_text), trigger_event(_trigger_event)  {}

	void draw_button(DrawLines &lines);
};

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	// Visual
	void spawn_cricket();
	Scene::Transform *cricket_transform;
	Scene::Transform *bedding_transform;
	float elapsed_since_spawn = 0.0;

	struct Cricket {

		int cricketID;
		Scene::Transform *transform;
		float lifeSpan = 5.0; // is there a constant lifespan
		float matureAge = 3.5; //age that cricket is mature
		// or is the time of death determined by environment?
		float age = 0.f;

		static int seq;

		Cricket(int cricketID_, Scene::Transform *transform_ ): cricketID(cricketID_), transform(transform_)  {}
	};

	Scene::Transform *first_cricket; 
	std::vector<Cricket> Crickets;
	float total_elapsed = 0.0;
	size_t numLiveCrickets = 0;
	float cricketEatingRate = .2f;
	size_t numBabyCrickets = 0;
	size_t numMatureCrickets = 0;
	size_t numDeadCrickets = 0;
	float totalMoney = 0;
	float totalFood = 2e9;

	// Buttons
	std::vector<Button_UI> buttons;
	void invoke_callback(Button_UI::call_back);
	void buy_food();
	void buy_eggs();
	void sell_mature();

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;
	
	//camera:
	Scene::Camera *camera = nullptr;

};