#include <stdlib.h>
#include "cellEngine.hpp"

#define WIDTH 200
#define HEIGTH 200
#define PIXEL_SIZE 4


int main() {
    cellEngine simulation(WIDTH, HEIGTH, PIXEL_SIZE, "test");

    simulation.update = [&simulation](){
        for(int i = 0; i < WIDTH; i++) {
            for(int j = 0; j < HEIGTH; j++) {
                simulation.cells.set(i, j, glm::u8vec3(rand() % 255, rand() % 255, rand() % 255));
            }
        }
    };


    simulation.mainLoop();
    return 0;
}