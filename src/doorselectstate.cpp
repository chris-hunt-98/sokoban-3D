#include "doorselectstate.h"

#include "gameobject.h"
#include "room.h"
#include "roommap.h"
#include "door.h"

DoorSelectState::DoorSelectState(Room* room, Point3 cam_pos, Point3* door_pos, Door** door):
EditorBaseState(), room_ {room}, cam_pos_ {cam_pos}, door_pos_ {door_pos}, door_ {door} {}

DoorSelectState::~DoorSelectState() {}

void DoorSelectState::main_loop() {
    bool p_open = true;
    if (!ImGui::Begin("Door Destination Select Window##DOOR", &p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    handle_mouse_input(cam_pos_, room_);
    handle_keyboard_input(cam_pos_, room_);
    room_->draw(gfx_, cam_pos_, true, one_layer_);

    if (door_pos_->x == -1) {
        ImGui::Text("Destination not selected.");
    } else {
        //ImGui::Text(("Destination Position: " + door_pos_->to_str()).c_str());
    }
    ImGui::Text("Press escape to return.");

    ImGui::End();
}

void DoorSelectState::handle_left_click(Point3 pos) {
    *door_pos_ = pos;
    *door_ = dynamic_cast<Door*>(room_->room_map()->view(*door_pos_)->modifier());
}

void DoorSelectState::handle_right_click(Point3 pos) {}
