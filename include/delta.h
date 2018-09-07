#ifndef DELTA_H
#define DELTA_H

#include "common.h"

class WorldMap;
class GameObject;
class Block;
class PushBlock;
struct Point;
enum class Layer;

class Delta {
public:
    virtual void revert(WorldMap*) = 0;
    virtual ~Delta() {};
};

class DeltaFrame {
public:
    DeltaFrame();
    void revert(WorldMap*);
    void push(std::unique_ptr<Delta>);
    bool trivial();

private:
    std::deque<std::unique_ptr<Delta>> deltas_;
};

class UndoStack {
public:
    UndoStack(unsigned int max_depth);
    void push(std::unique_ptr<DeltaFrame>);
    void pop(WorldMap*);

private:
    unsigned int max_depth_;
    unsigned int size_;
    std::deque<std::unique_ptr<DeltaFrame>> frames_;
};

class CreationDelta: public Delta {
public:
    CreationDelta(GameObject* object);
    void revert(WorldMap*);

private:
    GameObject* object_;
};

class DeletionDelta: public Delta {
public:
    DeletionDelta(std::unique_ptr<GameObject>);
    void revert(WorldMap*);

private:
    std::unique_ptr<GameObject> object_;
};

class MotionDelta: public Delta {
public:
    MotionDelta(Block* object, Point p);
    void revert(WorldMap*);

private:
    Block* object_;
    Point p_; // The previous position
};

class LinkUpdateDelta: public Delta {
public:
    LinkUpdateDelta(Block*, ObjSet);
    void revert(WorldMap*);

private:
    Block* object_;
    ObjSet links_;
};
#endif // DELTA_H
