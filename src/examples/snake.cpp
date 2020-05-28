#include <iostream>
#include <random>
#include <algorithm>
#include "glm/glm.hpp"
#include "cellEngine.hpp"

#define WIDTH 1000
#define HEIGTH 1000
#define GRID_SIZE 100

class snakeGame : public cellEngine {
private:
    std::vector<glm::vec2> snake;
    glm::vec2 dir;
    glm::vec2 newDir;

    glm::vec2 food;

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution;

    glm::vec2 getRandomLocation() {
        return glm::vec2(distribution(generator), distribution(generator));
    }

    bool test(glm::vec2 pos) {
        for(auto i = snake.begin()+1; i != snake.end(); i++) if(pos == *i) return 1;
        return (pos.x < 0) || (pos.x > GRID_SIZE-1) || (pos.y < 0) || (pos.y > GRID_SIZE-1);
    }

public:
    snakeGame() : distribution(0, GRID_SIZE), snake(1, glm::vec2(GRID_SIZE/4, GRID_SIZE/2)), dir(1, 0), newDir(1, 0) { getRandomLocation(); food = getRandomLocation(); }

    virtual void update() {
        if     (Key(GLFW_KEY_W)) newDir = glm::vec2(0, -1);
        else if(Key(GLFW_KEY_S)) newDir = glm::vec2(0, 1);
        else if(Key(GLFW_KEY_A)) newDir = glm::vec2(-1, 0);
        else if(Key(GLFW_KEY_D)) newDir = glm::vec2(1, 0);
        if(newDir+dir != glm::vec2(0)) dir = newDir;

        for(auto i = snake.rbegin(); i != snake.rend()-1; i++)
            *i = *(i+1);

        snake[0] += dir;

        if(test(snake[0])) Window.close();

        Cells.Colors.fill(glm::u8vec3(0));

        for(auto pos : snake)
            Cells.Colors(pos.x, pos.y) = glm::u8vec3(255);

        if(snake[0] == food) {
            food = getRandomLocation();
            snake.resize(snake.size()+1);
        }

        Cells.Colors(food.x, food.y) = glm::u8vec3(255, 0, 0);
    }
};

int main() {

    snakeGame Snake;

    Snake.init(WIDTH, HEIGTH, "snake", GRID_SIZE);
    Snake.mainLoop();

    return 0;
}