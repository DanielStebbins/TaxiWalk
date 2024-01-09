// Runs N=47 in 11 seconds using 0.5GB.
// Runs N=51 in 66 seconds using 2.8GB.

// Old Predictions (Time x1.58 (x6.2 per 4), Space x1.53 (x5.5 per 4)):
// N=55: 7m45s, 16GB.
// N=59: 48m, 87GB.
// N=63: 5h, 482GB.

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <climits>

struct State {
    uint64_t var1;
    uint64_t var2;
    State *children[2]{};

    State(uint16_t length, uint64_t steps, State *null_state):
        var1(length), var2(steps), children{null_state, null_state} {}
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


int approach(uint64_t steps, uint16_t length) {
    return steps >> (length - 2) & 3;
}


void reduce(uint64_t &steps, uint16_t &length, int endX, int endY, int n, std::vector<int> const &stepsToOrigin) {
    while(stepsToOrigin[approach(steps, length) * 40401 + (endX + 100) * 201 + endY + 100] > n - length) {
        if(steps & 1) {
            endX = -endX;
            endY -= 1;
        } else {
            endX -= 1;
            endY = -endY;
        }
        steps >>= 1;
        --length;

        if(steps & 1) {
            steps ^= (1ULL << length) - 1;
            int temp = endX;
            endX = endY;
            endY = temp;
        }
    }

    // If first step is vertical, flip to horizontal.
    if(steps & 1) {
        steps ^= (1ULL << length) - 1;
    }
}

std::vector<int> getStepsToOrigin()
{
    std::ifstream ifs(R"(C:\Users\danrs\Documents\GitHub\TaxiWalk\CPP\StepsToOriginFlipped.txt)");
    std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    std::stringstream stream(content);
    std::vector<int> stepsToOrigin;
    int num;
    while(stream >> num) {
        stepsToOrigin.push_back(num);
    }
    return stepsToOrigin;
}

#include <algorithm>

std::vector<State> makeAutomaton(int n)
{
    std::vector<int> stepsToOrigin = getStepsToOrigin();

    std::vector<State> states;
    states.reserve(91355000);
    // Null State.
    states.emplace_back(0, 0, nullptr);
    // Origin.
    states.emplace_back(0, 0, &states[0]);

    uint64_t untreated = 1;

    while(untreated < states.size()) {
        // Horizontal Step.
        if(approach(states[untreated].var2, states[untreated].var1) != 2 || states[untreated].var1 < 2) {
            uint16_t length = states[untreated].var1 + 1;
            uint64_t steps = states[untreated].var2;

            int x = 0, y = 0;
            getPoint(steps, length, x, y);
            if(noLoop(steps, length, x, y)) {
                reduce(steps, length, x, y, n, stepsToOrigin);
                State *parent = &states[1];
                uint64_t tempSteps = steps;
                for(int i = 0; i < length - 1; ++i) {
                    parent = parent->children[tempSteps & 1];
                    tempSteps >>= 1;
                }
                if(parent->children[tempSteps] == &states[0]) {
                    states.emplace_back(length, steps, &states[0]);
                    states[untreated].children[0] = &states.back();
                } else {
                    states[untreated].children[0] = parent->children[tempSteps];
                }
            }
        }

        // Vertical Step.
        if(approach(states[untreated].var2, states[untreated].var1) != 1 || states[untreated].var1 < 2)
        {
            uint16_t length = states[untreated].var1 + 1;
            uint64_t steps = states[untreated].var2 | (1ULL << states[untreated].var1);

            int x = 0, y = 0;
            getPoint(steps, length, x, y);
            if(noLoop(steps, length, x, y)) {
                reduce(steps, length, x, y, n, stepsToOrigin);
                State *parent = &states[1];
                uint64_t tempSteps = steps;
                for(int i = 0; i < length - 1; ++i) {
                    parent = parent->children[tempSteps & 1];
                    tempSteps >>= 1;
                }
                if(parent->children[tempSteps] == &states[0]) {
                    states.emplace_back(length, steps, &states[0]);
                    states[untreated].children[1] = &states.back();
                } else {
                    states[untreated].children[1] = parent->children[tempSteps];
                }
            }
        }
        ++untreated;
    }

    // Reset first 16 bytes to use when running the automaton.
    for(auto & state : states) {
        state.var1 = 0;
        state.var2 = 0;
    }
    return states;
}

uint64_t taxi(int N) {
    std::vector<State> automaton = makeAutomaton(15);

    // Sets "H" to current count 1.
    automaton[2].var1 = 1;

    // Ends one step early, because on the final loop there's no need to move var2 to var1.
    for(int n = 2; n < N; ++n) {
        // Skips first state, the null state.
        for(int i = 1; i < automaton.size(); ++i) {
            State state = automaton[i];
            if(state.var1) {
                state.children[0]->var2 += state.var1;
                state.children[1]->var2 += state.var1;
            }
        }
        for(auto & state : automaton) {
            state.var1 = state.var2;
            state.var2 = 0;
        }
    }

    State *null_state = &automaton[0];
    uint64_t taxiWalks = 0;
    for(int i = 1; i < automaton.size(); ++i) {
        State state = automaton[i];
        taxiWalks += state.var1 * ((state.children[0] != null_state) + (state.children[1] != null_state));
    }
    return taxiWalks << 1;
}

void upTo(int start, int stop) {
    for(int n = start; n <= stop; n += 4) {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        std::cout << "\nn=" << n << ": " << taxi(n) << std::endl;
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        double totalTime = (double)(end - begin).count() / 1000000000.0;
        std::cout << "Total Time: " << totalTime << std::endl;
    }
}

int main(int argc, char *argv[]) {
    if(argc == 2) {
        int N = atoi(argv[1]);
        upTo(N, N);
    } else if(argc == 3) {
        int start = atoi(argv[1]);
        int end = atoi(argv[2]);
        upTo(start, end);
    }
    return 0;
}