#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "field_renderer.h"
#include "game_field.h"

#include <godot_cpp/classes/animation_player.hpp>

namespace godot {

class MainMenu : public Node {
  GDCLASS(MainMenu, Node)

private:
  GameField *_game_field = nullptr;
  AnimationPlayer *_game_flow_animation_player = nullptr;
  AnimationPlayer *_background_animation_player = nullptr;
  FieldRenderer *_field_renderer = nullptr;

protected:
  static void _bind_methods();

public:
  MainMenu();
  ~MainMenu();

  void _ready() override;
  void handle_button_press(int index);
};

} // namespace godot

#endif
