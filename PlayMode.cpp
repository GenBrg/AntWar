#include "PlayMode.hpp"

#include "DrawLines.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "hex_dump.hpp"
#include "Scene.hpp"
#include "Load.hpp"
#include "LitColorTextureProgram.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

Load< MeshBuffer > ant_war_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer *ret = new MeshBuffer(data_path("AntWar.pnct"));
	ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > ant_war_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("AntWar.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = ant_war_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.mesh = &mesh;
	});
});

PlayMode::PlayMode(Client &client_) : 
client(client_),
scene(*ant_war_scene),
left_side(ChannelId::LEFT_SIDE),
right_side(ChannelId::RIGHT_SIDE)
{
	channel_dispatcher.RegisterMessageCallback(ChannelId::CONTROL_CHANNEL, [&](MessageReader<ChannelId, 4> reader){
		bool is_left_side;
		reader >> is_left_side;

		game_start = true;
		std::cout << "Game starts!" << std::endl;
		player = (is_left_side) ? &left_side : &right_side;
		camera->transform->position.y = (is_left_side) ? -5.0f : 5.0f;
	});

	channel_dispatcher.RegisterMessageCallback(ChannelId::LEFT_SIDE, [&](MessageReader<ChannelId, 4> reader){
		left_side.GetMessageDispatcher().OnRecv(reader.GetBuffer());
	});

	channel_dispatcher.RegisterMessageCallback(ChannelId::RIGHT_SIDE, [&](MessageReader<ChannelId, 4> reader){
		right_side.GetMessageDispatcher().OnRecv(reader.GetBuffer());
	});

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	if (!game_start) {
		return false;
	}

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.repeat) {
			//ignore repeats
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
		} else if (evt.key.keysym.sym == SDLK_LEFT) {
			arrow_left.downs += 1;
			arrow_left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			arrow_right.downs += 1;
			arrow_right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			arrow_up.downs += 1;
			arrow_up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			arrow_down.downs += 1;
			arrow_down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RETURN) {
			enter.downs += 1;
			enter.pressed = true;
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
		} else if (evt.key.keysym.sym == SDLK_LEFT) {
			arrow_left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			arrow_right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			arrow_up.pressed = false;
			return true;
		}else if (evt.key.keysym.sym == SDLK_DOWN) {
			arrow_down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RETURN) {
			enter.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	client.poll([this](Connection *c, Connection::Event event){
		if (event == Connection::OnOpen) {
			std::cout << "[" << c->socket << "] opened" << std::endl;
		} else if (event == Connection::OnClose) {
			std::cout << "[" << c->socket << "] closed (!)" << std::endl;
			throw std::runtime_error("Lost connection to server!");
		} else { assert(event == Connection::OnRecv);
			// std::cout << "[" << c->socket << "] recv'd data. Current buffer:\n" << hex_dump(c->recv_buffer); std::cout.flush();
			if (!channel_dispatcher.OnRecv(c->recv_buffer)) {
				throw std::runtime_error("Unexpected packet");
			}
		}
	}, 0.0);

	if (!game_start) {
		return;
	}

	// Update camera
	constexpr float kCameraScrollingSpeed { 3.0f };
	float& camera_y = camera->transform->position.y;
	float& camera_z = camera->transform->position.z;

	if (up.pressed && !down.pressed) camera_z += kCameraScrollingSpeed * elapsed;
	if (!up.pressed && down.pressed) camera_z -= kCameraScrollingSpeed * elapsed;
	if (right.pressed && !left.pressed) camera_y += kCameraScrollingSpeed * elapsed;
	if (!right.pressed && left.pressed) camera_y -= kCameraScrollingSpeed * elapsed;

	camera_y = glm::clamp(camera_y, -5.0f, 5.0f);
	camera_z = glm::clamp(camera_z, -1.0f, 2.0f);

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;

	//send/receive data:
	if (enter.pressed) {
		player->SummonAntStub(Ant::Type::FIGHTER);
	}

	player->SendRpcs(client.connection.send_buffer);
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	GL_ERRORS();
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	GL_ERRORS();
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	GL_ERRORS();
	glUseProgram(0);

	glClearColor(0.4f, 0.9f, 0.9f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	left_side.Draw(scene);
	right_side.Draw(scene);
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

		auto draw_text = [&](glm::vec2 const &at, std::string const &text, float H) {
			lines.draw_text(text,
				glm::vec3(at.x, at.y, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0x00, 0x00, 0x00, 0x00));
			float ofs = 2.0f / drawable_size.y;
			lines.draw_text(text,
				glm::vec3(at.x + ofs, at.y + ofs, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		};

		// draw_text(glm::vec2(-aspect + 0.1f, 0.0f), server_message, 0.09f);

		// draw_text(glm::vec2(-aspect + 0.1f,-0.9f), "(press WASD to change your total)", 0.09f);
	}
	GL_ERRORS();
}
