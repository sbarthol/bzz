#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

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


void PlayMode::spawn_cricket() {
	Mesh const &mesh = bzz_meshes->lookup("Cricket");

	scene.transforms.emplace_back();
	Scene::Transform *transform = &scene.transforms.back();
	transform->position = cricket_transform->position + glm::vec3(get_rng_range(-0.5,0.5), get_rng_range(-0.5,0.5), 0.0);
	transform->rotation = glm::angleAxis(glm::radians(get_rng_range(0.f,360.f)), glm::vec3(0.0,0.0,1.0));
	transform->scale = glm::vec3(1.f);

	scene.drawables.emplace_back(Scene::Drawable(transform));
	Scene::Drawable &drawable = scene.drawables.back();

	drawable.pipeline = lit_color_texture_program_pipeline;

	drawable.pipeline.vao = bzz_meshes_for_lit_color_texture_program;
	drawable.pipeline.type = mesh.type;
	drawable.pipeline.start = mesh.start;
	drawable.pipeline.count = mesh.count;

	Crickets.push_back(Cricket(Cricket::seq++, transform));
}

PlayMode::PlayMode() : scene(*bzz_scene) {

	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {
		if (transform.name == "Cricket") {
			cricket_transform = &transform;
			cricket_transform->scale = glm::vec3(0.f);
		}
		if (transform.name == "Bedding") {
			bedding_transform = &transform;
		}
	}

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	// Spawn the first cricket
	spawn_cricket();
	
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
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
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	

	//spawn a new cricket
	{
		const float CricketSpawnPeriod = 3.0;
		elapsed_since_spawn += elapsed;
		if (elapsed_since_spawn > CricketSpawnPeriod) {
			elapsed_since_spawn -= CricketSpawnPeriod;
			spawn_cricket();
		}
	}

	// Move some crickets randomly
	{
		const float JumpDistance = 0.1;
		for(Cricket &cricket: Crickets) {
			if(std::rand() % 40 == 0) {

				glm::quat quat = glm::angleAxis(glm::radians(get_rng_range(-20.f,20.f)), glm::vec3(0.0,0.0,1.0));
				cricket.transform->rotation = glm::normalize(cricket.transform->rotation * quat);

				glm::vec3 dir = cricket.transform->rotation * glm::vec3(0.f, 1.f, 0.f) ;
				dir = JumpDistance * glm::normalize(dir);
				cricket.transform->position += dir;

				Mesh const &mesh = bzz_meshes->lookup("Bedding");
				glm::mat4x3 to_world = bedding_transform->make_local_to_world();
				glm::vec3 mesh_min = to_world * glm::vec4(mesh.min, 1.f);
				glm::vec3 mesh_max = to_world * glm::vec4(mesh.max, 1.f);

				glm::vec3 &pos = cricket.transform->position;
				const float eps = 0.2;
				if (!(mesh_min.x + eps < pos.x && pos.x < mesh_max.x - eps && mesh_min.y + eps < pos.y && pos.y < mesh_max.y - eps)) {
					glm::quat turn_around = glm::angleAxis(glm::radians(180.f), glm::vec3(0.0,0.0,1.0));
					cricket.transform->rotation = glm::normalize(cricket.transform->rotation * turn_around);

					pos.x = glm::clamp(pos.x, mesh_min.x + eps, mesh_max.x - eps);
					pos.y = glm::clamp(pos.y, mesh_min.y + eps, mesh_max.y - eps);
				}
			}
		}
	}

	//update state
	{
		//update food
		totalFood -= numLiveCrickets * cricketEatingRate;
		if (totalFood <= 0){
			totalFood = 0;
			int deathProbability = rand() % 10 + 1;
			if (deathProbability < 7){
				numLiveCrickets --;
				numDeadCrickets ++;
				if (Crickets.size() > 0){
					Cricket c = Crickets.at(0);
					Crickets.erase(Crickets.begin());
					if (c.age >= c.matureAge){
						numMatureCrickets --;
					}else{
						numBabyCrickets --;
					}
				}
			}
		}
		//update crickets
		for (size_t i  = 0; i < Crickets.size(); i++){
			Crickets.at(i).age += .001;
			if (Crickets.at(i).age > Crickets.at(i).matureAge){
				numBabyCrickets --;
				numMatureCrickets ++;
			}
			if (Crickets.at(i).age > Crickets.at(i).lifeSpan){
				Crickets.erase(Crickets.begin()+i);
				numDeadCrickets ++;
				numMatureCrickets --;
			}
		}
	}

	//move camera:
	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 frame_forward = -frame[2];

		camera->transform->position += move.x * frame_right + move.y * frame_forward;
	}

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		glm::vec3 frame_at = frame[3];
		Sound::listener.set_position_right(frame_at, frame_right, 1.0f / 60.0f);
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
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}


