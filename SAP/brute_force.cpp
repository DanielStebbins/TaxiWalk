#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>

int isTaxiPolygon(uint64_t steps, int N) {
    if((steps & 1) == 0) {
        return 0;
    }

    // No two turns.  
    // Connecting the end of the "loop" to the beginning and checking the two-turn rule.
    int first = steps & 1;
    int second = (steps >> 1) & 1;
    int second_last = (steps >> (N - 2)) & 1;
    int last = (steps >> (N - 1)) & 1;
    if((second_last != last && last != first) || (last != first && first != second)) {
        return 0;
    }

    // temp gets eaten by the >> operations.
    uint64_t temp = steps;
    // Has different bounds than the bottom loop.
    for(int i = 0; i < N - 2; ++i) {
        int previous_three = temp & 0b111;
        if(previous_three == 0b010 || previous_three == 0b101) {
            return 0;
        }
        temp >>= 1;
    }


    // Intersections
    // First point is the origin.
    int points[N + 1] = {0};
    int x = 0;
    int xStep = 1;
    int y = 0;
    int yStep = 1;

    // Makes sure no step before the last step intersects.
    for(int i = 1; i < N; ++i) {
        // Get next point.
        if(steps & 1) {
            y += yStep;
            xStep = -xStep;
        } else {
            x += xStep;
            yStep = -yStep;
        }
        int point = (x << 16) + y;

        // Check if there is an intersection.
        for(int j = 0; j < i; ++j) {
            if(points[j] == point) {
                return 0;
            }
        }
        points[i] = point;
        steps >>= 1;
    }

    // Get last point.
    if(temp & 1) {
        y += yStep;
    } else {
        x += xStep;
    }
    return (x << 16) + y == 0;
}

uint64_t taxi(int N) {
    uint64_t count = 0LL;
    uint64_t current = 0LL;
    uint64_t max = 1LL << N;
    while(current < max) {
        count += isTaxiPolygon(current, N);
        ++current;
    }

    return count * 2;
}

void upTo(int start, int stop)
{
    for(int n = start; n <= stop; n += 4)
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        std::cout << "\nn=" << n << ": " << taxi(n) << std::endl;
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        double totalTime = (double)(end - begin).count() / 1000000000.0;
        std::cout << "Total Time: " << totalTime << std::endl;
    }
}

int main(int argc, char *argv[])
{
    if(argc == 2) {
        int N = atoi(argv[1]);
        upTo(N, N);
    }
    else if(argc == 3) {
        int start = atoi(argv[1]);
        int end = atoi(argv[2]);
        upTo(start, end);
    }

    return 0;
}