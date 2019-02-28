#ifndef WALL_H
#define WALL_H

#include "pushblock.h"

class Wall: public PushBlock {
public:
    Wall();
    ~Wall();
    ObjCode obj_code();

    bool skip_serialization();
    void draw(GraphicsManager*);
};

#endif // WALL_H
