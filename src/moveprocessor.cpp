#include "moveprocessor.h"

#include <iostream>

#include "common.h"
#include "gameobject.h"
#include "player.h"
#include "delta.h"
#include "roommap.h"
#include "door.h"

#include "playingstate.h"

#include "horizontalstepprocessor.h"
#include "fallstepprocessor.h"

MoveProcessor::MoveProcessor(PlayingState* playing_state, RoomMap* room_map, DeltaFrame* delta_frame):
fall_check_ {}, moving_blocks_ {},
playing_state_ {playing_state}, map_ {room_map}, delta_frame_ {delta_frame},
frames_ {0}, state_ {} {}

MoveProcessor::~MoveProcessor() {}

bool MoveProcessor::try_move(Player* player, Point3 dir) {
    if (player->state_ == RidingState::Bound) {
        move_bound(player, dir);
    } else {
        move_general(dir);
    }
    if (moving_blocks_.empty()) {
        return false;
    }
    state_ = MoveStep::Horizontal;
    frames_ = HORIZONTAL_MOVEMENT_FRAMES;
    return true;
}

void MoveProcessor::move_bound(Player* player, Point3 dir) {
    // When Player is Bound, no other agents move
    if (map_->view(player->shifted_pos(dir))) {
        return;
    }
    // If the player is bound, it's on top of a block!
    GameObject* car = map_->view(player->shifted_pos({0,0,-1}));
    GameObject* adj = map_->view(car->shifted_pos(dir));
    if (adj && car->color_ == adj->color_) {
        map_->take(player);
        player->set_linear_animation(dir);
        delta_frame_->push(std::make_unique<MotionDelta>(player, dir, map_));
        moving_blocks_.push_back(player);
        player->shift_pos_from_animation();
        map_->put(player);
    }
}

void MoveProcessor::move_general(Point3 dir) {
    HorizontalStepProcessor(map_, delta_frame_, dir, fall_check_, moving_blocks_).run();
}

bool MoveProcessor::update() {
    if (--frames_ == 0) {
        switch (state_) {
        case MoveStep::Horizontal:
        case MoveStep::Fall:
        case MoveStep::ColorChange:
            perform_switch_checks();
            try_fall_step();
            break;
        default:
            break;
        }
    }
    for (GameObject* block : moving_blocks_) {
        block->update_animation();
    }
    return frames_ == 0;
}

void MoveProcessor::abort() {
    for (GameObject* block : moving_blocks_) {
        block->reset_animation();
    }
}

void MoveProcessor::color_change(Player* player) {
    Car* car = player->get_car(map_, false);
    if (!(car && car->cycle_color(false))) {
        return;
    }
    state_ = MoveStep::ColorChange;
    // TODO: consider renaming
    frames_ = FALL_MOVEMENT_FRAMES;
    delta_frame_->push(std::make_unique<ColorChangeDelta>(car));
    fall_check_.push_back(car->parent_);
    for (Point3 d : DIRECTIONS) {
        if (GameObject* block = map_->view(car->shifted_pos(d))) {
            fall_check_.push_back(block);
        }
    }
}

void MoveProcessor::try_fall_step() {
    moving_blocks_.clear();
    if (!fall_check_.empty()) {
        if (FallStepProcessor(map_, delta_frame_, std::move(fall_check_)).run()) {
            state_ = MoveStep::Fall;
            frames_ = FALL_MOVEMENT_FRAMES;
        }
        fall_check_.clear();
    }
}

void MoveProcessor::try_door_move(Door* door) {
    if (!door->state()) {
        return;
    }
    std::vector<GameObject*> objs_to_move {};
    // TODO: make this more general later
    if (GameObject* above = map_->view(door->pos_above())) {
        if (Player* player = dynamic_cast<Player*>(above)) {
            objs_to_move.push_back(player);
        } else if (Car* car = dynamic_cast<Car*>(above->modifier())) {
            if (Player* player = dynamic_cast<Player*>(map_->view(car->pos_above()))) {
                if (player->state_ == RidingState::Riding) {
                    objs_to_move.push_back(player);
                    objs_to_move.push_back(above);
                }
            }
        }
    }
    if (objs_to_move.empty()) {
        return;
    }
    // IMPORTANT TODO: separate the acts of "entering" and "exiting" the door
    if (playing_state_->try_use_door(door, objs_to_move)) {
        // I'm actually not sure what should be done when a door successfully gets entered
    }
    return;
}

void MoveProcessor::perform_switch_checks() {
    map_->alert_activated_listeners(delta_frame_, this);
    map_->reset_local_state();
    map_->check_signalers(delta_frame_, this);
}

void MoveProcessor::add_to_fall_check(GameObject* obj) {
    fall_check_.push_back(obj);
}
