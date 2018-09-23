#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

#include <dear/imgui.h>

#pragma GCC diagnostic pop

#include "editor.h"
#include "room.h"
#include "block.h"

Editor::Editor(GLFWwindow* window, Room* room): window_ {window}, room_ {room}, pos_ {Point{0,0}},
save_load_tab_ {SaveLoadTab(room)},
object_tab_ {ObjectTab(room)},
camera_tab_ {CameraTab(room)}
{
    active_tab_ = &save_load_tab_;
}

Point Editor::pos() {
    return pos_;
}

void Editor::shift_pos(Point d) {
    pos_ = Point{pos_.x + d.x, pos_.y + d.y};
}

void Editor::set_pos(Point p) {
    pos_ = p;
}

void Editor::clamp_pos(int width, int height) {
    pos_ = Point {
        std::max(0, std::min(width-1, pos_.x)),
        std::max(0, std::min(height-1, pos_.y))
    };
}

void Editor::ShowMainWindow(bool* p_open) {
    if (!ImGui::Begin("My Editor Window", p_open, 0)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Save/Load")) {
        active_tab_ = &save_load_tab_;
    } ImGui::SameLine();
    if (ImGui::Button("Create/Delete Objects")) {
        active_tab_ = &object_tab_;
    } ImGui::SameLine();
    if (ImGui::Button("Camera")) {
        active_tab_ = &camera_tab_;
    }

    active_tab_->draw();

    ImGui::End();
}

bool Editor::want_capture_keyboard() {
    return ImGui::GetIO().WantCaptureKeyboard;
}

bool Editor::want_capture_mouse() {
    return ImGui::GetIO().WantCaptureMouse;
}

void Editor::handle_input() {
    if (want_capture_mouse()) {
        return;
    }
    Point mouse_pos = room_->get_pos_from_mouse();
    if (!room_->valid(mouse_pos)) {
        return;
    }
    if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        active_tab_->handle_left_click(mouse_pos);
    } else if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        active_tab_->handle_right_click(mouse_pos);
    }
}

void SaveLoadTab::draw() {
    static char buf[32] = "";
    ImGui::InputText(".map", buf, IM_ARRAYSIZE(buf));
    if (ImGui::Button("Load Map")) {
        room_->load(buf);
    }
    if (ImGui::Button("Save Map")) {
        room_->save(buf, false);
    }
    if (ImGui::Button("Save Map (Force Overwrite)")) {
        room_->save(buf, true);
    }
}

void ObjectTab::draw() {
    ImGui::RadioButton("Wall", &solid_obj, (int)ObjCode::Wall);
    ImGui::RadioButton("PushBlock", &solid_obj, (int)ObjCode::PushBlock);
    ImGui::RadioButton("SnakeBlock", &solid_obj, (int)ObjCode::SnakeBlock);

    ImGui::Text("Object Properties");

    if (solid_obj == (int)ObjCode::Wall) { }
    else if (solid_obj == (int)ObjCode::PushBlock) {
        ImGui::Text("Stickiness");
        ImGui::RadioButton("Not Sticky", &pb_sticky, (int)StickyLevel::None);
        ImGui::RadioButton("Weakly Sticky", &pb_sticky, (int)StickyLevel::Weak);
        ImGui::RadioButton("Strongly Sticky", &pb_sticky, (int)StickyLevel::Strong);

        ImGui::Checkbox("Is Player?", &is_car);
    }
    else if (solid_obj == (int)ObjCode::SnakeBlock) {
        ImGui::Text("Number of Ends");
        ImGui::RadioButton("One Ended", &sb_ends, 1);
        ImGui::RadioButton("Two Ended", &sb_ends, 2);

        ImGui::Checkbox("Is Player?", &is_car);
    }
}

void CameraTab::draw() {
    if (ImGui::Button("Create Camera Rect")) {
        int x = std::min(x1, x2);
        int y = std::min(y1, y2);
        int w = abs(x1 - x2) + 1;
        int h = abs(y1 - y2) + 1;
        camera_->push_context(std::make_unique<FixedCameraContext>(x, y, w, h, priority, radius, x + w/2.0, y + h/2.0));
    }
}

void SaveLoadTab::handle_left_click(Point) {}
void SaveLoadTab::handle_right_click(Point) {}

void ObjectTab::handle_left_click(Point pos) {
    int x = pos.x;
    int y = pos.y;
    std::unique_ptr<GameObject> obj;
    switch (solid_obj) {
    case (int)ObjCode::Wall :
        obj = std::make_unique<Wall>(x, y);
        break;
    case (int)ObjCode::PushBlock :
        obj = std::make_unique<PushBlock>(x, y, is_car, static_cast<StickyLevel>(pb_sticky));
        break;
    case (int)ObjCode::SnakeBlock :
        obj = std::make_unique<SnakeBlock>(x, y, is_car, sb_ends);
        break;
    default:
        return;
    }
    room_->create_obj(std::move(obj));
}

void ObjectTab::handle_right_click(Point pos) {
    room_->delete_obj(pos);
}

void CameraTab::handle_left_click(Point pos) {
    x1 = pos.x;
    y1 = pos.y;
}

void CameraTab::handle_right_click(Point pos) {
    x2 = pos.x;
    y2 = pos.y;
}

EditorTab::EditorTab(Room* room): room_ {room} {}
EditorTab::~EditorTab() {}

SaveLoadTab::SaveLoadTab(Room* room): EditorTab(room) {}

SaveLoadTab::~SaveLoadTab() {}

ObjectTab::ObjectTab(Room* room): EditorTab(room),
solid_obj {(int)ObjCode::NONE}, pb_sticky {(int)StickyLevel::None},
is_car {true}, sb_ends {2} {}

ObjectTab::~ObjectTab() {}

CameraTab::CameraTab(Room* room): EditorTab(room), camera_ {room->camera()},
x1 {0}, y1 {0}, x2 {0}, y2 {0},
radius {6.0}, priority {10} {}

CameraTab::~CameraTab() {}
