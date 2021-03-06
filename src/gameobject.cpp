#include "gameobject.h"

#include "common_constants.h"

#include "roommap.h"
#include "graphicsmanager.h"
#include "delta.h"
#include "mapfile.h"
#include "objectmodifier.h"
#include "animation.h"

#include "component.h"


Sticky operator &(Sticky a, Sticky b) {
    return static_cast<Sticky>(static_cast<unsigned char>(a) &
                               static_cast<unsigned char>(b));
}


// id_ begins in an "inconsistent" state - it *must* be set by the GameObjectArray
GameObject::GameObject(Point3 pos, int color, bool pushable, bool gravitable):
    modifier_ {}, animation_ {}, comp_ {},
    pos_ {pos}, id_ {-1},
    color_ {color}, pushable_ {pushable}, gravitable_ {gravitable},
    tangible_ {false} {}

GameObject::~GameObject() {}

// Copy Constructor creates trivial unique_ptr members
GameObject::GameObject(const GameObject& obj):
    modifier_ {}, animation_ {}, comp_ {},
    pos_ {obj.pos_}, id_ {-1},
    color_ {obj.color_}, pushable_ {obj.pushable_}, gravitable_ {obj.gravitable_} {}

std::string GameObject::to_str() {
    std::string mod_str {""};
    if (modifier_) {
        mod_str = "-" + modifier_->name();
    }
    char buf[64] = "";
    // TODO: Make this less ugly
    // This is only used to make the editor more user friendly, so it's not a huge deal.
    sprintf(buf, "%s:%s:%s%s", pushable_ ? "P" : "NP", gravitable_ ? "G" : "NG", name().c_str(), mod_str.c_str());
    return std::string{buf};
}

bool GameObject::relation_check() {
    return false;
}

bool GameObject::skip_serialization() {
    return false;
}

void GameObject::relation_serialize(MapFileO& file) {}

bool GameObject::is_agent() {
    return (modifier_ && modifier()->is_agent());
}

Point3 GameObject::shifted_pos(Point3 d) {
    return pos_ + d;
}

void GameObject::shift_internal_pos(Point3 d) {
    pos_ += d;
    if (modifier_) {
        modifier_->shift_internal_pos(d);
    }
}

void GameObject::setup_on_put(RoomMap* room_map) {
    if (modifier_) {
        modifier_->setup_on_put(room_map);
    }
}

void GameObject::cleanup_on_take(RoomMap* room_map) {
    if (modifier_) {
        modifier_->cleanup_on_take(room_map);
    }
}

void GameObject::cleanup_on_destruction(RoomMap* room_map) {
    if (modifier_) {
        modifier_->cleanup_on_destruction(room_map);
    }
}

void GameObject::setup_on_undestruction(RoomMap* room_map) {
    if (modifier_) {
        modifier_->setup_on_undestruction(room_map);
    }
}

void GameObject::set_modifier(std::unique_ptr<ObjectModifier> mod) {
    modifier_ = std::move(mod);
}

ObjectModifier* GameObject::modifier() {
    return modifier_.get();
}

// NOTE: these can be static_casts as long as the code using them is careful
PushComponent* GameObject::push_comp() {
    return dynamic_cast<PushComponent*>(comp_);
}

FallComponent* GameObject::fall_comp() {
    return dynamic_cast<FallComponent*>(comp_);
}


void GameObject::collect_sticky_component(RoomMap* room_map, Sticky sticky_level, Component* comp) {
    std::vector<GameObject*> to_check {this};
    while (!to_check.empty()) {
        GameObject* cur = to_check.back();
        to_check.pop_back();
        if (cur->comp_) {
            continue;
        }
        cur->comp_ = comp;
        comp->blocks_.push_back(cur);
        cur->collect_sticky_links(room_map, sticky_level, to_check);
        cur->collect_special_links(room_map, sticky_level, to_check);
        if (ObjectModifier* mod = cur->modifier()) {
            mod->collect_sticky_links(room_map, sticky_level, to_check);
        }
    }
}

bool GameObject::has_sticky_neighbor(RoomMap* room_map) {
    for (Point3 d : H_DIRECTIONS) {
        if (GameObject* adj = room_map->view(pos_ + d)) {
            if ((adj->color_ == color_) && static_cast<bool>(adj->sticky() & sticky())) {
                return true;
            }
        }
    }
    return false;
}

void GameObject::collect_special_links(RoomMap*, Sticky, std::vector<GameObject*>&) {}

void GameObject::reset_animation() {
    animation_.reset(nullptr);
}

void GameObject::set_linear_animation(Point3 d) {
    animation_ = std::make_unique<LinearAnimation>(d);
}

bool GameObject::update_animation() {
    if (animation_ && animation_->update()) {
        animation_.reset(nullptr);
        return true;
    }
    return false;
}

void GameObject::shift_pos_from_animation() {
    pos_ = animation_->shift_pos(pos_);
}

#include <iostream>

void GameObject::abstract_shift(Point3 dpos, DeltaFrame* delta_frame) {
    if (!(dpos == Point3{})) {
        pos_ += dpos;
        delta_frame->push(std::make_unique<AbstractMotionDelta>(this, dpos));
    }
}

FPoint3 GameObject::real_pos() {
    if (animation_) {
        return pos_ + animation_->dpos();
    } else {
        return pos_;
    }
}

void GameObject::draw_force_indicators(GraphicsManager* gfx, glm::mat4& model) {
    gfx->set_tex(Texture::Blank);
    if (!pushable_) {
        gfx->set_color(COLORS[BLACK]);
        auto new_model = glm::translate(model, glm::vec3(0.0, -0.2, 0.0));
        gfx->set_model(glm::scale(new_model, glm::vec3(1.1, .1, 1.1)));
        gfx->draw_cube();
    }
    if (!gravitable_) {
        gfx->set_color(COLORS[WHITE]);
        auto new_model = glm::translate(model, glm::vec3(0.0, 0.2, 0.0));
        gfx->set_model(glm::scale(new_model, glm::vec3(1.1, .1, 1.1)));
        gfx->draw_cube();
    }
}
