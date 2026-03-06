#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "game_field.h"

#include <godot_cpp/classes/animation_player.hpp>

namespace godot {

class MainMenu : public Node {
  GDCLASS(MainMenu, Node)

private:
  GameField *_game_field;
  AnimationPlayer *_game_flow_animation_player;

protected:
  static void _bind_methods();

public:
  MainMenu();
  ~MainMenu();

  void _ready() override;
  void handle_button_press(int index);
  void _process(double delta) override;
};

} // namespace godot

#endif
