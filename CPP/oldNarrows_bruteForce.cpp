#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <climits>
#include <unordered_set>
#include <deque>

struct State {
    uint64_t var1;
    uint64_t var2;
    State *children[2]{};

    State(uint16_t length, uint64_t steps):
        var1(length), var2(steps) {}
};

std::string toBinary(uint64_t n, uint16_t len) {
    if(len == 0) {
        return "Origin";
    }
    std::string binary;
    for(uint16_t i = 0; i < len; i++) {
        if((n >> i) & 1) {
            binary += "V";
        } else {
            binary += "H";
        }
    }
    return binary;
}

void getPoint(uint64_t steps, uint16_t length, int &x, int &y) {
    int xStep = 1;
    int yStep = 1;
    for(int i = 0; i < length; ++i) {
        if(steps & 1) {
            y += yStep;
            xStep = -xStep;
        } else {
            x += xStep;
            yStep = -yStep;
        }
        steps >>= 1;
    }
}

bool noLoop(uint64_t steps, uint16_t length, int endX, int endY) {
    int x = 0;
    int xStep = 1;
    int y = 0;
    int yStep = 1;
    bool noLoop = x != endX || y != endY;
    int i = 0;
    while(noLoop && i < length - 12) {
        if(steps & 1) {
            y += yStep;
            xStep = -xStep;
        } else {
            x += xStep;
            yStep = -yStep;
        }
        noLoop = x != endX || y != endY;
        steps >>= 1;
        ++i;
    }
    return noLoop;
}

bool noNarrow(uint64_t steps, uint16_t length, int endX, int endY) {
    if(length < 3) {
        return true;
    }

    // if(length == 13) {
    //     std::cout << toBinary(steps, length) << " " << endX << "," << endY << std::endl;
    // }
    

    int back2X = 0; int back2Y = 0;
    getPoint(steps, length - 2, back2X, back2Y);
    int x = 0;
    int xStep = 1;
    int y = 0;
    int yStep = 1;
    bool back2Flag = abs(x - back2X) + abs(y - back2Y) != 1;
    bool endFlag = abs(x - endX) + abs(y - endY) != 1;
    int i = 0;
    int test1X = 0, test1Y = 0, test2X = 0, test2Y = 0;
    uint64_t tempSteps = steps;
    while((back2Flag || endFlag) && i < length - 10) {
        if(tempSteps & 1) {
            y += yStep;
            xStep = -xStep;
        } else {
            x += xStep;
            yStep = -yStep;
        }
        back2Flag &= abs(x - back2X) + abs(y - back2Y) != 1;
        if(abs(x - back2X) + abs(y - back2Y) == 1) {
            test1X = x;
            test1Y = y;
        }
        endFlag &= abs(x - endX) + abs(y - endY) != 1;
        if(abs(x - endX) + abs(y - endY) == 1) {
            test2X = x;
            test2Y = y;
        }
        tempSteps >>= 1;
        ++i;
    }
    // if(!back2Flag && !endFlag) {
    //     std::cout << toBinary(steps, length) << ": " << back2X << "," << back2Y << ":" << test1X << "," << test1Y << " " << endX << "," << endY << ":" << test2X << "," << test2Y << std::endl;
    // }
    return back2Flag || endFlag;
}


int approach(uint64_t steps, uint16_t length) {
    return steps >> (length - 2) & 3;
}


uint64_t run(int n)
{
    std::vector<State> walks;
    // H start.
    walks.emplace_back(1, 0);

    uint64_t count = 0;

    while(!walks.empty()) {
        State current = walks.back();
        walks.pop_back();
        // std::cout << "\nParent: " << toBinary(current->var2, current->var1) << std::endl;
        // Horizontal Step.
        if(approach(current.var2, current.var1) != 2 || current.var1 < 2) {
            uint16_t length = current.var1 + 1;
            uint64_t steps = current.var2;
            // std::cout << "H: " << toBinary(steps, length) << std::endl;

            int x = 0, y = 0;
            getPoint(steps, length, x, y);
            // if(length == 13) {
            //     std::cout  << toBinary(steps, length) << " " << x << "," << y << std::endl;
            // }
            if(noLoop(steps, length, x, y) && noNarrow(steps, length, x, y)) {
                if(length != n) {
                    walks.emplace_back(length, steps);
                    // std::cout << "Added!" << std::endl;

                } else {
                    count++;
                    // std::cout << "Counted!" << std::endl;
                }
            }
        }

        // Vertical Step.
        if(approach(current.var2, current.var1) != 1 || current.var1 < 2) {
            uint16_t length = current.var1 + 1;
            uint64_t steps = current.var2 | (1ULL << current.var1);
            // std::cout << "V: " << toBinary(steps, length) << std::endl;


            int x = 0, y = 0;
            getPoint(steps, length, x, y);
            // if(length == 13) {
            //     std::cout << toBinary(steps, length) << " " << x << "," << y << std::endl;
            // }
            if(noLoop(steps, length, x, y) && noNarrow(steps, length, x, y)) {
                if(length != n) {
                    walks.emplace_back(length, steps);
                    // std::cout << "Added!" << std::endl;

                } else {
                    count++;
                    // std::cout << "Counted!" << std::endl;
                }
            }
        }
    }

    return count << 1;
}

void upTo(int start, int stop) {
    for(int n = start; n <= stop; n += 1) {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        std::cout << "\nn=" << n << ": " << run(n) << std::endl;
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        double totalTime = (double)(end - begin).count() / 1000000000.0;
        std::cout << "Total Time: " << totalTime << std::endl;
    }
}

const int MAX_N = 63;
int main(int argc, char *argv[]) {
    if(argc == 2) {
        int N = atoi(argv[1]);
        if(N >= MAX_N) {
            std::cout << N << " is too large." << std::endl;
            return -1;
        }
        upTo(N, N);
    } else if(argc == 3) {
        int start = atoi(argv[1]);
        int end = atoi(argv[2]);
        if(end >= MAX_N) {
            std::cout << end << " is too large." << std::endl;
            return -1;
        }
        upTo(start, end);
    }
    // int x = 0; int y = 0;
    // // 0001110001100
    // uint64_t steps = 0b0011000111000;
    // uint64_t length = 13;
    // getPoint(steps, length, x, y);
    // std::cout << x << "," << y << std::endl;
    // return 0;
}