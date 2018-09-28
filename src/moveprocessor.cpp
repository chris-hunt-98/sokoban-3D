#include "moveprocessor.h"
#include "block.h"
#include "delta.h"
#include "roommap.h"

MoveProcessor::MoveProcessor(Player* player, RoomMap* room_map, Point dir):
player_ {player}, map_ {room_map}, dir_ {dir}, comps_ {},
maybe_broken_weak_ {}, touched_snakes_ {} {}

void MoveProcessor::try_move(DeltaFrame* delta_frame) {
    // Any Player-layer object will stop the player from moving
    if (map_->view(player_->shifted_pos(dir_), Layer::Player)) {
        return;
    }
    switch (player_->state()) {
    case RidingState::Free :
        player_->shift_pos_auto(dir_, map_, delta_frame);
        break;
    case RidingState::Bound :
        move_bound(delta_frame);
        break;
    case RidingState::Riding :
        move_riding(delta_frame);
        break;
    default:
        break;
    }
}

void MoveProcessor::move_bound(DeltaFrame* delta_frame) {
    // We know that we're standing on a block already!
    Block* cur = static_cast<Block*>(map_->view(player_->pos(), Layer::Solid));
    Block* adj = dynamic_cast<Block*>(map_->view(player_->shifted_pos(dir_), Layer::Solid));
    if (adj && cur->color() == adj->color()) {
        player_->shift_pos_auto(dir_, map_, delta_frame);
    }
}

void MoveProcessor::move_riding(DeltaFrame* delta_frame) {
    std::vector<Component*> roots {};
    // NOTE: for now we'll effectively assume that car is a PushBlock, not a SnakeBlock
    Block* car = static_cast<Block*>(map_->view(player_->pos(), Layer::Solid));
    roots.push_back(move_component(car, false));
    // When the player can have "clones", we'll have to add more roots
    // They may also have DIFFERENT (rotated/reflected) directions!!
    /*
    for (Block* block : map_->movers()) {
        roots.push_back(move_component(block, false));
    }
    */
    for (Component* comp : roots) {
        comp->resolve_contingent();
    }
    if (comps_[car]->good()) {
        player_->shift_pos_auto(dir_, map_, delta_frame);
    }
    // Reset targets of unmoved snakes, look for snakes to check for link adding later
    std::unordered_set<SnakeBlock*> check_snakes = {};
    std::vector<SnakeBlock*> good_snakes = {};
    for (auto& sb : touched_snakes_) {
        if (comps_.count(sb) && comps_[sb]->good()) {
            good_snakes.push_back(sb);
            if (sb->available()) {
                sb->collect_unlinked_neighbors(map_, check_snakes);
            }
        } else {
            sb->reset_target();
        }
    }
    // Standard block movement
    for (auto& p : comps_) {
        if (p.second->good()) {
            p.first->shift_pos_auto(dir_, map_, delta_frame);
        }
    }
    // Remove any links that were broken
    for (auto& block : maybe_broken_weak_) {
        block->check_remove_local_links(map_, delta_frame);
    }
    // Now that links are broken, we can pull snakes
    SnakePuller puller(map_, delta_frame, check_snakes, dir_);
    for (auto& sb : good_snakes) {
        if (sb->distance() == 1) {
            puller.prepare_pull(sb);
        }
    }
    // Now that everything has moved, check for created links
    for (auto& p : comps_) {
        if (p.second->good()) {
            p.first->check_add_local_links(map_, delta_frame);
        }
    }
    // It's possible that some snakes we never "directly interacted with" also can add links!
    for (auto& sb : check_snakes) {
        sb->check_add_local_links(map_, delta_frame);
    }
    for (auto& sb : touched_snakes_) {
        sb->check_add_local_links(map_, delta_frame);
        sb->reset_target();
    }
}

/** Attempt to move the strong component containing a block
 * In certain circumstances, we want to check a component even if
 * we have already seen it before; this is represented by recheck.
 * The return value is the component, whose state indicates success/failure.
 */
Component* MoveProcessor::move_component(Block* block, bool recheck) {
    if (!comps_.count(block)) {
        find_strong_component(block);
    }
    Component* comp = comps_[block].get();
    if (comp->check() || recheck) {
        comp->set_check(false);
        for (Block* cur : comp->blocks()) {
            try_push(comp, cur->shifted_pos(dir_));
            if (comp->bad()) {
                break;
            }
            for(auto& link : cur->get_weak_links()) {
                //NOTE: should it be necessary, false can be replaced with a new
                // method called Block::weak_recheck()
                auto moved_comp = move_component(link, false);
                comp->add_weak(moved_comp);
                // It is possible that we'll arrive at a branch "backwards", so
                // it's better to have the connection be symmetric.
                moved_comp->add_weak(comp);
                if (moved_comp->bad()) {
                    maybe_broken_weak_.insert(link);
                }
            }
        }
    }
    return comp;
}

void MoveProcessor::try_push(Component* comp, Point pos) {
    if (!map_->valid(pos)) {
        comp->set_bad();
    } else {
        GameObject* obj = map_->view(pos, Layer::Solid);
        if (obj) {
            Block* block = dynamic_cast<Block*>(obj);
            if (block) {
                auto pushed_comp = move_component(block, block->push_recheck(this));
                if (pushed_comp->bad()) {
                    comp->set_bad();
                } else {
                    comp->add_push(pushed_comp);
                }
            } else {
                comp->set_bad();
            }
        }
    }
}

void MoveProcessor::find_strong_component(Block* block) {
    std::vector<Block*> to_check = {block};
    auto comp = std::make_shared<Component>();
    while (!to_check.empty()) {
        Block* cur = to_check.back();
        to_check.pop_back();
        comp->add_block(cur);
        comps_[cur] = comp;
        for (auto& link : cur->get_strong_links()) {
            if (!comps_.count(link)) {
                to_check.push_back(link);
            }
        }
    }
}

void MoveProcessor::insert_touched_snake(SnakeBlock* sb) {
    touched_snakes_.insert(sb);
}

Component::Component(): state_ {ComponentState::Contingent}, check_ {true}, blocks_ {}, push_ {}, weak_ {} {}

void Component::resolve_contingent() {
    if (state_ == ComponentState::Contingent) {
        state_ = ComponentState::Good;
        for (auto& comp : push_) {
            comp->resolve_contingent();
        }
        for (auto& comp : weak_) {
            comp->resolve_contingent();
        }
    }
}

bool Component::bad() {
    return state_ == ComponentState::Bad;
}

bool Component::good() {
    return state_ == ComponentState::Good;
}

void Component::set_bad() {
    state_ = ComponentState::Bad;
}

void Component::set_check(bool check) {
    check_ = check;
}

const std::vector<Block*>& Component::blocks() {
    return blocks_;
}

bool Component::check() {
    return check_;
}

void Component::add_block(Block* block) {
    blocks_.push_back(block);
}

void Component::add_push(Component* comp) {
    push_.push_back(comp);
}

void Component::add_weak(Component* comp) {
    weak_.push_back(comp);
}
