#include "Mode.hpp"
#include <png.h>

#include "Scene.hpp"
#include "Sound.hpp"
#include "DrawLines.hpp"
#include "Buttons.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <list>
#include <chrono>

#include <hb.h>
#include <hb-ft.h>
#include <queue>


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
	} left, right, down, up, space, switch_camera;


	struct Cricket {

		int cricketID;
		Scene::Transform *transform;
		float lifeSpan;
		float matureAge;
		float hatchAge;
		bool is_healthy = true;

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
	std::priority_queue<std::pair<float, std::string>> notif_pq;
	void display_notification(std::string filename);
	void schedule_notification(std::string filename, float in_time);
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
	float current_elapsed = 0.0;
	float max_elapsed = 0.02;

	// Tutorial
	bool first_time_food = false; 
	bool first_time_babies = false; 
	bool first_time_eggs = false;
	bool first_time_matured = false; 
	bool first_time_starved = false;
	bool first_time_sick = false;
	bool first_time_950_dollars = false;
	bool first_time_alt_view = false;

	Scene::Transform *first_cricket; 
	std::vector<Cricket> Crickets;
	float total_elapsed = 0.f;
	float cricketEatingRate = 0.01f;
	size_t numEggs = 0;
	size_t numBabyCrickets = 0;
	size_t numMatureCrickets = 0;
	size_t numDeadCrickets = 0;
	float totalMoney = 220.f;
	float totalFood = 200.f;
	size_t eggPrice = 200;
	size_t foodPrice = 200;
	bool gameOver = false;

	// PNG
	struct texture {
   GLuint id;
   GLenum format;
   GLenum min_filter;
   GLenum mag_filter;
   uint16_t w, h;
 	} strawberry_tex, egg_tex, dollars_tex, button_clicked_tex, button_unclicked_tex, board_tex;
	static int png_to_gl_texture(struct texture * tex, std::string filename);
	void draw_textured_quad(struct texture * tex, float x0, float y0, glm::uvec2 const &drawable_size);
	GLuint png_program;


	// Sound
	std::shared_ptr< Sound::PlayingSample > chirping_loop;

	void invoke_callback(Button_UI::call_back);
	bool buy_food();
	bool buy_eggs();
	bool sell_mature();

	std::vector<Popup_UI> popups;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;
	
	// Button Functions
	UI game_UI;

	//cameras:
	Scene::Camera *main_camera = nullptr;
	Scene::Camera *alt_camera = nullptr;
	bool alt_view = false;
};