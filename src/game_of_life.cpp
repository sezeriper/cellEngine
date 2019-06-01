#include <iostream>
#include <random>
#include <bitset>
#include "cellEngine.hpp"
#include "glm.hpp"

#define WIDTH 1000
#define HEIGTH 1000
#define GRID_SIZE 1000

class gameOfLife : public cellEngine {
private:
    inline int countNeighbors(int x, int y) {
        return  earth(x-1, y+1) +
                earth(x,   y+1) +
                earth(x+1, y+1) +
                earth(x-1, y)   +
                earth(x+1, y)   +
                earth(x-1, y-1) +
                earth(x,   y-1) +
                earth(x+1, y-1);
    }

    void randomize() {
        for(int i = 0; i < GRID_SIZE; i++) {
            for(int j = 0; j < GRID_SIZE; j++) {
                earth(i, j) = distribution(generator);
            }
        }
    }

    endlessGrid<bool> earth;
    endlessGrid<bool> nextEarth;

    std::default_random_engine generator;
    std::bernoulli_distribution distribution;

public:
    gameOfLife() : earth(GRID_SIZE), nextEarth(GRID_SIZE), generator(), distribution(0.1) {}

    virtual void update() {

        if(Key(GLFW_KEY_SPACE))
            randomize();

        for(int i = 0; i < GRID_SIZE; i++) {
            for(int j = 0; j < GRID_SIZE; j++) {
                Cells.Colors(i, j) = glm::u8vec3(earth(i, j) * 255);

                int neighbor = countNeighbors(i, j);

                if(neighbor < 2 || neighbor > 3)    nextEarth(i, j) = false;
                else if(neighbor == 3)  nextEarth(i, j) = true;
                else  nextEarth(i, j) = earth(i, j);
            }
        }

        std::swap(earth, nextEarth);
    }
};

int main() {
    gameOfLife simulation;

    simulation.init(WIDTH, HEIGTH, "game of life", GRID_SIZE);
    simulation.mainLoop();
    
    return 0;
}