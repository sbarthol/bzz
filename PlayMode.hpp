#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

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
	float elapsed_since_spawn = 0.0;

	struct Cricket {
		int cricketID;
		float lifeSpan; // is there a constant lifespan
		float matureAge = 1.5; //age that cricket is mature
		// or is the time of death determined by environment?
		float age;
	};

	void spawn_cricket();
	Scene::Transform *first_cricket; 
	std::vector<Cricket> Crickets;
	float total_elapsed = 0.0;
	size_t numLiveCrickets = 0;
	float cricketEatingRate = .2;
	size_t numBabyCrickets = 0;
	size_t numMatureCrickets = 0;
	size_t numDeadCrickets = 0;
	float totalMoney = 0;
	float totalFood = 0;


	

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
