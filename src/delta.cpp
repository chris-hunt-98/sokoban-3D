#include "delta.h"
#include "gameobject.h"
#include "worldmap.h"
#include "block.h"

#include <iostream>

UndoStack::UndoStack(unsigned int max_depth): max_depth_ {max_depth}, size_ {0}, frames_ {} {}

void UndoStack::push(std::unique_ptr<DeltaFrame> delta_frame) {
    if (!(*delta_frame).trivial()) {
        if (size_ == max_depth_) {
            frames_.pop_front();
        } else {
            ++size_;
        }
        frames_.push_back(std::move(delta_frame));
    }
}

void UndoStack::pop(WorldMap* world_map) {
    if (size_ > 0) {
        (*frames_.back()).revert(world_map);
        frames_.pop_back();
        --size_;
    }
}

DeltaFrame::DeltaFrame(): deltas_ {} {}

void DeltaFrame::revert(WorldMap* world_map) {
    for (auto it = deltas_.rbegin(); it != deltas_.rend(); ++it) {
        (**it).revert(world_map);
    }
}

void DeltaFrame::push(std::unique_ptr<Delta> delta) {
    deltas_.push_back(std::move(delta));
}

bool DeltaFrame::trivial() {
    return deltas_.empty();
}

DeletionDelta::DeletionDelta(std::unique_ptr<GameObject> object): object_ {std::move(object)} {}

void DeletionDelta::revert(WorldMap* world_map) {
    GameObject* obj = object_.get();
    world_map->put_quiet(std::move(object_));
    obj->reinit();
}

CreationDelta::CreationDelta(GameObject* object): object_ {object} {}

void CreationDelta::revert(WorldMap* world_map) {
    world_map->take_quiet_id(object_->pos(), object_->layer(), object_);
}

MotionDelta::MotionDelta(Block* object, Point p): object_ {object}, p_ {p} {}

void MotionDelta::revert(WorldMap* world_map) {
    auto object_unique = world_map->take_quiet_id(object_->pos(), object_->layer(), object_);
    object_->set_pos(p_, nullptr);
    world_map->put_quiet(std::move(object_unique));
}

AddLinkDelta::AddLinkDelta(Block* a, Block* b): a_ {a}, b_ {b} {}

void AddLinkDelta::revert(WorldMap* world_map) {
    a_->remove_link(b_, nullptr);
}

RemoveLinkDelta::RemoveLinkDelta(Block* a, Block* b): a_ {a}, b_ {b} {}

void RemoveLinkDelta::revert(WorldMap* world_map) {
    a_->add_link(b_, nullptr);
}