#include "effects.h"

#include <algorithm>

#include "graphicsmanager.h"
#include "gameobject.h"


Effects::Effects(): trails_ {} {}

Effects::~Effects() {}

const unsigned int FALL_TRAIL_OPACITY = 8;
const float MAX_OPACITY = 10.0;
const float MAX_WIDTH = 16.0;

void Effects::sort_by_distance(float angle) {}

void Effects::update() {
    for (auto& trail : trails_) {
        --trail.opacity;
    }
}

void Effects::draw(GraphicsManager* gfx) {
    for (auto& trail : trails_) {
        Color4 color = COLORS[trail.color];
        color.a = trail.opacity/MAX_OPACITY;
        gfx->set_color(color);
        Point3 base = trail.base;
        glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(base.x, base.z, base.y));
        model = glm::scale(model, glm::vec3(trail.opacity/MAX_WIDTH, trail.height, trail.opacity/MAX_WIDTH));
        model = glm::translate(model, glm::vec3(0,0.5,0));
        gfx->set_model(model);
        gfx->draw_cube();
    }
    trails_.erase(std::remove_if(trails_.begin(), trails_.end(), [](FallTrail t) {return t.opacity == 0;}), trails_.end());
}

void Effects::push_trail(GameObject* block, int height, int drop) {
    trails_.push_back({block->pos_ - Point3{0,0,drop}, height+drop, FALL_TRAIL_OPACITY, block->color_});
}
