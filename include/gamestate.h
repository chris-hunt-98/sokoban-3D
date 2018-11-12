#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "common.h"

class GraphicsManager;

class GameState {
public:
    GameState(GraphicsManager*);
    virtual ~GameState() = default;
    void create_child(std::unique_ptr<GameState> child);
    void defer_to_parent();
    void set_csp(std::unique_ptr<GameState>*);
    virtual void main_loop() = 0;
    void check_for_quit();

protected:
    GraphicsManager* gfx_;
    GLFWwindow* window_;

private:
    std::unique_ptr<GameState> parent_;
    std::unique_ptr<GameState>* current_state_ptr_;
    bool can_quit_;
};

class MainMenuState: public GameState {
public:
    MainMenuState(GraphicsManager*);
    void main_loop();
};

#endif // GAMESTATE_H
