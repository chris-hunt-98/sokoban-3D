#ifndef CAR_H
#define CAR_H

#include "common.h"
#include "objectmodifier.h"
#include "colorcycle.h"
#include "mapfile.h"

class Car: public ObjectModifier {
public:
    Car(GameObject* parent, ColorCycle color_cycle);
    virtual ~Car();

    ModCode mod_code();
    void serialize(MapFileO& file);
    static void deserialize(MapFileI&, RoomMap*, GameObject*);

    void collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>&);

    bool cycle_color(bool undo);

    std::unique_ptr<ObjectModifier> duplicate(GameObject*);

    ColorCycle color_cycle_;
};

#endif // CAR_H
