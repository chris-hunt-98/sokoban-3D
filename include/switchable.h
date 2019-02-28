#ifndef SWITCHABLE_H
#define SWITCHABLE_H

#include <vector>

#include "objectmodifier.h"

class RoomMap;
class DeltaFrame;
class GameObject;
class MoveProcessor;
class Signaler;

class Switchable: public ObjectModifier {
public:
    Switchable(GameObject* parent, bool default_state, bool initial_state);
    virtual ~Switchable();

    void push_signaler(Signaler*);
    void connect_to_signalers();
    void set_aw(bool active, bool waiting, RoomMap*);
    bool state();
    virtual bool can_set_state(bool state, RoomMap*) = 0;
    void receive_signal(bool signal, RoomMap*, DeltaFrame*, MoveProcessor*);
    virtual void apply_state_change(RoomMap*, MoveProcessor*);
    void check_waiting(RoomMap*, DeltaFrame*, MoveProcessor*);

    virtual void cleanup_on_destruction(RoomMap* room_map);
    virtual void setup_on_undestruction(RoomMap* room_map);

protected:
    bool default_;
    bool active_; // Opposite of default behavior
    bool waiting_; // Toggle active as soon as possible

    std::vector<Signaler*> signalers_;

    friend class ModifierTab;
};


#endif // SWITCHABLE_H
