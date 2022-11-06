#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"
#include "DrawLines.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <list>
#include <chrono>


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

struct Popup_UI {
	glm::vec2 anchor;
	std::string text;
	glm::u8vec4 color;

	float duration;
	std::chrono::time_point<std::chrono::high_resolution_clock> start_time;

	Popup_UI(glm::vec2 _anchor, std::string _text, glm::u8vec4 _color, float _duration) 
	 : anchor(_anchor), text(_text), color(_color), duration(_duration), start_time(std::chrono::high_resolution_clock::now()) {}

	void draw(DrawLines &lines);
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


	struct Cricket {

		int cricketID;
		Scene::Transform *transform;
		float lifeSpan = 100.f; // is there a constant lifespan
		float matureAge = 20.f; //age that cricket is mature
		bool starved = false;
		// or is the time of death determined by environment?
		float age = 0.f;

		static int seq;

		bool is_mature() {
			return !is_dead() && age >= matureAge;
		}

		bool is_dead() {
			return starved || age >= lifeSpan;
		}

		bool is_baby() {
			return !is_dead() && age < matureAge;
		}

		Cricket(int cricketID_, Scene::Transform *transform_ ): cricketID(cricketID_), transform(transform_)  {}
	};

	// Visual
	void spawn_cricket();
	void kill_cricket(Cricket &cricket);
	void mature_cricket(Cricket &cricket);
	Scene::Transform *adult_cricket_transform;
	Scene::Transform *baby_cricket_transform;
	Scene::Transform *strawberry_transform;
	Scene::Transform *bedding_transform;
	float elapsed_since_spawn = 0.0;
	std::list<Scene::Transform*> strawberry_transforms;
	Scene::Transform* spawn_strawberry();
	glm::vec3 bedding_min;
	glm::vec3 bedding_max;

	Scene::Transform *first_cricket; 
	std::vector<Cricket> Crickets;
	float total_elapsed = 0.f;
	float cricketEatingRate = 0.01f;
	size_t numBabyCrickets = 0;
	size_t numMatureCrickets = 0;
	size_t numDeadCrickets = 0;
	float totalMoney = 1000.f;
	float totalFood = 20.f;

	// Sound
	std::shared_ptr< Sound::PlayingSample > chirping_loop;

	// Buttons
	std::vector<Button_UI> buttons;
	void invoke_callback(Button_UI::call_back);
	bool buy_food();
	bool buy_eggs();
	bool sell_mature();

	std::vector<Popup_UI> popups;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;
	
	//camera:
	Scene::Camera *camera = nullptr;

};