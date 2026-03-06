#include "main_menu.h"
#include "field_renderer.h"

#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector2i.hpp>

using namespace godot;

void MainMenu::_bind_methods() { ClassDB::bind_method(D_METHOD("handle_button_press", "index"), &MainMenu::handle_button_press); }

MainMenu::MainMenu() = default;

MainMenu::~MainMenu() = default;

void MainMenu::_ready() {
  if (Engine::get_singleton()->is_editor_hint()) {
    set_process_mode(Node::ProcessMode::PROCESS_MODE_DISABLED);
    return;
  }
  auto buttons = get_node<VBoxContainer>("../Buttons");

  for (int i = 0; i < buttons->get_child_count(); i++) {
    auto v = Callable(this, "handle_button_press").bind(i);

    buttons->get_child(i)->get_node<Button>(".")->connect("pressed", v);
  }

  _game_field = get_node<GameField>("/root/FieldRepresenter");
  _game_flow_animation_player = get_node<AnimationPlayer>("/root/Game/GameFlowAnimationPlayer");

  get_node<AnimationPlayer>("/root/Game/BackgroundAnimationPlayer")->set_current_animation("flow");
  _game_flow_animation_player->set_current_animation("pop_in");
}

void MainMenu::handle_button_press(int index) {
  Vector2i resolution;
  int mines_quantity;

  switch (index) {
  case 0:
    resolution = Vector2i(8, 8);
    mines_quantity = 10;
    break;
  case 1:
    resolution = Vector2i(16, 16);
    mines_quantity = 40;
    break;
  case 2:
    resolution = Vector2i(30, 16);
    mines_quantity = 99;
    break;
  case 3:
    get_node<FieldRenderer>("/root/Game/MainGame/FieldRenderer")->show_records();
    return;
    break;
  case 4:
    _game_flow_animation_player->set_current_animation("pop_out");
    get_tree()->quit();
    return;
    break;
  }

  _game_flow_animation_player->set_current_animation("to_game");
  _game_field->set_properties(resolution, mines_quantity);
  get_node<FieldRenderer>("/root/Game/MainGame/FieldRenderer")->start_game();
}

void MainMenu::_process(double delta) {}
