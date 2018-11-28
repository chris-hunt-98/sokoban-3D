#include "moveprocessor.h"
#include "block.h"
#include "player.h"
#include "delta.h"
#include "roommap.h"
#include "switch.h"
#include "component.h"

MoveProcessor::MoveProcessor(Player* player, RoomMap* room_map, Point3 dir, DeltaFrame* delta_frame):
player_ {player}, map_ {room_map}, delta_frame_ {delta_frame}, dir_ {dir},
move_comps_ {}, fall_check_ {}, fall_comps_ {} {}

MoveProcessor::~MoveProcessor() {}

void MoveProcessor::try_move() {
    if (player_->state() == RidingState::Bound) {
        move_bound();
    } else {
        move_general();
    }
}

void MoveProcessor::move_bound() {
    // This is more complicated in 3D...
    // For now, don't let bound player push anything
    if (map_->view(player_->shifted_pos(dir_))) {
        return;
    }
    // If the player is bound, it's on top of a block!
    Block* car = static_cast<Block*>(map_->view(player_->shifted_pos({0,0,-1})));
    Block* adj = dynamic_cast<Block*>(map_->view(car->shifted_pos(dir_)));
    if (adj && car->color() == adj->color()) {
        auto player_unique = map_->take_quiet(player_);
        delta_frame_->push(std::make_unique<MotionDelta>(std::vector<Block*> {player_}, dir_, map_));
        player_->shift_pos(dir_);
        map_->put_quiet(std::move(player_unique));
    }
}

void MoveProcessor::move_general() {
    init_movement_components();
    move_components();
    // Update snake links, do switch/other checks
    // Wait until the animation finishes
    try_fall();
}

void MoveProcessor::init_movement_components() {
    std::vector<StrongComponent*> roots {};
    std::vector<std::pair<StrongComponent*, StrongComponent*>> dependent_pairs {};
    make_root(player_, roots);
    if (player_->state() == RidingState::Riding) {
        Block* car = player_->get_car(map_, true);
        make_root(car, roots);
        dependent_pairs.push_back(std::make_pair(player_->s_comp(), car->s_comp()));
    }
    // When relevant, create other root components
    for (StrongComponent* comp : roots) {
        try_move_component(comp);
    }
    for (auto& p : dependent_pairs) {
        if (p.first->bad() || p.second->bad()) {
            p.first->set_bad();
            p.second->set_bad();
        }
    }
    for (StrongComponent* comp : roots) {
        comp->resolve_contingent();
    }
}

void MoveProcessor::make_root(Block* obj, std::vector<StrongComponent*>& roots) {
    auto component = obj->make_strong_component(map_);
    roots.push_back(component.get());
    move_comps_.push_back(std::move(component));
}

void MoveProcessor::move_components() {
    std::vector<Block*> to_move {};
    for (auto& comp : move_comps_) {
        comp->collect_good(to_move);
    }
    std::vector<std::unique_ptr<GameObject>> move_unique {};
    for (auto block : to_move) {
        move_unique.push_back(map_->take_quiet(block));
        fall_check_.push_back(block);
        Block* above = dynamic_cast<Block*>(map_->view(block->shifted_pos({0,0,1})));
        if (above && !above->s_comp()) {
            fall_check_.push_back(above);
        }
        block->shift_pos(dir_);
    }
    for (auto& comp : move_comps_) {
        comp->reset_blocks_comps();
    }
    move_comps_.clear();
    for (auto& obj : move_unique) {
        map_->put_quiet(std::move(obj));
    }
    if (!to_move.empty()) {
        delta_frame_->push(std::make_unique<MotionDelta>(std::move(to_move), dir_, map_));
    }
}

bool MoveProcessor::try_move_component(StrongComponent* comp) {
    for (Point3 pos : comp->to_push(dir_)) {
        if (!try_push(comp, pos)) {
            comp->set_bad();
            return false;
        }
    }
    for(Block* link : comp->get_weak_links(map_)) {
        if (!link->s_comp()) {
            auto link_comp = link->make_strong_component(map_);
            try_move_component(link_comp.get());
            move_comps_.push_back(std::move(link_comp));
        }
        if (!link->s_comp()->bad()) {
            comp->add_weak(link->s_comp());
        }
    }
    return true;
}

bool MoveProcessor::try_push(StrongComponent* comp, Point3 pos) {
    if (!map_->valid(pos)) {
        return false;
    }
    GameObject* obj = map_->view(pos);
    if (!obj) {
        return true;
    }
    Block* block = dynamic_cast<Block*>(obj);
    if (!block) {
        return false;
    }
    if (block->s_comp()) {
        return !block->s_comp()->bad();
    }
    auto unique_comp = block->make_strong_component(map_);
    StrongComponent* pushed_comp = unique_comp.get();
    move_comps_.push_back(std::move(unique_comp));
    comp->add_push(pushed_comp);
    return try_move_component(pushed_comp);
}

void MoveProcessor::try_fall() {
    while (!fall_check_.empty()) {
        std::cout << "Fall check wasn't empty" << std::endl;
        std::vector<Block*> next_check {};
        for (Block* block : fall_check_) {
            if (!block->w_comp()) {
                std::cout << "Made a component for block at " << block->pos() << std::endl;
                fall_comps_.push_back(block->make_weak_component(map_));
                block->w_comp()->collect_above(next_check, map_);
            }
        }
        fall_check_ = std::move(next_check);
    }
    std::cout << "Made Weak Components" << std::endl;
    // Initial check for land
    check_land_first();
    std::vector<std::pair<std::unique_ptr<GameObject>, int>> falling_blocks {};
    for (auto& comp : fall_comps_) {
        comp->collect_falling_from_map(falling_blocks, map_);
        comp->reset_blocks_comps();
    }
    for (auto& p : falling_blocks) {
        std::cout << p.first->pos() << p.second << std::endl;
    }
    while (true) {
        bool done_falling = true;
        for (auto& comp : fall_comps_) {
            if (comp->drop_check()) {
                done_falling = false;
            }
        }
        if (done_falling) {
            break;
        }
        for (auto& comp : fall_comps_) {
            if (comp->falling()) {
                comp->check_land_sticky(map_);
            }
        }
    }
    fall_comps_.clear();
    std::vector<std::pair<Block*, int>> fall_delta_entries {};
    for (auto& p : std::move(falling_blocks)) {
        Block* block = static_cast<Block*>(p.first.get());
        if (block->z() >= 0) {
            map_->put_quiet(std::move(p.first));
            fall_delta_entries.push_back(std::make_pair(block, p.second));
        } else {
            block->set_z(p.second);
            delta_frame_->push(std::make_unique<DeletionDelta>(std::move(p.first), map_));
        }
    }
    delta_frame_->push(std::make_unique<FallDelta>(std::move(fall_delta_entries), map_));
}

void MoveProcessor::check_land_first() {
    for (auto& comp : fall_comps_) {
        comp->check_land_first(map_);
    }
    for (auto& comp : fall_comps_) {
        comp->reset_blocks_comps();
    }
}
