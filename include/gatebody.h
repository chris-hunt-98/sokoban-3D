#ifndef GATEBODY_H
#define GATEBODY_H

#include "pushblock.h"

class Gate;

// The part of a Gate that comes up above the ground
// It inherits the color, pushability, and gravitability of its corresponding Gate object
class GateBody: public PushBlock {
public:
    GateBody(Gate* parent);
    ~GateBody();

    bool skip_serialization();
    ObjCode obj_code();

    void draw(GraphicsManager*);

    Gate* parent_;
};

#endif // GATEBODY_H
