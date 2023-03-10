#include "Mode.hpp"
#include <png.h>

#include "Scene.hpp"
#include "Sound.hpp"
#include "DrawLines.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <list>
#include <chrono>

#include <hb.h>
#include <hb-ft.h>
#include <queue>
#include <functional>


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
	PlayMode(glm::uvec2 window_size_);
	virtual ~PlayMode();

	glm::uvec2 window_size;

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space, switch_camera, turn_off_tutorial;


	struct Cricket {

		int cricketID;
		Scene::Transform *transform;
		float lifeSpan;
		float matureAge;
		float hatchAge;
		float juicyAge;
		float initialStarvationRobustness;
		float currentstarvationRobustness;
		bool is_healthy = true;
		bool is_juicy = false;
		bool is_attacking = false;

		float age = 0.f;

		static int seq;

		bool is_egg() {
			return !is_dead() && age < hatchAge;
		}

		bool is_mature() {
			return !is_dead() && age >= matureAge;
		}

		bool is_dead() {
			return !is_healthy || age >= lifeSpan;
		}

		bool is_baby() {
			return !is_dead() && hatchAge <= age && age < matureAge;
		}

		Cricket(int cricketID_, Scene::Transform *transform_ ): cricketID(cricketID_), transform(transform_)  {}
	};

	// Visual
	void spawn_cricket();
	void kill_cricket(Cricket &cricket);
	void mature_cricket(Cricket &cricket);
	void hatch_cricket(Cricket &cricket);
	Scene::Transform *adult_cricket_transform;
	Scene::Transform *baby_cricket_transform;
	Scene::Transform *strawberry_transform;
	Scene::Transform *bedding_transform;
	Scene::Transform *camera_body_transform;
	float elapsed_since_spawn = 0.0;
	std::list<Scene::Transform*> strawberry_transforms;
	Scene::Transform* spawn_strawberry();
	glm::vec3 bedding_min;
	glm::vec3 bedding_max;

	// Filled rectangles
	GLuint rect_program;
	void draw_filled_rect(glm::vec2 lower_left, glm::vec2 upper_right, glm::vec4 color);
	
	// Notification
	int lambda_counter = 0;
	std::function<void(void)> all_lambdas[30];
	std::priority_queue<std::pair<float, int>> lambda_pq;
	void display_notification(std::string filename);
	void schedule_lambda(std::function<void(void)> f, float in_time);
	void hide_current_notification();
	bool notification_active = false;
	std::vector<std::string> notification_text;

	// Text drawing
	GLuint text_program;
	FT_Face ft_face;
	FT_Library ft_library;
	hb_font_t *hb_font;
	void draw_text_line(std::string s, glm::uvec2 const &drawable_size, float x, float y);
	void draw_text_lines(glm::uvec2 const &drawable_size, float x, float y);
	void draw_stats(glm::uvec2 const &drawable_size, float x, float y);
	std::string load_text_from_file(std::string filename);
	size_t total_letters{0};
	size_t letter_counter{0};
	float current_elapsed = 0.0f;
	float max_elapsed = 0.02f;

	// Tutorial
	bool toggle = false;
	bool first_time_radio = false; 
	bool first_time_food = false; 
	bool first_time_babies = false; 
	bool first_time_eggs = false;
	bool first_time_matured = false; 
	bool first_time_starved = false;
	bool first_time_sick = false;
	bool first_time_500_dollars = false;
	bool first_time_alt_view = false;
	bool first_time_steroids = false;
	bool first_time_too_big = false;
	bool first_time_is_attacking = false;
	bool first_time_cage_too_small = false;
	bool alt_camera_bought = false;

	Scene::Transform *first_cricket; 
	std::vector<Cricket> Crickets;
	float total_elapsed = 0.f;
	float cricketEatingRate = 0.8f;
	size_t numEggs = 0;
	size_t numBabyCrickets = 0;
	size_t numMatureCrickets = 0;
	size_t numDeadCrickets = 0;
	size_t cageCapacity = 50; 
	size_t unitEggs = 20;
	float totalMoney = 250.f;
	float totalFood = 400.f;
	float eggPrice = 200.f;
	float foodPrice = 100.f;
	float cricketPrice = 50.f;
	bool gameOver = false;

	// PNG
	struct texture {
		GLuint id;
		GLenum format;
		GLenum min_filter;
		GLenum mag_filter;
		uint16_t w, h;
 	} board_tex, lens_view_tex;
	static int png_to_gl_texture(PlayMode::texture * tex, std::string filename);
	static void draw_textured_quad(PlayMode::texture * tex, float x0, float y0, glm::uvec2 const &drawable_size);
	static inline GLuint png_program = 0;

	// Sound
	std::shared_ptr< Sound::PlayingSample > chirping_loop;

	// Buttons
	struct Button_UI {
		PlayMode* game;

		glm::vec2 anchor;
		texture pressedTex;
		texture unpressedTex;
		texture icon;

		// button state
		bool pressed;
		bool clicked;

		enum call_back {
			BUY_FOOD = 0,
			SELL_MATURE = 1,
			BUY_EGG = 2,
			REMOVE_DEAD = 3,
			BUY_CAMERA = 4,
			BUY_RADIO = 5,
			BUY_STEROIDS = 6,
			UPGRADE_CAGE = 7
		};
		call_back trigger_event;

		Button_UI(PlayMode* _game, std::string icon_png, call_back _trigger_event);

		void draw(glm::uvec2 const &drawable_size);
		void interact(int mouse_x, int mouse_y, glm::vec2 drawable_size);
		void set_pressed(int mouse_x, int mouse_y, glm::vec2 drawable_size);
	};
	void invoke_callback(Button_UI::call_back);
	bool buy_food();
	bool buy_eggs();
	bool sell_mature();
	bool upgrade_cage();
	bool remove_dead_crickets();
	bool is_at_capacity() {
		size_t total = numBabyCrickets + numMatureCrickets+ numDeadCrickets + numEggs;
		if (total < unitEggs) return false;
		return (total + unitEggs) >= cageCapacity;
	}
	std::vector<Popup_UI> popups;
	std::vector<Button_UI> buttons;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//cameras:
	Scene::Camera *main_camera = nullptr;
	Scene::Camera *alt_camera = nullptr;
	bool alt_view = false;
};