#ifndef BLOCK_H
#define BLOCK_H

#include "common.h"
#include "gameobject.h"

class GameObject;
class MoveProcessor;

/** Abstract class for Solid objects which can move around and do things
 * Everything that's not a Wall is a Block
 */
class Block: public GameObject {
public:
    static BlockSet EMPTY_BLOCK_SET;

    Block(int x, int y);
    Block(int x, int y, unsigned char color, bool is_car);
    virtual ~Block();
    Layer layer();
    bool is_car();
    void set_car(bool is_car);
    void draw(GraphicsManager*);
    unsigned char color();
    virtual bool push_recheck(MoveProcessor*) = 0;
    virtual const BlockSet& get_strong_links() = 0;
    virtual const BlockSet& get_weak_links() = 0;
    void add_link(Block*, DeltaFrame*);
    void remove_link(Block*, DeltaFrame*);
    void remove_all_links(DeltaFrame*);
    void cleanup(DeltaFrame*);
    void reinit();
    virtual void check_remove_local_links(RoomMap*, DeltaFrame*);
    virtual void check_add_local_links(RoomMap*, DeltaFrame*) = 0;
    virtual void post_move_reset();

protected:
    unsigned char color_;
    bool car_;
    BlockSet links_;
};

enum class StickyLevel {
    None,
    Weak,
    Strong,
};

/** The standard type of block, represented by a grid aligned square
 */
class PushBlock: public Block {
public:
    PushBlock(int x, int y);
    PushBlock(int x, int y, unsigned char color, bool is_car, StickyLevel sticky);
    virtual ~PushBlock();
    virtual ObjCode obj_code();
    virtual void serialize(std::ofstream& file);
    static GameObject* deserialize(unsigned char* buffer);
    bool relation_check();
    void relation_serialize(std::ofstream& file);

    bool push_recheck(MoveProcessor*);
    void set_sticky(StickyLevel sticky);
    void draw(GraphicsManager*);
    StickyLevel sticky();
    const BlockSet& get_strong_links();
    const BlockSet& get_weak_links();
    void check_add_local_links(RoomMap*, DeltaFrame*);

private:
    StickyLevel sticky_;
};

/** A different type of block which forms "chains", represented by a diamond
 */
class SnakeBlock: public Block {
public:
    SnakeBlock(int x, int y);
    SnakeBlock(int x, int y, unsigned char color, bool is_car, unsigned int ends);
    virtual ~SnakeBlock();
    virtual ObjCode obj_code();
    void serialize(std::ofstream& file);
    static GameObject* deserialize(unsigned char* buffer);
    bool relation_check();
    void relation_serialize(std::ofstream& file);

    bool push_recheck(MoveProcessor*);
    unsigned int ends();
    int distance();
    void reset_target();
    const BlockSet& get_strong_links();
    const BlockSet& get_weak_links();
    void draw(GraphicsManager*);
    bool available();
    bool confused(RoomMap*);
    void check_add_local_links(RoomMap*, DeltaFrame*);
    void collect_unlinked_neighbors(RoomMap*, std::unordered_set<SnakeBlock*>&);
    void post_move_reset();

private:
    unsigned int ends_;
    int distance_;
    SnakeBlock* target_;

    friend class SnakePuller;
};

class SnakePuller {
public:
    SnakePuller(RoomMap*, DeltaFrame*, std::unordered_set<SnakeBlock*>&, PointSet&, Point);
    ~SnakePuller();
    void prepare_pull(SnakeBlock*);
    void pull(SnakeBlock*);

private:
    RoomMap* room_map_;
    DeltaFrame* delta_frame_;
    std::unordered_set<SnakeBlock*>& check_;
    PointSet& floor_check_;
    Point dir_;
};

#endif // BLOCK_H
