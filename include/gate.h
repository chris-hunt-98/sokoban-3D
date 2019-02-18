#ifndef GATE_H
#define GATE_H

#include <memory>

#include "objectmodifier.h"
#include "switchable.h"
#include "common.h"

class GateBody;
class GraphicsManager;
class MapFileO;

class Gate: public ObjectModifier, public Switchable {
public:
    Gate(GameObject* parent, GateBody* body, bool def);
    virtual ~Gate();
    ObjCode obj_code();
    void serialize(MapFileO& file);
    //static GameObject* deserialize(MapFileI& file);
    bool can_set_state(bool state, RoomMap*);
    void apply_state_change(RoomMap*, std::vector<GameObject*>&);

    void setup_on_put(RoomMap*);
    void cleanup_on_take(RoomMap*);

    void draw(GraphicsManager*);

    void check_above_vacant(RoomMap*, DeltaFrame*);

private:
    GateBody* body_;
};

#endif // GATE_H
