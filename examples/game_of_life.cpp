#include <random>
#include "cellEngine.hpp"

#define WIDTH 200
#define HEIGTH 200
#define PIXEL_SIZE 4

class gameOfLife : public cellEngine {
private:
    int countNeighbors(int x, int y) {
        return  earth.get(x-1, y+1) +
                earth.get(x,   y+1) +
                earth.get(x+1, y+1) +
                earth.get(x-1, y)   +
                earth.get(x+1, y)   +
                earth.get(x-1, y-1) +
                earth.get(x,   y-1) +
                earth.get(x+1, y-1);
    }

    void randomize() {

        for(int i = 0; i < WIDTH; i++) {
            for(int j = 0; j < HEIGTH; j++) {
                earth.set(i, j, distribution(generator));
            }
        }
    }

    grid<bool> earth;
    grid<bool> nextEarth;

    std::default_random_engine generator;
    std::bernoulli_distribution distribution;

public:
    void start() {
        earth = grid<bool>(WIDTH, HEIGTH);
        nextEarth = grid<bool>(WIDTH, HEIGTH);
    }

    void update() {

        if(window->getKey(GLFW_KEY_SPACE))
            randomize();

        for(int i = 0; i < WIDTH; i++) {
            for(int j = 0; j < HEIGTH; j++) {
                cells->set(i, j, glm::u8vec3(earth.get(i, j) * 255));

                int neighbor = countNeighbors(i, j);

                if(neighbor < 2 || neighbor > 3)    nextEarth.set(i, j, false);
                else if(neighbor == 3)  nextEarth.set(i, j, true);
                else  nextEarth.set(i, j, earth.get(i, j));
            }
        }

        std::swap(earth, nextEarth);
    }
};

int main() {
    gameOfLife simulation;

    simulation.setup(WIDTH, HEIGTH, PIXEL_SIZE, "lo");
    simulation.mainLoop();
    return 0;
}