#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <climits>
#include <unordered_set>

const int MAX_N_POWER = 6;
const int MAX_N = 64;


// Coordinate pair hash function - Only good for walks shorter than MAX_N.
struct hash_function { 
    size_t operator()(const std::pair<int, int> &p) const { 
        return p.first << MAX_N_POWER + p.second; 
    } 
};
typedef std::unordered_set<std::pair<int, int>, hash_function> point_set;

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

void getEndPoint(uint64_t steps, uint16_t length, int &x, int &y, bool reverse) {
    // Step direction impacted by the starting point (Manhattan Lattice).
    int xStep = 1 - ((y & 1 ^ reverse) << 1);
    int yStep = 1 - ((x & 1 ^ reverse) << 1);
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

point_set getPoints(uint64_t steps, uint16_t length, int &x, int &y, bool reverse) {
    // Step direction impacted by the starting point (Manhattan Lattice).
    int xStep = 1 - ((y & 1 ^ reverse) << 1);
    int yStep = 1 - ((x & 1 ^ reverse) << 1);
    point_set points;
    points.reserve(length + 1);
    for(int i = 0; i < length; ++i) {
        points.emplace(x, y);
        if(steps & 1) {
            y += yStep;
            xStep = -xStep;
        } else {
            x += xStep;
            yStep = -yStep;
        }
        steps >>= 1;
    }
    // points.emplace(x, y);
    return points;
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

bool noIntersection(uint64_t steps, uint16_t length, int x, int y) {
    int cx = 0;
    int xStep = 1;
    int cy = 0;
    int yStep = 1;
    bool noIntersection = cx != x || cy != y;
    int i = 0;
    while(noIntersection && i < length) {
        if(steps & 1) {
            cy += yStep;
            xStep = -xStep;
        } else {
            cx += xStep;
            yStep = -yStep;
        }
        noIntersection = cx != x || cy != y;
        steps >>= 1;
        ++i;
    }
    return noIntersection;
}

inline int approach(uint64_t steps, uint16_t length) {
    return (steps >> (length - 2) & 3);
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

void getBoundingBox(uint64_t steps, uint16_t length, int &minX, int &maxX, int &minY, int &maxY) {
    // Step direction impacted by the starting point (Manhattan Lattice).
    int xStep = 1;
    int yStep = 1;
    int x = 0;
    int y = 0;
    for(int i = 0; i < length; ++i) {
        if(steps & 1) {
            y += yStep;
            xStep = -xStep;
            minY = std::min(y, minY);
            maxY = std::max(y, maxY);
        } else {
            x += xStep;
            yStep = -yStep;
            minX = std::min(x, minX);
            maxX = std::max(x, maxX);
        }
        steps >>= 1;
    }
}


// "On the minX boundary, horizontal step goes left (-x), and taking that horizontal step doesn't break the 2-turn rule (should never be less than length 2)".
inline bool canEscape(uint64_t steps, uint16_t length, int x, int y, int minX, int maxX, int minY, int maxY) {   
    return (x == minX && (y & 1) || x == maxX && !(y & 1)) && (approach(steps, length) != 2)
            || (y == minY && (x & 1) || y == maxY && !(x & 1)) && (approach(steps, length) != 1);
}

// Can some valid walk go from the given point (one of the endpoints of the walk we're testing)
// to the bounding box of the walk (minX to maxX, minY to maxY)? Or do all walks originating at
// the given point die out?
bool extendable(uint64_t jail_steps, uint16_t jail_length, int startX, int startY, bool reverse) {
    // int minX = 0, maxX = 0, minY = 0, maxY = 0;
    // getBoundingBox(jail_steps, jail_length, minX, maxX, minY, maxY);
    return true;
    // Make sure the start of the escape steps have the end of the jail steps for 2-turn.
}



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
            getEndPoint(steps, length, x, y, false);
            if(noLoop(steps, length, x, y) && extendable(steps, length, x, y, false) && extendable(steps, length, 0, 0, true)) {
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
            getEndPoint(steps, length, x, y, false);
            if(noLoop(steps, length, x, y) && extendable(steps, length, x, y, false) && extendable(steps, length, 0, 0, true)) {
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
    std::vector<State> automaton = makeAutomaton(N);

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
    return 0;
}