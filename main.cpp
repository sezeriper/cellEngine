#include <random>
#include "cellEngine.hpp"

#define WIDTH 100
#define HEIGTH 100
#define PIXEL_SIZE 5

int main() {
    cellEngine simulation(WIDTH, HEIGTH, PIXEL_SIZE, "lo");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> intDist(0, 255);

    simulation.update = [&simulation, &intDist, &gen](){

        for(int i = 0; i < WIDTH; i++) {
            for(int j = 0; j < HEIGTH; j++) {
                simulation.cells.set(i, j, glm::u8vec3(intDist(gen), intDist(gen), intDist(gen)));
           }
        }
    };


    simulation.mainLoop();
    return 0;
}