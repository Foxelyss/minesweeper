#ifndef FIELD_H
#define FIELD_H

#include <cstdint>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/variant/vector2i.hpp>

typedef struct Cell {
  uint8_t mines_around = 0;
  bool mine = false;
  bool hidden = true;
  bool flagged = false;
} Cell_t;

enum GameState { PLAYING, WIN, LOST };
using namespace std;

namespace godot {

class GameField : public Node {
  GDCLASS(GameField, Node);

private:
  RandomNumberGenerator _random_generator;
  uint8_t _width, _height;
  uint8_t _mines_quantity;

  vector<Cell_t> _field;

  int get_index_of_cell(int x, int y);

  Vector2i get_coords_of_cell(int index);

  void place_mines(int selected_cell);

  void prepare_field();

public:
  static void _bind_methods();

  GameField();
  ~GameField();

  void start_game(int selected_cell = -1);
  void set_properties(Vector2i resolution, int mines_quantity);
  void reveal(int cell_index);
  void reveal_all_hidden();

  void clear();

  void toggle_flag(int index);

  GameState get_game_state();

  Cell_t get_cell(int index);
  int get_mines_quantity();
  Vector2i get_field_resolution();
  int get_cells_quantity();
};

} // namespace godot

#endif
