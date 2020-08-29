#include <iostream>
#include <random>
#include <algorithm>
#include "glm/glm.hpp"
#include "cellEngine.hpp"

#define WIDTH 100
#define HEIGTH 100
#define PIXEL_SIZE 10

int main() {

    std::vector<glm::vec2> snake(1, glm::vec2(WIDTH/2, HEIGTH/2));
    glm::vec2 dir(1, 0);
    glm::vec2 newDir(1, 0);

    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_int_distribution<int> distribution(0, WIDTH-1);

    auto getRandomLocation = [&distribution, &generator] () {
        return glm::vec2(distribution(generator), distribution(generator));
    };

    glm::vec2 food = getRandomLocation();

    auto test = [&] (glm::vec2 pos) {
        for(auto i = snake.begin()+1; i != snake.end(); i++) if(pos == *i) return true;
        return (pos.x < 0) || (pos.x > WIDTH-1) || (pos.y < 0) || (pos.y > WIDTH-1);
    };

    cellEngine simulation(WIDTH, HEIGTH, PIXEL_SIZE, "lo");

    simulation.update = [&] () {
        if     (simulation.window.getKey(GLFW_KEY_W)) newDir = glm::vec2(0, -1);
        else if(simulation.window.getKey(GLFW_KEY_S)) newDir = glm::vec2(0, 1);
        else if(simulation.window.getKey(GLFW_KEY_A)) newDir = glm::vec2(-1, 0);
        else if(simulation.window.getKey(GLFW_KEY_D)) newDir = glm::vec2(1, 0);
        if(newDir+dir != glm::vec2(0)) dir = newDir;

        for(auto i = snake.rbegin(); i != snake.rend()-1; i++)
            *i = *(i+1);

        snake[0] += dir;

        if(test(snake[0])) simulation.window.close();

        simulation.cells.fill(glm::u8vec3(0));

        for(auto pos : snake)
            simulation.cells.set(pos.x, pos.y, glm::u8vec3(255));

        if(snake[0] == food) {
            food = getRandomLocation();
            snake.resize(snake.size()+1);
        }

        simulation.cells.set(food.x, food.y, glm::u8vec3(255, 0, 0));
    };

    simulation.mainLoop();

    return 0;
}