#ifndef PLAYER_H
#define PLAYER_H

#include "pushblock.h"
#include "car.h"

class Player: public PushBlock {
public:
    Player(Point3 pos, RidingState state);
    ~Player();
    ObjCode obj_code() const;
    void serialize(MapFileO& file);
    static GameObject* deserialize(MapFileI& file);
    void set_riding(RidingState);
    RidingState state();
    void toggle_riding(RoomMap* room_map, DeltaFrame*);
    Car* get_car(RoomMap* room_map, bool strict);

    virtual void collect_special_links(RoomMap*, Sticky sticky_level, std::vector<GameObject*>& links) const;

    void draw(GraphicsManager*);

private:
    RidingState state_;
};

#endif // PLAYER_H
