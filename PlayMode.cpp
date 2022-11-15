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

	const float eps = 0.2f;
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
	// TODO remove!!
	// cricket.age = cricket.matureAge;
	Crickets.push_back(cricket);

	transform->position = glm::vec3(get_rng_range(bedding_min.x,bedding_max.x), get_rng_range(bedding_min.y,bedding_max.y), baby_cricket_transform->position.z);
	transform->rotation = glm::angleAxis(glm::radians(get_rng_range(0.f,360.f)), glm::vec3(0.0,0.0,1.0));
	transform->scale = glm::vec3(1.f);
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


PlayMode::PlayMode() : scene(*bzz_scene), game_UI(this) {

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
	}

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	// Loop chirping sound and background music
	// chirping_loop = Sound::loop(*chirping_sample, 1.0f, 0.0f);
	// chirping_loop.
	Sound::loop(*background_sample, 1.0f, 0.0f);

	Mesh const &mesh = bzz_meshes->lookup("Bedding");
	glm::mat4x3 to_world = bedding_transform->make_local_to_world();
	bedding_min = to_world * glm::vec4(mesh.min, 1.f);
	bedding_max = to_world * glm::vec4(mesh.max, 1.f);

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
	GL_ERRORS();

  fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &FRAGMENT_SHADER, 0);
  glCompileShader(fs);
	GL_ERRORS();

  rect_program= glCreateProgram();
  glAttachShader(rect_program, vs);
  glAttachShader(rect_program, fs);
  glLinkProgram(rect_program);
	GL_ERRORS();

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
				"const vec4 color = vec4(0.388, 0.765, 0.196, 1);\n"
        "void main(void) {\n"
        "    fragColor = vec4(1, 1, 1, texture(tex, texCoords).r) * color;\n"
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
	#define FONT_SIZE 32

	// https://www.1001fonts.com/risque-font.html
	std::string fontfile = data_path("../scenes/Risque-Regular.ttf");

	/* Initialize FreeType and create FreeType font face. */
  FT_Error ft_error;

  if ((ft_error = FT_Init_FreeType (&ft_library)))
    abort();
  if ((ft_error = FT_New_Face (ft_library, fontfile.c_str(), 0, &ft_face)))
    abort();
  if ((ft_error = FT_Set_Char_Size (ft_face, FONT_SIZE*64, FONT_SIZE*64, 0, 0)))
    abort();

  /* Create hb-ft font. */
  hb_font = hb_ft_font_create (ft_face, NULL);

	//show_notification(data_path("../scenes/intro"));

}

PlayMode::~PlayMode() {
	FT_Done_Face(ft_face);
	FT_Done_FreeType(ft_library);
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

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
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if(notification_active) {
			hide_notification();
		} else {
			int x, y;
			SDL_GetMouseState(&x, &y);
			std::cout << x << ", " << y << std::endl;
			game_UI.update(x, y, evt.type == SDL_MOUSEBUTTONDOWN);
		}
		return true;
	} 

	return false;
}

void PlayMode::update(float elapsed) {
	total_elapsed += elapsed;

	if(!notification_active) {

		printf("a\n");
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
				mature_cricket(cricket);
			}
		}
	}
	printf("b5\n");

	// Move some crickets randomly
	{
		const float JumpDistance = 0.1f;
		for(Cricket &cricket: Crickets) {
			if(cricket.is_dead()) {
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
		}
		assert(numBabyCrickets + numMatureCrickets + numDeadCrickets == Crickets.size());
	}
	printf("b4\n");

	//update disease rate if we have too many dead crickets in the environment
	if ((uint64_t)(total_elapsed)%2 == 0)
	{
		printf("inside if\n");
		auto is_sick = [=, *this](){
			return uint(rand() % 2000 + 1) < 8*numDeadCrickets/Crickets.size();
		};
		printf("inside if 2\n");
		printf("size = %zu\n", Crickets.size());

		for( Cricket &cricket: Crickets) {
			printf("inside loop\n");
			if(cricket.is_dead()) {
				continue;
			}
			cricket.is_healthy = !is_sick();
			if(cricket.is_dead()) {
				kill_cricket(cricket);
			}
		}
		printf("after loop\n");
	}
	printf("b3\n");
	printf("b3\n");
	printf("b3\n");
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
	printf("b2\n");

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
				if(cricket.is_dead()) {
					continue;
				}
				cricket.is_healthy = !is_starving();
				if(cricket.is_dead()) {
					kill_cricket(cricket);
				}
			}
		}
		totalFood = std::max(0.f, totalFood - (numBabyCrickets + numMatureCrickets) * cricketEatingRate);
	}
	printf("b1\n");

	//move camera:
	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 10.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		camera->transform->position.x += move.x;
		camera->transform->position.y += move.y;
	}

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		glm::vec3 frame_at = frame[3];
		Sound::listener.set_position_right(frame_at, frame_right, 1.0f / 60.0f);
	}

	
	}

	if ((totalFood == 0 && totalMoney < foodPrice) ||  (totalMoney < eggPrice && numMatureCrickets == 0)){
		gameOver = true;
		//show_notification("Game Over");

		
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

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

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		game_UI.draw();
		if (totalFood <= 0){
			lines.draw_text("Food: " + std::to_string((int)totalFood),
				glm::vec3(aspect - 0.8f, 1.0f - 0.30f, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xEE, 0x22, 0x22, 0x00));
			lines.draw_text("Crickets are starving.",
				glm::vec3(aspect - 0.8f, 1.0f - 0.35f, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H/2, 0.0f),
				glm::u8vec4(0xff, 0x11, 0x11, 0x00));
		}else
		{
			lines.draw_text("Food: " + std::to_string((int)totalFood),
				glm::vec3(aspect - 0.8f, 1.0f - 0.30f, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		}
		lines.draw_text("Money: " + std::to_string((int)totalMoney),
			glm::vec3(aspect - 0.8f, 1.0f - 0.45f, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));

		lines.draw_text("Baby Crickets: " + std::to_string(numBabyCrickets),
			glm::vec3(aspect - 0.8f, 1.0f - 0.60f, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));

		lines.draw_text("Mature Crickets: " + std::to_string(numMatureCrickets),
			glm::vec3(aspect - 0.8f, 1.0f - 0.75f, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		lines.draw_text("WASD moves the camera",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("WASD moves the camera",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		if (numDeadCrickets > 0){
			lines.draw_text("Dead crickets are spreading disease.",
			glm::vec3(aspect -2.f, -1.0 + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0xff));
		}

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

		
	}

	if (notification_active) {
		draw_filled_rect(glm::vec2(-1.f,-1.f), glm::vec2(1.f, 1.f), glm::vec4(0.f, 0.f, 0.f, 0.4f));
		draw_filled_rect(glm::vec2(-0.5f,-0.5f), glm::vec2(0.5f, 0.5f), glm::vec4(45.f / 256.f, 32.f / 256.f, 107.f / 256.f, 1.f));
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		const float eps1 = 0.03;
		const float eps2 = 0.01;
		const float eps3 = 0.02;
		draw_filled_rect(glm::vec2(-0.5f + eps1 / aspect,-0.5f + eps1), glm::vec2(0.5f - eps1 / aspect, 0.5f - eps1), glm::vec4(138.f / 256.f, 119.f / 256.f, 67.f / 256.f, 1.f));
		GL_ERRORS();
		draw_filled_rect(glm::vec2(-0.5f +  (eps1 + eps2) / aspect,-0.5f + eps1 + eps3), glm::vec2(0.5f - (eps1 + eps3)  / aspect, 0.5f - (eps1 + eps2)), glm::vec4(227.f / 256.f, 213.f / 256.f, 184.f / 256.f, 1.f));
		GL_ERRORS();
		draw_text_lines(drawable_size,-0.8,0.8);
		GL_ERRORS();
		glDisable(GL_DEPTH_TEST);
		GL_ERRORS();

	}
}

void PlayMode::show_notification(std::string file_name) {

	std:: string text = load_text_from_file(file_name);

	notification_active = true;
	notification_text = std::vector<std::string>();

	uint start=0, len=0;
	for(char c:text) {
		len++;
		if(len >= 50 && c == ' ') {
			notification_text.push_back(text.substr(start,len));
			start += len;
			len = 0;
		}
	}
	if(start < text.size()) {
		notification_text.push_back(text.substr(start));
	}
}

void PlayMode::hide_notification() {
	notification_active = false;
	notification_text = std::vector<std::string>();
	if (gameOver){
		exit(0);
	}
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
	const float unitFood = foodPrice;
	const float unitPrice = 10;

	return buy<float>(unitFood, unitPrice, totalFood, totalMoney);
}

bool PlayMode::buy_eggs() {
	std::cout << "buy_eggs" << std::endl;
	const size_t unitEggs = 10;
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

bool PlayMode::sell_mature() {
	std::cout << "sell_mature" << std::endl;
	const float price = 30;

	std::unordered_set<std::string> mature_cricket_names;
	std::vector<Cricket> mature_crickets;
	std::vector<Cricket> non_mature_crickets;
	bool success = false;

	for(Cricket &cricket: Crickets) {
		if(cricket.is_mature() && !cricket.is_dead()) {
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
		
		popups.emplace_back(glm::vec2(235,345), "+$" + std::to_string(int(profit)), glm::u8vec4(0x38, 0x66, 0x41, 0x00), 0.75);
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

std::string PlayMode::load_text_from_file(std::string filename) {
	std::string text = "";
  std::ifstream input(filename);
  for (std::string line; getline(input, line);) {
		text += line;
  }
	return text;
}

void PlayMode::draw_text_lines(glm::uvec2 const &drawable_size, float x, float y) {

	GLuint vbo{0}, vao{0}, texture{0}, sampler{0};

	GL_ERRORS();
	glUseProgram(text_program);
	GL_ERRORS();

	GLuint texUniform = glGetUniformLocation(text_program, "tex");
	glUniform1i(texUniform, 0);

	glGenBuffers(1, &vbo);
  glGenVertexArrays(1, &vao);
  glGenTextures(1, &texture);
  glGenSamplers(1, &sampler);
	GL_ERRORS();
  glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GL_ERRORS();

	// Set some GL state
  glEnable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0, 0, 0, 1);
	GL_ERRORS();

	GLuint fill_color_loc = glGetUniformLocation(text_program, "fillColor");
	GL_ERRORS();
	const GLfloat black[4] = {0.f, 0.f, 0.f, 1.0};
	glUniform4fv(fill_color_loc, 1, black);
	GL_ERRORS();

	// Bind stuff
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glBindSampler(0, sampler);
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

