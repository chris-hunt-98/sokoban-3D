#ifndef DELTA_H
#define DELTA_H

#include <deque>

#include "common.h"

class Room;
class RoomMap;
class GameObject;
class PushBlock;
class SnakeBlock;
class Switchable;
class Switch;
class Signaler;
class Player;
class PlayingState;
class Car;

struct Point;
enum class Layer;


class Delta {
public:
    virtual ~Delta();
    virtual void revert() = 0;
};


class DeltaFrame {
public:
    DeltaFrame();
    virtual ~DeltaFrame();
    void revert();
    void push(std::unique_ptr<Delta>);
    bool trivial();

private:
    std::vector<std::unique_ptr<Delta>> deltas_;
};


class UndoStack {
public:
    UndoStack(unsigned int max_depth);
    virtual ~UndoStack();
    void push(std::unique_ptr<DeltaFrame>);
    bool non_empty();
    void pop();
    void reset();

private:
    std::deque<std::unique_ptr<DeltaFrame>> frames_;
    unsigned int max_depth_;
    unsigned int size_;
};


class CreationDelta: public Delta {
public:
    CreationDelta(GameObject* obj, RoomMap* room_map);
    ~CreationDelta();
    void revert();

private:
    GameObject* obj_;
    RoomMap* map_;
};


class DeletionDelta: public Delta {
public:
    DeletionDelta(GameObject* obj, RoomMap* room_map);
    ~DeletionDelta();
    void revert();

private:
    GameObject* obj_;
    RoomMap* map_;
};


class MotionDelta: public Delta {
public:
    MotionDelta(GameObject* obj, Point3 dpos, RoomMap* room_map);
    ~MotionDelta();
    void revert();

private:
    GameObject* obj_;
    Point3 dpos_;
    RoomMap* map_;
};


class BatchMotionDelta: public Delta {
public:
    BatchMotionDelta(std::vector<GameObject*> objs, Point3 dpos, RoomMap* room_map);
    ~BatchMotionDelta();
    void revert();

private:
    std::vector<GameObject*> objs_;
    Point3 dpos_;
    RoomMap* map_;
};


class AddLinkDelta: public Delta {
public:
    AddLinkDelta(SnakeBlock* a, SnakeBlock* b);
    ~AddLinkDelta();
    void revert();

private:
    SnakeBlock* a_;
    SnakeBlock* b_;
};


class RemoveLinkDelta: public Delta {
public:
    RemoveLinkDelta(SnakeBlock* a, SnakeBlock* b);
    ~RemoveLinkDelta();
    void revert();

private:
    SnakeBlock* a_;
    SnakeBlock* b_;
};


class DoorMoveDelta: public Delta {
public:
    DoorMoveDelta(PlayingState* state, Room* room, Point3 pos);
    ~DoorMoveDelta();
    void revert();

private:
    PlayingState* state_;
    Room* room_;
    Point3 pos_;
};


class SwitchableDelta: public Delta {
public:
    SwitchableDelta(Switchable* obj, bool active, bool waiting, RoomMap* room_map);
    ~SwitchableDelta();
    void revert();

private:
    Switchable* obj_;
    RoomMap* map_;
    bool active_;
    bool waiting_;
};


class SwitchToggleDelta: public Delta {
public:
    SwitchToggleDelta(Switch* obj);
    ~SwitchToggleDelta();
    void revert();

private:
    Switch* obj_;
};

class SignalerToggleDelta: public Delta {
public:
    SignalerToggleDelta(Signaler*);
    ~SignalerToggleDelta();
    void revert();

private:
    Signaler* obj_;
};


class RidingStateDelta: public Delta {
public:
    RidingStateDelta(Player* player, RidingState state);
    ~RidingStateDelta();
    void revert();

private:
    Player* player_;
    RidingState state_;
};


class ColorChangeDelta: public Delta {
public:
    ColorChangeDelta(Car* car);
    ~ColorChangeDelta();
    void revert();

private:
    Car* car_;
};

#endif // DELTA_H
