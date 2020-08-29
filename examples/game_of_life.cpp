#include <stdlib.h>
#include "cellEngine.hpp"

#define WIDTH 500
#define HEIGTH 500
#define PIXEL_SIZE 1

int countNeighbors(int x, int y, const grid<bool>& grid) {
    return  grid.get(x-1, y+1) +
            grid.get(x,   y+1) +
            grid.get(x+1, y+1) +
            grid.get(x-1, y)   +
            grid.get(x+1, y)   +
            grid.get(x-1, y-1) +
            grid.get(x,   y-1) +
            grid.get(x+1, y-1);
}

void randomize(grid<bool>& grid) {
    for(int i = 0; i < WIDTH; i++) {
        for(int j = 0; j < HEIGTH; j++) {
            grid.set(i, j, rand() % 2);
        }
    }
}

int main() {
    cellEngine simulation(WIDTH, HEIGTH, PIXEL_SIZE, "lo");

    grid<bool> earth(WIDTH, HEIGTH);
    grid<bool> nextEarth(WIDTH, HEIGTH);

    simulation.update = [&simulation, &earth, &nextEarth](){

        if(simulation.window.getKey(GLFW_KEY_SPACE))
            randomize(earth);

        for(int i = 1; i < WIDTH - 1; i++) {
            for(int j = 1; j < HEIGTH - 1; j++) {
                simulation.cells.set(i, j, glm::u8vec3(earth.get(i, j) * 255));

                int neighbor = countNeighbors(i, j, earth);

                if(neighbor < 2 || neighbor > 3)    nextEarth.set(i, j, false);
                else if(neighbor == 3)  nextEarth.set(i, j, true);
                else  nextEarth.set(i, j, earth.get(i, j));
            }
        }

        std::swap(earth, nextEarth);
    };


    simulation.mainLoop();
    return 0;
}