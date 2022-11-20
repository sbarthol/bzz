#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "GL.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <unordered_set>
#include <fstream>
#include <iostream>
#include <errno.h>
#include <stdio.h>

static int png_to_gl_texture(PlayMode::texture * tex, std::string filename);
static void draw_textured_quad(PlayMode::texture * tex, float x0, float y0, glm::uvec2 const &drawable_size);
static GLuint png_program;

int PlayMode::Cricket::seq = 0;

GLuint bzz_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > bzz_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("bzz.pnct"));
	bzz_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > bzz_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("bzz.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = bzz_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = bzz_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

		if(mesh_name == "Bedding") {
			drawable.pipeline.blend = true;
			struct PlayMode::texture tex;
			int ret = png_to_gl_texture(&tex, data_path("../scenes/sand.png"));
  		if(ret) {
			printf("%s\n", data_path("../scenes/sand.png").c_str());
  			printf("Cannot load texture, error code %d.\n", ret);
    		abort();
  		}
			drawable.pipeline.textures[0].texture = tex.id;
		} 
		if(mesh_name == "Floor") {
			drawable.pipeline.blend = true;
			struct PlayMode::texture tex;
			int ret = png_to_gl_texture(&tex, data_path("../scenes/floor.png"));
  		if(ret) {
			printf("%s\n", data_path("../scenes/floor.png").c_str());
  			printf("Cannot load texture, error code %d.\n", ret);
    		abort();
  		}
			drawable.pipeline.textures[0].texture = tex.id;
		} 
		if(mesh_name == "Walls") {
			drawable.pipeline.blend = true;
			struct PlayMode::texture tex;
			int ret = png_to_gl_texture(&tex, data_path("../scenes/wallpaper.png"));
  		if(ret) {
			printf("%s\n", data_path("../scenes/wallpaper.png").c_str());
  			printf("Cannot load texture, error code %d.\n", ret);
    		abort();
  		}
			drawable.pipeline.textures[0].texture = tex.id;
		} 
		if(mesh_name == "SoilBag") {
			drawable.pipeline.blend = true;
			struct PlayMode::texture tex;
			int ret = png_to_gl_texture(&tex, data_path("../scenes/soilbag.png"));
  		if(ret) {
  			printf("Cannot load texture, error code %d.\n", ret);
    		abort();
  		}
			drawable.pipeline.textures[0].texture = tex.id;
		} 
		if(mesh_name == "Table") {
			drawable.pipeline.blend = true;
			struct PlayMode::texture tex;
			int ret = png_to_gl_texture(&tex, data_path("../scenes/table.png"));
  		if(ret) {
  			printf("Cannot load texture, error code %d.\n", ret);
    		abort();
  		}
			drawable.pipeline.textures[0].texture = tex.id;
		} 
		if(mesh_name == "Frame") {
			drawable.pipeline.blend = true;
			struct PlayMode::texture tex;
			int ret = png_to_gl_texture(&tex, data_path("../scenes/frame.png"));
  		if(ret) {
  			printf("Cannot load texture, error code %d.\n", ret);
    		abort();
  		}
			drawable.pipeline.textures[0].texture = tex.id;
		} 
		if(mesh_name == "Fruit") {
			drawable.pipeline.blend = true;
			struct PlayMode::texture tex;
			int ret = png_to_gl_texture(&tex, data_path("../scenes/fruit.png"));
  		if(ret) {
  			printf("Cannot load texture, error code %d.\n", ret);
    		abort();
  		}
			drawable.pipeline.textures[0].texture = tex.id;
		} 
		if(mesh_name == "Soil") {
			drawable.pipeline.blend = true;
			struct PlayMode::texture tex;
			int ret = png_to_gl_texture(&tex, data_path("../scenes/soil.png"));
  		if(ret) {
  			printf("Cannot load texture, error code %d.\n", ret);
    		abort();
  		}
			drawable.pipeline.textures[0].texture = tex.id;
		} 
		if(mesh_name == "Lens") {
			drawable.pipeline.blend = true;
			struct PlayMode::texture tex;
			int ret = png_to_gl_texture(&tex, data_path("../scenes/lens.png"));
  		if(ret) {
  			printf("Cannot load texture, error code %d.\n", ret);
    		abort();
  		}
			drawable.pipeline.textures[0].texture = tex.id;
		} 
		if(mesh_name == "Crate") {
			drawable.pipeline.blend = true;
			struct PlayMode::texture tex;
			int ret = PlayMode::png_to_gl_texture(&tex, data_path("../scenes/crate.png"));
  		if(ret) {
  			printf("Cannot load texture, error code %d.\n", ret);
    		abort();
  		}
			drawable.pipeline.textures[0].texture = tex.id;
		}
	});
});

// https://stackoverflow.com/questions/5289613/generate-random-float-between-two-floats
float get_rng_range(float a, float b) {
	float random = ((float) rand()) / (float) RAND_MAX;
  float diff = b - a;
  float r = random * diff;
  return a + r;
}

Load< Sound::Sample > chirping_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("chirping.wav"));
});

Load< Sound::Sample > click_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("click.wav"));
});

Load< Sound::Sample > cash_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("cash.wav"));
});

Load< Sound::Sample > background_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("background_music.wav"));
});

Load< Sound::Sample > click_error_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("access_denied.wav"));
});


Scene::Transform* PlayMode::spawn_strawberry() {

	Mesh const &mesh = bzz_meshes->lookup("Strawberry");

	scene.transforms.emplace_back();

	Scene::Transform *transform = &scene.transforms.back();

	const float eps = 0.3f;
	transform->position = glm::vec3(get_rng_range(bedding_min.x + eps, bedding_max.x - eps), get_rng_range(bedding_min.y + eps, bedding_max.y - eps), strawberry_transform->position.z);
	transform->rotation = glm::angleAxis(glm::radians(get_rng_range(0.f,360.f)), glm::vec3(0.0,0.0,1.0));
	transform->scale = glm::vec3(1.f);
	transform->name = "Strawberry";

	scene.drawables.emplace_back(Scene::Drawable(transform));
	Scene::Drawable &drawable = scene.drawables.back();

	drawable.pipeline = lit_color_texture_program_pipeline;

	drawable.pipeline.vao = bzz_meshes_for_lit_color_texture_program;
	drawable.pipeline.type = mesh.type;
	drawable.pipeline.start = mesh.start;
	drawable.pipeline.count = mesh.count;

	return transform;
}

void PlayMode::spawn_cricket() {

	Mesh const &mesh = bzz_meshes->lookup("BabyCricket");

	scene.transforms.emplace_back();

	Scene::Transform *transform = &scene.transforms.back();
	Cricket cricket = Cricket(Cricket::seq++, transform);

	cricket.hatchAge = get_rng_range(3.f, 7.f);
	cricket.matureAge = get_rng_range(17.f, 23.f);
	cricket.lifeSpan = get_rng_range(80.f, 110.f);
	
	Crickets.push_back(cricket);

	transform->position = glm::vec3(get_rng_range(bedding_min.x,bedding_max.x), get_rng_range(bedding_min.y,bedding_max.y), baby_cricket_transform->position.z);
	transform->rotation = glm::angleAxis(glm::radians(get_rng_range(0.f,360.f)), glm::vec3(0.0,0.0,1.0));
	transform->scale = glm::vec3(0.f);
	transform->name = "Cricket_" + std::to_string(cricket.cricketID);

	scene.drawables.emplace_back(Scene::Drawable(transform));
	Scene::Drawable &drawable = scene.drawables.back();

	drawable.pipeline = lit_color_texture_program_pipeline;

	drawable.pipeline.vao = bzz_meshes_for_lit_color_texture_program;
	drawable.pipeline.type = mesh.type;
	drawable.pipeline.start = mesh.start;
	drawable.pipeline.count = mesh.count;

}

void PlayMode::kill_cricket(Cricket &cricket) {
	glm::vec3 axis = glm::normalize(glm::vec3(0.f, 1.f, 0.f));
	glm:: quat turn_upside_down = glm::angleAxis(glm::radians(180.f), axis);
	cricket.transform->rotation = glm::normalize(cricket.transform->rotation * turn_upside_down);
}


PlayMode::PlayMode(glm::uvec2 window_size_) : window_size(window_size_), scene(*bzz_scene){


	for (auto &transform : scene.transforms) {

		if (transform.name == "AdultCricket") {
			adult_cricket_transform = &transform;
			adult_cricket_transform->scale = glm::vec3(0.f);
		}
		if (transform.name == "BabyCricket") {
			baby_cricket_transform = &transform;
			baby_cricket_transform->scale = glm::vec3(0.f);
		}
		if (transform.name == "Strawberry") {
			strawberry_transform = &transform;
			strawberry_transform->scale = glm::vec3(0.f);
		}
		if (transform.name == "Bedding") {
			bedding_transform = &transform;
		}
		if (transform.name == "CameraBody") {
			camera_body_transform = &transform;
		}
	}

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 2) throw std::runtime_error("Expecting scene to have exactly two cameras, but it has " + std::to_string(scene.cameras.size()));
	main_camera = &scene.cameras.front();
	alt_camera = &scene.cameras.back();

	if(alt_camera->transform->position.z < main_camera->transform->position.z) {
		std::swap(main_camera, alt_camera);
	}

	// Loop chirping sound and background music
	// chirping_loop = Sound::loop(*chirping_sample, 1.0f, 0.0f);
	// chirping_loop.
	Sound::loop(*background_sample, 1.0f, 0.0f);

	Mesh const &mesh = bzz_meshes->lookup("Bedding");
	glm::mat4x3 to_world = bedding_transform->make_local_to_world();
	bedding_min = to_world * glm::vec4(mesh.min, 1.f);
	bedding_max = to_world * glm::vec4(mesh.max, 1.f);

	if(bedding_min.x > bedding_max.x) {
		std::swap(bedding_min.x, bedding_max.x);
	}
	if(bedding_min.y > bedding_max.y) {
		std::swap(bedding_min.y, bedding_max.y);
	}

	const char *VERTEX_SHADER = ""
        "#version 330\n"
        "in vec2 position;\n"
        "void main(void) {\n"
        "    gl_Position = vec4(position.xy, 0, 1);\n"
        "}\n";


	const char *FRAGMENT_SHADER = ""
        "#version 330\n"
				"uniform vec4 fillColor;\n"
				"out vec4 fragColor;\n"
        "void main(void) {\n"
        "    fragColor = fillColor;\n"
        "}\n";

	GLuint vs{0}, fs{0};
	vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &VERTEX_SHADER, 0);
  glCompileShader(vs);

  fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &FRAGMENT_SHADER, 0);
  glCompileShader(fs);

  rect_program= glCreateProgram();
  glAttachShader(rect_program, vs);
  glAttachShader(rect_program, fs);
  glLinkProgram(rect_program);

	VERTEX_SHADER = ""
        "#version 330\n"
        "in vec4 position;\n"
        "out vec2 texCoords;\n"
        "void main(void) {\n"
        "    gl_Position = vec4(position.xy, 0, 1);\n"
        "    texCoords = position.zw;\n"
        "}\n";


	FRAGMENT_SHADER = ""
        "#version 330\n"
				"uniform vec4 fillColor;\n"
        "uniform sampler2D tex;\n"
        "in vec2 texCoords;\n"
        "out vec4 fragColor;\n"
        "void main(void) {\n"
        "    fragColor = vec4(1, 1, 1, texture(tex, texCoords).r) * fillColor;\n"
        "}\n";

	vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &VERTEX_SHADER, 0);
  glCompileShader(vs);
	GL_ERRORS();

  fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &FRAGMENT_SHADER, 0);
  glCompileShader(fs);
	GL_ERRORS();

  text_program = glCreateProgram();
  glAttachShader(text_program, vs);
  glAttachShader(text_program, fs);
  glLinkProgram(text_program);
	GL_ERRORS();

	// Initialize text data structures
	#define FONT_SIZE 50

	// https://www.1001fonts.com/november-font.html
	std::string fontfile = data_path("../scenes/novem___.ttf");

	/* Initialize FreeType and create FreeType font face. */
  FT_Error ft_error;

  ft_error = FT_Init_FreeType (&ft_library);
  if (ft_error)
    abort();
  ft_error = FT_New_Face (ft_library, fontfile.c_str(), 0, &ft_face);
  if (ft_error)
    abort();
  ft_error = FT_Set_Char_Size (ft_face, FONT_SIZE*64, FONT_SIZE*64, 0, 0);
  if (ft_error)
    abort();

  /* Create hb-ft font. */
  hb_font = hb_ft_font_create (ft_face, NULL);

	// png program
	VERTEX_SHADER = ""
        "#version 330\n"
        "in vec4 position;\n"
        "out vec2 texCoords;\n"
        "void main(void) {\n"
        "    gl_Position = vec4(position.xy, 0, 1);\n"
        "    texCoords = position.zw;\n"
        "}\n";


	FRAGMENT_SHADER = ""
        "#version 330\n"
        "uniform sampler2D tex;\n"
        "in vec2 texCoords;\n"
        "out vec4 fragColor;\n"
        "void main(void) {\n"
				"    fragColor = texture(tex, texCoords);\n"
        "}\n";

	vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &VERTEX_SHADER, 0);
  glCompileShader(vs);
	GL_ERRORS();

  fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &FRAGMENT_SHADER, 0);
  glCompileShader(fs);
	GL_ERRORS();

  png_program = glCreateProgram();
  glAttachShader(png_program, vs);
  glAttachShader(png_program, fs);
  glLinkProgram(png_program);
	GL_ERRORS();

	// set up buttons
	buttons.emplace_back(glm::vec2(-0.9f, 0.5f), "../scenes/strawberry.png", Button_UI::BUY_FOOD);
	buttons.emplace_back(glm::vec2(-0.9f + 0.2f, 0.5f), "../scenes/egg.png", Button_UI::BUY_EGG);
	buttons.emplace_back(glm::vec2(-0.9f + 0.4f, 0.5f), "../scenes/dollars.png", Button_UI::SELL_MATURE);

	// load some png textures
// 	int ret = png_to_gl_texture(&button_clicked_tex, data_path("../scenes/button_clicked.png"));
//   if(ret) {
//   	printf("Cannot load texture, error code %d.\n", ret);
//     abort();
//   }
// 	ret = png_to_gl_texture(&button_unclicked_tex, data_path("../scenes/button_unclicked.png"));
//   if(ret) {
//   	printf("Cannot load texture, error code %d.\n", ret);
//     abort();
//   }
// 	ret = png_to_gl_texture(&strawberry_tex, data_path("../scenes/strawberry.png"));
//   if(ret) {
//   	printf("Cannot load texture, error code %d.\n", ret);
//     abort();
//   }
// 	ret = png_to_gl_texture(&egg_tex, data_path("../scenes/egg.png"));
//   if(ret) {
//   	printf("Cannot load texture, error code %d.\n", ret);
//     abort();
//   }
// 	ret = png_to_gl_texture(&dollars_tex, data_path("../scenes/dollars.png"));
//   if(ret) {
//   	printf("Cannot load texture, error code %d.\n", ret);
//     abort();
//   }
	int ret;
	ret = png_to_gl_texture(&board_tex, data_path("../scenes/board.png"));
  if(ret) {
  	printf("Cannot load texture, error code %d.\n", ret);
    abort();
  }
	ret = png_to_gl_texture(&lens_view_tex, data_path("../scenes/lens_view.png"));
  if(ret) {
  	printf("Cannot load texture, error code %d.\n", ret);
    abort();
  }

	schedule_notification(data_path("../text/intro.txt"), 1.5);

	// sampler
	GLuint sampler{0};
  glGenSamplers(1, &sampler);
  glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindSampler(0, sampler);
}

PlayMode::~PlayMode() {
	//FT_Done_Face(ft_face);
	//FT_Done_FreeType(ft_library);
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size_) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_c) {
			switch_camera.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_c) {
			switch_camera.downs += 1;
			switch_camera.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs += 1;
			down.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if(!notification_active) {
			int x, y;
			SDL_GetMouseState(&x, &y);
			for (auto &button: buttons) {
				button.interact(x, y, window_size);
			}
			return true;
		} else {
			return false;
		}
	} 

	return false;
}

void PlayMode::update(float elapsed) {
	total_elapsed += elapsed;

	if(notif_pq.size() && !notification_active) {
		auto top = notif_pq.top();
		float t = -top.first;
		std::string filename = top.second;
		if(t <= total_elapsed) {
			notif_pq.pop();
			display_notification(filename);
		}
	}

	if(!notification_active) {

		// update food visuals
	{
		size_t n_strawberries = int((totalFood + 199.f) / 200.f);
		while(strawberry_transforms.size() > n_strawberries) {
			strawberry_transforms.pop_front();
		}
		while(strawberry_transforms.size() < n_strawberries) {
			strawberry_transforms.push_back(spawn_strawberry());
		}
		auto it = scene.drawables.begin();
		while(it != scene.drawables.end()) {
			Scene::Drawable dr = *it;
			if ( it->transform->name == "Strawberry" && std::find(strawberry_transforms.begin(), strawberry_transforms.end(), it->transform) == strawberry_transforms.end() ) {
				it = scene.drawables.erase(it);
			} else {
				it++;
			}
		}
	}

	// update crickets age and state
	{
		for(Cricket &cricket: Crickets) {
			if (cricket.is_dead()) {
				continue;
			}
			cricket.age += elapsed;
			if(cricket.is_dead()) {
				kill_cricket(cricket);
			} else if (cricket.is_mature()) {
				if(!first_time_matured) {
					first_time_matured = true;
					schedule_notification(data_path("../text/first_time_matured.txt"), 1);
				}
				mature_cricket(cricket);
			} else if (cricket.is_baby()) {
				if(!first_time_babies) {
					first_time_babies = true;
					schedule_notification(data_path("../text/first_time_babies.txt"), 1.000f);
					schedule_notification(data_path("../text/first_time_babies2.txt"), 1.001f);
					schedule_notification(data_path("../text/first_time_babies3.txt"), 1.002f);
				}
				hatch_cricket(cricket);
			}
		}
	}

	// Move some crickets randomly
	{
		const float JumpDistance = 0.1f;
		for(Cricket &cricket: Crickets) {
			if(cricket.is_dead() || cricket.is_egg()) {
				continue;
			}
			if(std::rand() % 40 == 0) {

				glm::quat quat = glm::angleAxis(glm::radians(get_rng_range(-20.f,20.f)), glm::vec3(0.0,0.0,1.0));
				cricket.transform->rotation = glm::normalize(cricket.transform->rotation * quat);

				Scene::Transform *closest_strawberry = nullptr;
				for(Scene::Transform *transform: strawberry_transforms) {
					if(closest_strawberry == nullptr || 
						glm::length(cricket.transform->position - transform->position) < glm::length(cricket.transform->position - closest_strawberry->position)) {
						closest_strawberry = transform;
					}
				}
				if(closest_strawberry != nullptr && glm::length(cricket.transform->position - closest_strawberry->position) < 0.4f) {
					continue;
				}

				glm::vec3 dir = cricket.transform->rotation * glm::vec3(0.f, 1.f, 0.f) ;
				dir = JumpDistance * glm::normalize(dir);
				cricket.transform->position += dir;

				glm::vec3 &pos = cricket.transform->position;
				const float eps = 0.2f;

				if (!(bedding_min.x + eps < pos.x && pos.x < bedding_max.x - eps && bedding_min.y + eps < pos.y && pos.y < bedding_max.y - eps)) {
					glm::quat turn_around = glm::angleAxis(glm::radians(180.f), glm::vec3(0.0,0.0,1.0));
					cricket.transform->rotation = glm::normalize(cricket.transform->rotation * turn_around);

					pos.x = glm::clamp(pos.x, bedding_min.x + eps, bedding_max.x - eps);
					pos.y = glm::clamp(pos.y, bedding_min.y + eps, bedding_max.y - eps);
				}
			}
		}
	}

	//update counts
	{
		numBabyCrickets = 0;
		numMatureCrickets = 0;
		numDeadCrickets = 0;
		numEggs = 0;
		for(Cricket &cricket: Crickets) {
			if (cricket.is_mature()){
				numMatureCrickets ++;
			}
			if (cricket.is_dead()) {
				numDeadCrickets++;
			}
			if(cricket.is_baby()) {
				numBabyCrickets++;
			}
			if (cricket.is_egg()) {
				numEggs++;
			}
		}
		assert(numBabyCrickets + numMatureCrickets + numDeadCrickets + numEggs == Crickets.size());
	}

	//update disease rate if we have too many dead crickets in the environment
	if ((uint64_t)(total_elapsed)%2 == 0)
	{
		auto is_sick = [=, *this](){
			return size_t(rand() % 2000 + 1) < 8*numDeadCrickets/Crickets.size();
		};

		for( Cricket &cricket: Crickets) {
			if(cricket.is_dead() || cricket.is_egg()) {
				continue;
			}
			cricket.is_healthy = !is_sick();
			if(cricket.is_dead()) {
				if(!first_time_sick) {
					first_time_sick = true;
					schedule_notification(data_path("../text/first_time_sick.txt"), 1.5f);
					schedule_notification(data_path("../text/first_time_sick2.txt"), 1.5001f);
				}
				kill_cricket(cricket);
			}
		}
	}
	// Set sounds
	{
		// Stop / start chirping sounds if no crickets
		if (numBabyCrickets > 0 || numMatureCrickets > 0) {
			if (chirping_loop == NULL) {
				chirping_loop = Sound::loop(*chirping_sample, 1.0f, 0.0f);
			}
		} else {
			if (chirping_loop != NULL) {
				chirping_loop->stop();
				chirping_loop = NULL;
			}
		}
	}

	//update food and starvation
	if ((uint64_t)(total_elapsed)%2 == 0)
	{
		// Todo: do adults eat more than babies ?
		// Todo: do not eat at every frame
		
		if (totalFood == 0.f){
			auto is_starving = [](){
				return rand() % 1000 + 1 < 6;
			};
			for( Cricket &cricket: Crickets) {
				if(cricket.is_dead() || cricket.is_egg()) {
					continue;
				}
				cricket.is_healthy = !is_starving();
				if(cricket.is_dead()) {
					if(!first_time_starved) {
						first_time_starved = true;
						schedule_notification(data_path("../text/first_time_starved.txt"), 0);
					}
					kill_cricket(cricket);
				}
			}
		}
		totalFood = std::max(0.f, totalFood - (numBabyCrickets + numMatureCrickets) * cricketEatingRate);
	}

	// switch camera
	if(switch_camera.downs) {
		alt_view = !alt_view;
		if(!first_time_alt_view) {
			first_time_alt_view = true;
			schedule_notification(data_path("../text/first_time_alt_view.txt"), 0);
		}
		camera_body_transform->scale = glm::vec3(alt_view ? 0.f : 1.f);
	}

	//move camera:
	if(!alt_view){

		//combine inputs into a move:
		float motion = 0.f;

		if (down.pressed && !up.pressed) motion = 0.3f;
		if (!down.pressed && up.pressed) motion = -0.3f;

		glm::vec3 dir = main_camera->transform->rotation * glm::vec3(0.f, 0.f, 1.f) ;
		dir = motion * glm::normalize(dir);
		main_camera->transform->position += dir;

		main_camera->transform->position.x = glm::clamp(main_camera->transform->position.x, -10.f, 10.f);
		main_camera->transform->position.y = glm::clamp(main_camera->transform->position.y, -10.f, 10.f);

		motion = 0.f;
		if (left.pressed && !right.pressed)motion = 0.03f;
		else if (!left.pressed && right.pressed)motion = -0.03f;

		main_camera->transform->rotation = glm::normalize(
			main_camera->transform->rotation
				* glm::angleAxis(motion * main_camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
		);
	} else {
		float motion = 0.f;

		if (down.pressed && !up.pressed) motion = 0.01f;
		if (!down.pressed && up.pressed) motion = -0.01f;

		alt_camera->fovy += motion;
		alt_camera->fovy = std::clamp(alt_camera->fovy, 0.01f, 1.6f);
		// Todo: move left and right
	}

	{ //update listener to camera position:
		glm::mat4x3 frame = main_camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		glm::vec3 frame_at = frame[3];
		Sound::listener.set_position_right(frame_at, frame_right, 1.0f / 60.0f);
	}

	
	} else {

		if(space.downs && letter_counter < total_letters) {
			letter_counter = total_letters;
		} else if(space.downs) {
			hide_current_notification();
		} else {
			current_elapsed += elapsed;
			if(current_elapsed >= max_elapsed) {
				letter_counter = std::min(letter_counter + 1, total_letters);
				current_elapsed = 0.0;
			}
		}
	}

	if(numDeadCrickets == Crickets.size() && totalMoney < eggPrice && !gameOver) {
		gameOver = true;
		schedule_notification(data_path("../text/game_over.txt"), 1);
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	space.downs = 0;
	switch_camera.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	main_camera->aspect = float(drawable_size.x) / float(drawable_size.y);
	alt_camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	if(alt_view) {
		scene.draw(*alt_camera);
	} else {
		scene.draw(*main_camera);
	}

	// Draw stats
	{
		draw_stats(drawable_size, -1.f, -0.7f);
	}

	{ //use DrawLines to overlay some text:

		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		auto popup = popups.begin();
		while(popup != popups.end()) {
			auto current_time = std::chrono::high_resolution_clock::now();
			float time_elapsed = std::chrono::duration< float >(current_time - popup->start_time).count();
			if (time_elapsed < popup->duration) {
				popup->draw(lines);
				popup++;
			} else {
				popup = popups.erase(popup);
			}
		}

		for (auto &button : buttons) {
			button.draw(drawable_size);
		}
	}

	// draw_textured_quad(&button_unclicked_tex, -0.9f, 0.5f, drawable_size);
	// draw_textured_quad(&strawberry_tex, -0.9f, 0.5f, drawable_size);
	// draw_textured_quad(&button_clicked_tex, -0.9f + 0.2f, 0.5f, drawable_size);
	// draw_textured_quad(&strawberry_tex, -0.9f + 0.2f, 0.5f, drawable_size);
	// draw_textured_quad(&button_unclicked_tex, -0.9f, 0.1f, drawable_size);
	// draw_textured_quad(&egg_tex, -0.9f, 0.1f, drawable_size);
	// draw_textured_quad(&button_unclicked_tex, -0.9f, -0.3f, drawable_size);
	// draw_textured_quad(&dollars_tex, -0.9f, -0.3f, drawable_size);

	if (notification_active) {
		draw_filled_rect(glm::vec2(-1.f,-1.f), glm::vec2(1.f, 1.f), glm::vec4(0.f, 0.f, 0.f, 0.4f));
		draw_textured_quad(&board_tex, -0.6f, -0.6f, drawable_size);
		draw_text_lines(drawable_size, -0.78f, 0.36f);
		GL_ERRORS();
		glDisable(GL_DEPTH_TEST);
		GL_ERRORS();

	} else if (alt_view) {
		draw_textured_quad(&lens_view_tex, -0.45f, -0.7f, drawable_size);
		GL_ERRORS();
	}
}

void PlayMode::display_notification(std::string filename) {
	std:: string text = load_text_from_file(filename);

	notification_active = true;
	notification_text = std::vector<std::string>();

	size_t start=0, len=0;
	total_letters = 0;
	for(char c:text) {
		if(c=='\n') {
			notification_text.push_back(text.substr(start,len));
			start += len + 1;
			total_letters += len;
			len = 0;
			continue;
		}
		len++;
		if((len >= 44 && c == ' ') || c=='\n') {
			notification_text.push_back(text.substr(start,len));
			start += len;
			total_letters += len;
			len = 0;
		}
	}
	if(start < text.size()) {
		notification_text.push_back(text.substr(start));
		total_letters += text.size() - start;
	}
}

void PlayMode::schedule_notification(std::string filename, float in_time) {
	notif_pq.push(make_pair(-total_elapsed - in_time, filename));
}

void PlayMode::hide_current_notification() {
	notification_active = false;
	notification_text = std::vector<std::string>();
	letter_counter = 0;
	current_elapsed = 0;
}

void PlayMode::draw_filled_rect(glm::vec2 lower_left, glm::vec2 upper_right, glm::vec4 color) {

	glUseProgram(rect_program);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_ERRORS();

	GLfloat fill_color[] = {color[0], color[1], color[2], color[3]};
	GLuint fill_color_loc = glGetUniformLocation(rect_program, "fillColor");
	GL_ERRORS();
	glUniform4fv(fill_color_loc, 1, fill_color);
	GL_ERRORS();

	GLuint vbo = 0, vao = 0;
	glGenBuffers(1, &vbo);
	GL_ERRORS();
	glGenVertexArrays(1, &vao);
	GL_ERRORS();

	GLfloat rect_data[] =
	{
		lower_left.x, lower_left.y,
		upper_right.x, lower_left.y,
		upper_right.x, upper_right.y,
		lower_left.x, upper_right.y
	};

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	GL_ERRORS();
	glBufferData(GL_ARRAY_BUFFER, sizeof(rect_data), rect_data, GL_STATIC_DRAW);
	GL_ERRORS();

	glBindVertexArray(vao);
	GL_ERRORS();
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	GL_ERRORS();
	glEnableVertexAttribArray(0);
	GL_ERRORS();

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glUseProgram(0);
	glDisable(GL_BLEND);

	GL_ERRORS();
}

void PlayMode::invoke_callback(Button_UI::call_back callback) {
	bool clickSuccess = true;
	switch(callback) {
		case Button_UI::BUY_FOOD:
			clickSuccess = buy_food();
			break;
		case Button_UI::BUY_EGG:
			clickSuccess = buy_eggs();
			break;
		case Button_UI::SELL_MATURE:	
			clickSuccess = sell_mature();
			if (clickSuccess)
				Sound::play(*cash_sample, 1.0f, 0.0f);
			break;
		default:
			throw std::runtime_error("unrecognized callback\n");
	}
	if (clickSuccess)
		Sound::play(*click_sample, 25.0f, 0.0f);
	else
		Sound::play(*click_error_sample, 1.0f, 0.0f);
}


template<typename T>
bool buy(T quantity, float price, T &total_quantity, float &total_money) {
	if (total_money >= price) {
		total_money -= price;
		total_quantity += quantity;
		return true;
	}
	return false;
}

bool PlayMode::buy_food() {
	std::cout << "buy_food" << std::endl;
	if(!first_time_food) {
		first_time_food = true;
		schedule_notification(data_path("../text/first_time_food.txt"), 1.5);
	}
	const size_t unitFood = 10;
	const float unitPrice = foodPrice;

	return buy<float>(unitFood, unitPrice, totalFood, totalMoney);
}

bool PlayMode::buy_eggs() {
	std::cout << "buy_eggs" << std::endl;
	if(!first_time_eggs) {
		first_time_eggs = true;
		schedule_notification(data_path("../text/first_time_eggs.txt"), 1.5);
	}
	const size_t unitEggs = 19;
	const float unitPrice = eggPrice;
	bool success = false;

	if (totalMoney >= unitPrice) {
		for (size_t i = 0; i < unitEggs; i++)
			spawn_cricket();
		totalMoney -= unitPrice;
		success = true;
	}

	return success;
}

void PlayMode::mature_cricket(Cricket &cricket) {
	Mesh const &mesh = bzz_meshes->lookup("AdultCricket");
	for(Scene::Drawable &drawable: scene.drawables) {
		if(drawable.transform->name == "Cricket_" + std::to_string(cricket.cricketID)) {
			drawable.pipeline.type = mesh.type;
			drawable.pipeline.start = mesh.start;
			drawable.pipeline.count = mesh.count;
			break;
		}
	}
}

void PlayMode::hatch_cricket(Cricket &cricket) {
	cricket.transform->scale = glm::vec3(1.f);
}

bool PlayMode::sell_mature() {
	std::cout << "sell_mature" << std::endl;
	const float price = 50;

	std::unordered_set<std::string> mature_cricket_names;
	std::vector<Cricket> mature_crickets;
	std::vector<Cricket> non_mature_crickets;
	bool success = false;

	for(Cricket &cricket: Crickets) {
		if(cricket.is_mature()) {
			mature_crickets.push_back(cricket);
			mature_cricket_names.insert("Cricket_" + std::to_string(cricket.cricketID));
		}else{
			non_mature_crickets.push_back(cricket);
		}
	}

	assert(mature_crickets.size() == numMatureCrickets);

	if (numMatureCrickets > 0) {
		success = true;
	}
	
	float profit = numMatureCrickets * price;
	totalMoney += profit;

	if(totalMoney >= 950 && !first_time_950_dollars) {
		first_time_950_dollars = true;
		schedule_notification(data_path("../text/first_time_950_dollars.txt"), 1.5);
	}	

	numMatureCrickets = 0;
	Crickets = std::move(non_mature_crickets);

	auto it = scene.drawables.begin();
	while(it != scene.drawables.end()) {
		Scene::Drawable dr = *it;
		if ( mature_cricket_names.count(dr.transform->name) ) {
			it = scene.drawables.erase(it);
		} else {
			it++;
		}
	}

	if (success) {
		// glm::u8vec4(0x00, 0x00, 0x00, 0x00)
		
		popups.emplace_back(glm::vec2(235,345), "+$" + std::to_string(int(profit)), glm::u8vec4(0x38, 0x66, 0x41, 0x00), 0.75f);
	}

	return success;
}

bool PlayMode::remove_dead_crickets() {
	std::cout << "removing dead" << std::endl;

	std::unordered_set<std::string> dead_cricket_names;
	std::vector<Cricket> dead_crickets;
	std::vector<Cricket> live_crickets;
	bool success = false;

	for(Cricket &cricket: Crickets) {
		if(cricket.is_dead()) {
			dead_crickets.push_back(cricket);
			dead_cricket_names.insert("Cricket_" + std::to_string(cricket.cricketID));
		}else{
			live_crickets.push_back(cricket);
		}
	}

	assert(dead_crickets.size() == numDeadCrickets);

	if (numDeadCrickets > 0) {
		success = true;
	}
	

	numDeadCrickets = 0;
	Crickets = std::move(live_crickets);

	auto it = scene.drawables.begin();
	while(it != scene.drawables.end()) {
		Scene::Drawable dr = *it;
		if ( dead_cricket_names.count(dr.transform->name) ) {
			it = scene.drawables.erase(it);
		} else {
			it++;
		}
	}

	return success;
}



void Popup_UI::draw(DrawLines &lines) {
	// hard coded: should change in the future
	const uint16_t width = 1280;
	const uint16_t height = 720;
	const float aspect = (float)width / (float)height;

	const float H = 0.09f;

	float screen_x = anchor.x / width * 2 - 1;
	screen_x *= aspect;
	float screen_y = (height - anchor.y) / height * 2 - 1;
	lines.draw_text(text,
	glm::vec3(screen_x, screen_y, 0.0f),
	glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
	color);

	anchor.x += 0.5;
}

PlayMode::Button_UI::Button_UI(glm::vec2 _anchor, std::string icon_png, call_back _trigger_event)	:
    anchor(_anchor), trigger_event(_trigger_event) {
	
	int ret;
	ret = png_to_gl_texture(&clickedTex, data_path("../scenes/button_clicked.png"));
	assert(ret == 0);
	ret = png_to_gl_texture(&unclickedTex, data_path("../scenes/button_unclicked.png"));
	assert(ret == 0);
	ret = png_to_gl_texture(&icon, data_path(icon_png));
	assert(ret == 0);

	active = false;
	clicked = false;

	GL_ERRORS();
}

void PlayMode::Button_UI::draw(glm::uvec2 const &drawable_size) {
	auto cur_time = std::chrono::high_resolution_clock::now();
	if (clicked && cur_time > reset_time) {
		clicked = false;
	}

	if (clicked) draw_textured_quad(&unclickedTex, anchor.x, anchor.y, drawable_size);
	else draw_textured_quad(&clickedTex, anchor.x, anchor.y, drawable_size);

	draw_textured_quad(&icon, anchor.x, anchor.y, drawable_size);

	GL_ERRORS();
}

void PlayMode::Button_UI::interact(int mouse_x, int mouse_y, glm::vec2 drawable_size) {
	int x0 = (int) (anchor.x * drawable_size.x);
	int y0 = (int) (anchor.x * drawable_size.x);

	int x1 = x0 + clickedTex.w;
	int y1 = y0 + clickedTex.h;

	if (mouse_x >= x0 && mouse_x <= x1 &&
		mouse_y >= y0 && mouse_y <= y1) {
		
		clicked = true;
		reset_time = std::chrono::high_resolution_clock::now() + std::chrono::microseconds(500);
	}
}

std::string PlayMode::load_text_from_file(std::string filename) {
	std::string text = "";
  std::ifstream input(filename);
  for (std::string line; getline(input, line);) {
		text += line + '\n';
  }
	return text;
}

void PlayMode::draw_stats(glm::uvec2 const &drawable_size, float x, float y) {

	GLuint vbo{0}, vao{0}, texture{0};

	GL_ERRORS();
	glUseProgram(text_program);
	GL_ERRORS();

	GLuint texUniform = glGetUniformLocation(text_program, "tex");
	glUniform1i(texUniform, 0);

	glGenBuffers(1, &vbo);
  glGenVertexArrays(1, &vao);
  glGenTextures(1, &texture);

	// Set some GL state
  glEnable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0, 0, 0, 1);
	GL_ERRORS();

	GLuint fill_color_loc = glGetUniformLocation(text_program, "fillColor");
	GL_ERRORS();
	const GLfloat green[4] = {0.f, 1.f, 0.f, 1.0};
	glUniform4fv(fill_color_loc, 1, green);
	GL_ERRORS();

	// Bind stuff
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
	GL_ERRORS();

	std:: string sep = "      ";
	std::string s = "Food: " + std::to_string((int)totalFood) + sep + "Money: " + std::to_string((int)totalMoney) + sep + "Eggs:" + std::to_string(numEggs) + sep + "Nymphs: " + std::to_string(numBabyCrickets) + sep + "Adults: " + std::to_string(numMatureCrickets);
	if(numDeadCrickets > 0) {
		s += sep + "Dead: " + std::to_string(numDeadCrickets);
	}
	draw_text_line(s,drawable_size,x,y);

	glDisable(GL_BLEND);
	glUseProgram(0);
}

void PlayMode::draw_text_lines(glm::uvec2 const &drawable_size, float x, float y) {
	GLuint vbo{0}, vao{0}, texture{0};

	GL_ERRORS();
	glUseProgram(text_program);
	GL_ERRORS();

	GLuint texUniform = glGetUniformLocation(text_program, "tex");
	glUniform1i(texUniform, 0);

	glGenBuffers(1, &vbo);
  glGenVertexArrays(1, &vao);
  glGenTextures(1, &texture);

	// Set some GL state
  glEnable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0, 0, 0, 1);
	GL_ERRORS();

	GLuint fill_color_loc = glGetUniformLocation(text_program, "fillColor");
	GL_ERRORS();
	const GLfloat grey[4] = {0.2f, 0.2f, 0.2f, 1.0};
	glUniform4fv(fill_color_loc, 1, grey);
	GL_ERRORS();

	// Bind stuff
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
	GL_ERRORS();

	size_t counter = 0;
	for( size_t i=0;i<notification_text.size();i++) {
		std::string line = notification_text[i];
		if (counter + line.size() <= letter_counter) {
			counter += line.size();
			draw_text_line(line,drawable_size,x,y);
		} else if (counter < letter_counter) {
			draw_text_line(line.substr(0,letter_counter - counter),drawable_size,x,y);
			counter = letter_counter;
		}
		
		y -= ft_face->size->metrics.height / (64.f * (float)drawable_size.y);
	}

	glDisable(GL_BLEND);
	glUseProgram(0);
}

// https://gedge.ca/blog/2013-12-08-opengl-text-rendering-with-freetype
// https://www.freetype.org/freetype2/docs/tutorial/step1.html
// https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
void PlayMode::draw_text_line(std::string s, glm::uvec2 const &drawable_size, float current_x, float current_y) {

  const char *text = s.c_str();

	/* Create hb-buffer and populate. */
  hb_buffer_t *hb_buffer = hb_buffer_create ();
  hb_buffer_add_utf8 (hb_buffer, text, -1, 0, -1);
  hb_buffer_guess_segment_properties (hb_buffer);

  /* Shape it! */
  hb_shape (hb_font, hb_buffer, NULL, 0);

  /* Get glyph information and positions out of the buffer. */
  unsigned int len = hb_buffer_get_length (hb_buffer);
  hb_glyph_info_t *info = hb_buffer_get_glyph_infos (hb_buffer, NULL);
  hb_glyph_position_t *pos = hb_buffer_get_glyph_positions (hb_buffer, NULL);

  /* And converted to absolute positions. */
  {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

    for (unsigned int i = 0; i < len; i++)
    {
      hb_codepoint_t glyph_index   = info[i].codepoint;

			/* load glyph image into the slot (erase previous one) */
  		FT_Error error = FT_Load_Glyph( ft_face, glyph_index, FT_LOAD_DEFAULT );
  		if ( error )
    		continue;  /* ignore errors */

			/* convert to an anti-aliased bitmap */
  		error = FT_Render_Glyph( ft_face->glyph, FT_RENDER_MODE_NORMAL );
  		if ( error )
    		continue;


      float x_position = current_x + (pos[i].x_offset + 400 * 64.f) / (64.f * drawable_size.x);
      float y_position = current_y + ((pos[i].y_offset / 64.f) - ((ft_face->bbox.yMax - ft_face->bbox.yMin) - ft_face->glyph->metrics.horiBearingY) / 64.f) / (drawable_size.y);

			FT_Bitmap *bm = &ft_face->glyph->bitmap;
			// FT_PIXEL_MODE_GRAY
			// each pixel stored in 8 bits

    	glTexImage2D(
      	GL_TEXTURE_2D,
        0,
        GL_R8,
        ft_face->glyph->bitmap.width,
        ft_face->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        ft_face->glyph->bitmap.buffer
    	);

      const float w = bm->width / (float)drawable_size.x;
      const float h = bm->rows/ (float)drawable_size.y;

      struct {
      	float x, y, s, t;
      } data[6] = {
      	{x_position    , y_position    , 0, 0},
        {x_position    , y_position - h, 0, 1},
        {x_position + w, y_position    , 1, 0},
        {x_position + w, y_position    , 1, 0},
        {x_position    , y_position - h, 0, 1},
        {x_position + w, y_position - h, 1, 1}
      };

      glBufferData(GL_ARRAY_BUFFER, 24*sizeof(float), data, GL_DYNAMIC_DRAW);
      glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
      glDrawArrays(GL_TRIANGLES, 0, 6);

      current_x += (pos[i].x_advance) / (64.f * drawable_size.x);
      current_y += pos[i].y_advance / (64.f * drawable_size.y);

			assert(current_x <= 1.0);
			assert(current_y <= 1.0);
    }
  }

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	GL_ERRORS();

}

// https://dcemulation.org/index.php?title=Loading_PNG_images_as_OpenGL_textures
static int png_to_gl_texture(PlayMode::texture * tex, std::string filename) {

	#define CLEANUP(x) { ret = (x); goto cleanup; }

	int ret = 0;
	FILE * file = 0;
	uint8_t * data = 0;
	png_structp parser = 0;
	png_infop info = 0;
	png_bytep * row_pointers = 0;
	int rowbytes;
	GLenum texture_format;

	png_uint_32 w, h;
	int bit_depth;
	int color_type;


	if(!tex || !filename.c_str()) {
		CLEANUP(1);
	}

	file = fopen(filename.c_str(), "rb");
	if(!file) {
		CLEANUP(2);
	}
	// printf("sometjign happened\n");
	// file = fopen( filename.c_str(), "rb");
	// if (file != NULL) {
	// 	printf("null file pointer\n");
	// 	CLEANUP(2);
	// }


	parser = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if(!parser) {
		CLEANUP(3);
	}

	info = png_create_info_struct(parser);
	if(!info) {
		CLEANUP(4);
	}

	if(setjmp(png_jmpbuf(parser))) {
		CLEANUP(5);
	}

	png_init_io(parser, file);
	png_read_info(parser, info);
	png_get_IHDR(parser, info, &w, &h, &bit_depth, &color_type, 0, 0, 0);

	if((w & (w-1)) || (h & (h-1)) || w < 8 || h < 8) {
		CLEANUP(6);
	}

	if(png_get_valid(parser, info, PNG_INFO_tRNS) || (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) || color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_expand(parser);
	}
	if(bit_depth == 16) {
		png_set_strip_16(parser);
	}
	if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(parser);
	}
	png_read_update_info(parser, info);

	rowbytes = (int) png_get_rowbytes(parser, info);
	rowbytes += 3 - ((rowbytes-1) % 4); // align to 4 bytes

	data = (uint8_t*)malloc(rowbytes * h * sizeof(png_byte) + 15);
	if(!data) {
		CLEANUP(7);
	}

	row_pointers = (png_bytep*)malloc(h * sizeof(png_bytep));
	if(!row_pointers) {
		CLEANUP(8);
  }

	// set the individual row_pointers to point at the correct offsets of data
	for(png_uint_32 i = 0; i < h; ++i) {
		row_pointers[h - 1 - i] = data + i * rowbytes;
	}

	png_read_image(parser, row_pointers);

	// Generate the OpenGL texture object
	GLuint texture_id;
	glGenTextures(1, &texture_id);
	GL_ERRORS();
	glBindTexture(GL_TEXTURE_2D, texture_id);
	GL_ERRORS();
	texture_format = (color_type & PNG_COLOR_MASK_ALPHA) ? GL_RGBA : GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, texture_format, w, h, 0, texture_format, GL_UNSIGNED_BYTE, data);
	GL_ERRORS();

	tex->id = texture_id;
	tex->w = (uint16_t) w;
	tex->h = (uint16_t) h;
	tex->format = texture_format;
	tex->min_filter = tex->mag_filter = GL_NEAREST;


cleanup:
	if(parser) {
		png_destroy_read_struct(&parser, info ? &info : 0, 0);
	}

	if(row_pointers) {
		free(row_pointers);
	}

	if(ret && data) {
		free(data);
	}

	if(file) {
		fclose(file);
	}

	return ret;
}

static void draw_textured_quad(PlayMode::texture * tex, float x0, float y0, glm::uvec2 const &drawable_size) {
	GLuint vbo{0}, vao{0};

	glUseProgram(png_program);
	glGenBuffers(1, &vbo);
  glGenVertexArrays(1, &vao);

	// Bind stuff
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex->id);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float x1 = x0 + (float)tex->w / drawable_size.x;
	float y1 = y0 + (float)tex->h / drawable_size.y;

	float const vertex_data[] = {
		/* 2D Coordinate, texture coordinate */
		x0, y1,  0.f, 1.f,
		x0, y0,  0.f, 0.f,
		x1, y1,  1.f, 1.f,
		x1, y0,  1.f, 0.f,
	};

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex->min_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex->mag_filter);

	glBufferData(GL_ARRAY_BUFFER, 16*sizeof(float), vertex_data, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	glUseProgram(0);
	glDisable(GL_BLEND);
}
