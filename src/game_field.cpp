#include "game_field.h"

#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <queue>

using namespace godot;

void GameField::_bind_methods() {};

GameField::GameField() = default;
GameField::~GameField() = default;

int GameField::get_index_of_cell(int x, int y) { return x + y * _width; }

Vector2i GameField::get_coords_of_cell(int index) { return Vector2i(index % _width, index / _width); }

void GameField::reveal(int cell_index) {

  if (_field[cell_index].mines_around) {
    _field[cell_index].hidden = false;
    return;
  }

  std::queue<int> cells_to_reveal;
  cells_to_reveal.push(cell_index);

  while (!cells_to_reveal.empty()) {
    int index = cells_to_reveal.front();
    cells_to_reveal.pop();

    if (!_field[index].hidden)
      continue;

    Vector2i coordinates = get_coords_of_cell(index);
    int x = coordinates.x;
    int y = coordinates.y;

    _field[index].hidden = false;
    _field[index].flagged = false;

    if (_field[index].mines_around > 0)
      continue;

    for (int j = 0; j < 3; j++) {
      int y = index / _width - 1 + j;
      if (y < 0 || y >= _height)
        continue;

      for (int k = 0; k < 3; k++) {
        int x = index % _width - 1 + k;
        if (x < 0 || x >= _width || (coordinates.x == x && coordinates.y == y))
          continue;

        cells_to_reveal.push(get_index_of_cell(x, y));
      }
    }
  }
}

void GameField::reveal_all_hidden() {
  for (int i = 0; i < get_cells_quantity(); i++) {
    if (_field[i].hidden) {
      _field[i].hidden = false;
    }
  }
}
void GameField::toggle_flag(int index) {
  if (_field[index].hidden) {
    _field[index].flagged = !_field[index].flagged;
  } else {
    _field[index].flagged = false;
  }
}

void GameField::place_mines(int selected_cell = -1) {
  for (int i = 0; i < _mines_quantity; i++) {
    int index = 0;

    do {
      index = _random_generator.randi_range(0, _width * _height - 1);
    } while (_field[index].mine || index == selected_cell);

    _field[index].mine = true;

    for (int j = 0; j < 3; j++) {
      int y = index / _width - 1 + j;
      if (y < 0 || y >= _height)
        continue;

      for (int k = 0; k < 3; k++) {
        int x = index % _width - 1 + k;
        if (x < 0 || x >= _width)
          continue;
        _field[get_index_of_cell(x, y)].mines_around += 1;
      }
    }
  }
}
void GameField::prepare_field() {
  for (int i = 0; i < _width * _height; i++) {
    _field.push_back(Cell_t());
  }
}

void GameField::clear() {
  for (int i = 0; i < _width * _height; i++) {
    _field[i] = Cell_t();
  }
}

void GameField::start_game(int selected_cell) {
  clear();
  place_mines(selected_cell);
}

void GameField::set_properties(Vector2i resolution, int mines_quantity) {
  _width = resolution.x;
  _height = resolution.y;
  _mines_quantity = mines_quantity;

  if (_field.size() != resolution.x * resolution.y) {
    _field.clear();
    prepare_field();
  }
}

GameState GameField::get_game_state() {
  int flagged = 0;
  int guessed = 0;
  int hidden = 0;

  for (int i = 0; i < get_cells_quantity(); i++) {
    auto x = _field[i];
    if (x.mine && !x.hidden)
      return LOST;
    if (x.hidden && x.mine)
      guessed += 1;
    if (x.flagged && x.mine)
      flagged += 1;
    if (x.hidden)
      hidden += 1;
    if (x.flagged && !x.mine)
      flagged -= 1;
  }

  if (flagged == _mines_quantity || guessed == _mines_quantity && hidden == _mines_quantity) {
    return WIN;
  }

  return PLAYING;
}

Cell_t GameField::get_cell(int index) { return _field[index]; }
int GameField::get_mines_quantity() { return _mines_quantity; }
Vector2i GameField::get_field_resolution() { return Vector2i(_width, _height); }
int GameField::get_cells_quantity() { return _width * _height; }
