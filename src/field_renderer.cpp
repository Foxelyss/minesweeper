#include "field_renderer.h"
#include "game_field.h"

#include <godot_cpp/classes/animated_sprite2d.hpp>
#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_magnify_gesture.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/input_event_pan_gesture.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/tab_container.hpp>
#include <godot_cpp/classes/texture_button.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>

using namespace godot;

void FieldRenderer::_bind_methods() {
  ClassDB::bind_method(D_METHOD("on_button_pressed", "input", "index"), &FieldRenderer::_on_button_pressed);
  ClassDB::bind_method(D_METHOD("retry_game"), &FieldRenderer::reset_game_state);
  ClassDB::bind_method(D_METHOD("go_to_menu"), &FieldRenderer::go_to_menu);

  ClassDB::bind_method(D_METHOD("set_mines_around_label", "mines_around_label"), &FieldRenderer::set_mines_around_label);
  ClassDB::bind_method(D_METHOD("get_mines_around_label"), &FieldRenderer::get_mines_around_label);
  ClassDB::add_property("FieldRenderer", PropertyInfo(Variant::NODE_PATH, "mines_around_label"), "set_mines_around_label", "get_mines_around_label");

  ClassDB::bind_method(D_METHOD("set_time_label", "time_label"), &FieldRenderer::set_time_label);
  ClassDB::bind_method(D_METHOD("get_time_label"), &FieldRenderer::get_time_label);
  ClassDB::add_property("FieldRenderer", PropertyInfo(Variant::NODE_PATH, "time_label"), "set_time_label", "get_time_label");

  ClassDB::bind_method(D_METHOD("set_retry_button", "retry_button"), &FieldRenderer::set_retry_button);
  ClassDB::bind_method(D_METHOD("get_retry_button"), &FieldRenderer::get_retry_button);
  ClassDB::add_property("FieldRenderer", PropertyInfo(Variant::NODE_PATH, "retry_button"), "set_retry_button", "get_retry_button");

  ClassDB::bind_method(D_METHOD("set_flagging_radio_button", "flagging_radio-button"), &FieldRenderer::set_flagging_radio_button);
  ClassDB::bind_method(D_METHOD("get_flagging_radio_button"), &FieldRenderer::get_flagging_radio_button);
  ClassDB::add_property("FieldRenderer", PropertyInfo(Variant::NODE_PATH, "flagging_radio-button"), "set_flagging_radio_button",
                        "get_flagging_radio_button");

  ClassDB::bind_method(D_METHOD("set_game_status_label", "game_status_label"), &FieldRenderer::set_game_status_label);
  ClassDB::bind_method(D_METHOD("get_game_status_label"), &FieldRenderer::get_game_status_label);
  ClassDB::add_property("FieldRenderer", PropertyInfo(Variant::NODE_PATH, "game_status_label"), "set_game_status_label", "get_game_status_label");

  ClassDB::bind_method(D_METHOD("set_back_to_menu_button", "back_to_menu_button"), &FieldRenderer::set_back_to_menu_button);
  ClassDB::bind_method(D_METHOD("get_back_to_menu_button"), &FieldRenderer::get_back_to_menu_button);
  ClassDB::add_property("FieldRenderer", PropertyInfo(Variant::NODE_PATH, "back_to_menu_button"), "set_back_to_menu_button",
                        "get_back_to_menu_button");
};

FieldRenderer::FieldRenderer() {};
FieldRenderer::~FieldRenderer() {};

void FieldRenderer::_ready() {
  if (Engine::get_singleton()->is_editor_hint()) {
    set_process_mode(Node::ProcessMode::PROCESS_MODE_DISABLED);
    return;
  } else {
    set_process_mode(Node::ProcessMode::PROCESS_MODE_INHERIT);
  }

  _menu = get_node<Control>("..");
  _time_label = get_node<Label>(NodePath(_time_label_path));
  _mines_around_label = get_node<Label>(NodePath(_mines_around_label_path));
  _game_status_label = get_node<Label>(NodePath(_game_status_label_path));
  _retry_button = get_node<Button>(NodePath(_retry_button_path));
  _back_to_menu_button = get_node<Button>(NodePath(_back_to_menu_button_path));
  _flagging_radio_button = get_node<Button>(NodePath(_flagging_radio_button_path));

  _back_to_menu_button->connect("pressed", Callable(this, "go_to_menu"));
  _retry_button->connect("pressed", Callable(this, "retry_game"));

  _game_field = get_node<GameField>("/root/FieldRepresenter");
  _grid = get_node<GridContainer>("../DraggableSpace/GameGrid");

  _ui_tweener = get_node<AnimationPlayer>("/root/Game/GameFlowAnimationPlayer");
  _pop_animator = get_node<AnimatedSprite2D>("../AnimatedSprite2D");
  _sfx = get_node<AudioStreamPlayer>("/root/Game/SFXPlayer");
  _music_player = get_node<AudioStreamPlayer>("/root/Game/BackgroundMusicPlayer");

  _timer = memnew(Timer);
  add_child(_timer);
  _timer->set_one_shot(true);
  _timer->set_wait_time(_time_before_timeout);

  ResourceLoader *resource_loader = ResourceLoader::get_singleton();

  for (int i = 1; i <= 13; i++) {
    _cells_textures.push_back(resource_loader->load("res://assets/Cell_Page " + String::num_int64(i) + ".png"));
  }

  _win_sound = resource_loader->load("res://sfx/win.wav");
  _lose_sound = resource_loader->load("res://sfx/lose.wav");
  _pop_sound = resource_loader->load("res://sfx/pop.wav");
}

void FieldRenderer::create_records_file() {
  auto file = FileAccess::open(RECORDS_FILENAME, FileAccess::ModeFlags::WRITE);
  file->store_string("[[0,0,0],[0,0,0],[0,0,0]]");
}

int FieldRenderer::get_game_category() {
  switch (_game_field->get_mines_quantity()) {
  case 40:
    return 1;
    break;
  case 99:
    return 2;
  }

  return 0;
}

void FieldRenderer::show_records() {
  if (!FileAccess::file_exists(RECORDS_FILENAME)) {
    create_records_file();
  }
  auto file = FileAccess::open(RECORDS_FILENAME, FileAccess::ModeFlags::READ);

  JSON json;
  Error error = json.parse(file->get_line());
  Array records_table = json.get_data();
  Popup *menu = get_node<Popup>("../PopupMenu");
  TabContainer *records_view = menu->get_node<TabContainer>("ui/Records");
  int category = get_game_category();

  for (int j = 0; j < 3; j++) {
    Node *group = records_view->get_child(j);
    Array column = ((Array)records_table[j]);

    for (int i = 0; i < 3; i++) {
      group->get_child(i)->get_node<Label>(".")->set_text(format_time(column[i]));
    }
  }

  menu->show();
}

void FieldRenderer::save_record(int time) {
  if (!FileAccess::file_exists(RECORDS_FILENAME)) {
    create_records_file();
  }
  auto file = FileAccess::open(RECORDS_FILENAME, FileAccess::ModeFlags::READ);

  JSON json;
  Error error = json.parse(file->get_line());
  Array records = json.get_data();
  int category = get_game_category();

  auto column = ((Array)records[category]);

  while (column.has(Variant(0.0))) {
    column.erase(Variant(0.0));
  }

  column.append(Variant(time));
  column.sort();
  column.resize(3);

  file->close();
  file = FileAccess::open(RECORDS_FILENAME, FileAccess::ModeFlags::WRITE_READ);

  file->store_string(json.stringify(records));
}

void FieldRenderer::show_best_record() {
  if (!FileAccess::file_exists(RECORDS_FILENAME)) {
    create_records_file();
  }
  auto file = FileAccess::open(RECORDS_FILENAME, FileAccess::ModeFlags::READ);

  JSON json;
  Error err = json.parse(file->get_line());
  Array records_table = json.get_data();
  int index = get_game_category();

  auto records_for_category = ((Array)records_table[index]);

  if ((int)records_for_category[0] == 0) {
    get_node<Label>("../MarginContainer/Control/TimeSegments/Records")->set_text(tr("NORECORD"));
  } else {
    get_node<Label>("../MarginContainer/Control/TimeSegments/Records")->set_text(vformat(tr("RECORD"), format_time((int)records_for_category[0])));
  }
}

void FieldRenderer::start_game() {
  Vector2i resolution = _game_field->get_field_resolution();
  _grid->set_columns(resolution.x);

  for (int i = 0; i < _grid->get_child_count(); i++) {
    _grid->get_child(i)->queue_free();
  }

  for (int i = 0; i < _game_field->get_cells_quantity(); i++) {
    TextureButton *button = memnew(TextureButton());
    _grid->add_child(button);

    button->connect("gui_input", Callable(this, "on_button_pressed").bind(i));

    int index = 11;
    button->set_texture_normal(_cells_textures[index]);
  }

  reset_game_state();

  _music_player->play();
}

void FieldRenderer::reset_game_state() {
  _first_cell = -1;
  _game_field->clear();

  show_best_record();
  _game_status_label->set_text("");
  _mines_around_label->set_text("");

  for (int i = 0; i < _game_field->get_cells_quantity(); i++) {
    TextureButton *target = _grid->get_child(i)->get_node<TextureButton>(".");

    target->set_disabled(false);

    int index = 11;
    target->set_texture_normal(_cells_textures[index]);
  }
}

String FieldRenderer::format_time(int time) {
  int minutes, seconds;

  minutes = time / 60;
  seconds = time % 60;

  return String::num_int64(minutes).lpad(2, "0") + ":" + String::num_int64(seconds).lpad(2, "0");
}

void FieldRenderer::_process(float delta) {
  String time_left;

  if (_timer->get_time_left() == 0) {
    time_left = tr("TIMEOUT");
  } else {
    time_left = vformat(tr("TIMECOUNTER"), format_time(_time_before_timeout - _timer->get_time_left()));
  }

  if (_first_cell == -1) {
    time_left = "";
  }

  _time_label->set_text(time_left);

  auto input = Input::get_singleton();

  if (!input->is_action_pressed("move_mode")) {
    _dragging = false;
    input->set_mouse_mode(Input::MOUSE_MODE_VISIBLE);
  }

  if (!is_grid_fully_on_screen()) {
    int mouse_scroll = -1 * input->is_action_just_pressed("scroll_down") + input->is_action_just_pressed("scroll_up");

    Vector2 scale = _grid->get_scale() + Vector2(1, 1) * mouse_scroll * 10.f * delta;
    scale = scale.clamp(Vector2(0.4, 0.4), Vector2(1.2, 1.2));
    _grid->set_scale(scale);
  }
}

void FieldRenderer::_on_button_pressed(InputEvent *event, int index) {
  TextureButton *target = _grid->get_child(index)->get_node<TextureButton>(".");

  if (_dragging) {
    return;
  }

  if (event->get_class() == "InputEventMouseButton" && ((InputEventMouseButton *)event)->is_released() && !event->is_echo() &&
      !target->is_disabled()) {
    InputEventMouseButton *mouse_event = (InputEventMouseButton *)event;

    if (mouse_event->get_button_index() == MOUSE_BUTTON_RIGHT || _flagging_radio_button->is_pressed()) {
      _game_field->toggle_flag(index);
    } else if (mouse_event->get_button_index() == MOUSE_BUTTON_LEFT) {
      if (_first_cell == -1) {
        _game_field->start_game(index);

        String string = vformat(tr("MINESAROUND"), String::num_int64(_game_field->get_mines_quantity()));
        _mines_around_label->set_text(string);

        _first_cell = index;
        _timer->set_paused(false);
        _timer->start();
      }

      if (!_game_field->get_cell(index).flagged) {
        _pop_animator->set_position(target->get_global_position() + target->get_size() / 2);

        _pop_animator->play("popit");

        _sfx->set_stream(_pop_sound);
        _sfx->play();
        _game_field->reveal(index);
      }
    }
    update_grid();
  }
}

void FieldRenderer::update_grid() {
  if (_first_cell == -1) {
    return;
  }

  GameState game_state = update_game_status();

  for (int i = 0; i < _game_field->get_cells_quantity(); i++) {
    TextureButton *target = _grid->get_child(i)->get_node<TextureButton>(".");
    Cell_t current_cell = _game_field->get_cell(i);
    int texture_index = 0;

    if (current_cell.flagged) {
      texture_index += 10;
    } else if (current_cell.hidden) {
      texture_index += 11;
    } else if (current_cell.mine) {
      texture_index += 9;
    } else if (current_cell.mines_around >= 0) {
      texture_index += current_cell.mines_around;
    }
    if (game_state == GameState::LOST && current_cell.flagged && !current_cell.mine) {
      texture_index = 12;
    }

    target->set_texture_normal(_cells_textures[texture_index]);

    target->set_disabled(!current_cell.hidden);
  }
}

GameState FieldRenderer::update_game_status() {
  GameState game_state = _game_field->get_game_state();

  if (game_state != PLAYING) {
    _timer->set_paused(true);
    _game_field->reveal_all_hidden();
  }

  switch (game_state) {
  case WIN:
    _game_status_label->set_text(tr("WIN"));
    save_record(_time_before_timeout - _timer->get_time_left());
    _ui_tweener->play("win");

    _sfx->set_stream(_win_sound);
    _sfx->play();

    show_best_record();
    break;
  case LOST:
    _ui_tweener->play("lose");

    _sfx->set_stream(_lose_sound);
    _sfx->play();

    _game_status_label->set_text(tr("LOSE"));
    break;
  default:
    _game_status_label->set_text("");
    break;
  }

  return game_state;
}

void FieldRenderer::go_to_menu() {
  _music_player->stop();

  _ui_tweener->play_backwards("to_game");
}

bool FieldRenderer::is_grid_fully_on_screen() {
  Vector2 grid = _grid->get_size();
  Vector2 menu = _menu->get_size();
  return grid.x / menu.x < 0.7 && grid.y / menu.y < 0.7;
}

void FieldRenderer::_input(Ref<InputEvent> event) {
  auto input = Input::get_singleton();

  Ref<InputEventMouseMotion> mouse_event = event;
  Ref<InputEventPanGesture> inputEventPanGesture = event;
  Ref<InputEventMagnifyGesture> inputEventMagnifyGesture = event;

  if (mouse_event.is_valid() && mouse_event->get_relative().length() > 4 && input->is_action_pressed("move_mode") && !is_grid_fully_on_screen()) {
    _dragging = true;

    move_grid(mouse_event->get_relative().x, mouse_event->get_relative().y);

  } else if (inputEventPanGesture.is_valid()) {
    _dragging = true;

    move_grid(-inputEventPanGesture->get_delta().x, -inputEventPanGesture->get_delta().y);
  }

  if (inputEventMagnifyGesture.is_valid()) {
    Vector2 scale = Vector2(inputEventMagnifyGesture->get_factor() * _grid->get_scale()).clamp(Vector2(0.4, 0.4), Vector2(1.2, 1.2));

    _dragging = true;

    if (!is_grid_fully_on_screen())
      _grid->set_scale(scale);
    _grid->set_position(_grid->get_position() * inputEventMagnifyGesture->get_factor());
  }

  if (_dragging) {
    input->set_mouse_mode(Input::MOUSE_MODE_CONFINED_HIDDEN);
  }
}

void FieldRenderer::move_grid(float x, float y) {
  auto grid_position = _grid->get_position();
  Vector2 size = _grid->get_size();

  grid_position.y = Math::clamp<float>(grid_position.y + y, -size.y, 0);
  grid_position.x = Math::clamp<float>(grid_position.x + x, -size.x, 0);
  _grid->set_position(grid_position);
}

NodePath FieldRenderer::get_mines_around_label() { return _mines_around_label_path; };
void FieldRenderer::set_mines_around_label(NodePath path) { _mines_around_label_path = path; };

NodePath FieldRenderer::get_time_label() { return _time_label_path; };
void FieldRenderer::set_time_label(NodePath path) { _time_label_path = path; };

NodePath FieldRenderer::get_retry_button() { return _retry_button_path; };
void FieldRenderer::set_retry_button(NodePath path) { _retry_button_path = path; };

NodePath FieldRenderer::get_flagging_radio_button() { return _flagging_radio_button_path; };
void FieldRenderer::set_flagging_radio_button(NodePath path) { _flagging_radio_button_path = path; };

NodePath FieldRenderer::get_game_status_label() { return _game_status_label_path; };
void FieldRenderer::set_game_status_label(NodePath path) { _game_status_label_path = path; };

NodePath FieldRenderer::get_back_to_menu_button() { return _back_to_menu_button_path; };
void FieldRenderer::set_back_to_menu_button(NodePath path) { _back_to_menu_button_path = path; };
