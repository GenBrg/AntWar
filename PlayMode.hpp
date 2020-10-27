#include "Mode.hpp"

#include "Connection.hpp"
#include "Scene.hpp"
#define CLIENT
#include "Player.hpp"
#include "Message.hpp"
#include "NetCommon.hpp"
#include "Load.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode(Client &client);
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
	} left, right, down, up, 
	arrow_left, arrow_right, arrow_down, arrow_up,
	enter;

	MessageDispatcher<ChannelId, 4> channel_dispatcher { kMagicHeader };
	Player left_side;
	Player right_side;
	Player* player { nullptr };

	//connection to server:
	Client &client;

	Scene scene;

	//camera:
	Scene::Camera *camera = nullptr;

	bool game_start { false };
};

extern Load< MeshBuffer > ant_war_meshes;
extern Load< Scene > ant_war_scene;
