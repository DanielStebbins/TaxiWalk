#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <climits>
#include <unordered_set>
#include <deque>

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

struct State {
    uint64_t length;
    uint64_t steps;

    State(uint16_t length, uint64_t steps):
        length(length), steps(steps) {}
};


// Bits go right to left, segments go left to right.
const uint64_t max_bit = 1ULL << 63;
struct LongWalk {
    std::vector<uint64_t> steps;
    uint16_t length;

    // No steps, they will get added either with = or + later.
    LongWalk() {
        length = 0;
    }

    void operator=(const LongWalk &other) {
		steps = other.steps;
        length = other.length;
	}

    LongWalk horizontalStep() {
        LongWalk result = *this;
        if(!(result.length & 63)) {
            result.steps.push_back(0);
        }
        result.length += 1;
        return result;
    }

    LongWalk verticalStep() {
        LongWalk result = *this;
        int m = result.length & 63;
        if(m) {
            result.steps.back() |= 1ULL << m;
        } else {
            result.steps.push_back(1);
        }
        result.length += 1;
        return result;
    }

    // operator bool() {
    //     return steps.size() > 1 || (steps.size() == 1 && steps[0] != 0ULL);
    // }

    std::string to_string() const {
        if(steps.empty()) {
            return "Empty!";
        } else {
            std::string out = "";
            for(int i = steps.size() - 1; i >= 0; --i) {
			    out += toBinary(steps[i], 64);
            }

            // Avoid leading 0s.
            int i = 0;
            while(i < out.length() && out[i] == '0') {
                ++i;
            }
            return out.substr(i);
            return out;
        }
    }

    friend std::ostream& operator<<(std::ostream &stream, const LongWalk &bigsum) {
        stream << bigsum.to_string();
        return stream;
	}
};


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

void getLongEndPoint(LongWalk *walk, int &x, int &y, bool reverse) {
    for(int i = 0; i < walk->steps.size() - 1; ++i) {
        getEndPoint(walk->steps[i], 64, x, y, reverse);
    }
    getEndPoint(walk->steps.back(), walk->length & 63, x, y, reverse);
}


bool noLoop(uint64_t steps, int limit, int &x, int &y, int endX, int endY, bool reverse) {
    int xStep = 1 - ((y & 1 ^ reverse) << 1);
    int yStep = 1 - ((x & 1 ^ reverse) << 1);
    bool noLoop = x != endX || y != endY;
    int i = 0;
    // Limit can be decreased to length-12 for most checks, but not all. Decide when passing length in.
    while(noLoop && i < limit) {
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

bool longNoLoop(LongWalk *walk, int x, int y, int endX, int endY, bool reverse) {
    bool flag = true;
    int i = 0;
    while(flag && i < walk->steps.size() - 1) {
        flag = noLoop(walk->steps[i], 64, x, y, endX, endY, reverse);
        ++i;
    }
    return flag && noLoop(walk->steps.back(), (walk->length & 63) - 12, x, y, endX, endY, reverse);
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


// HH -> 00 (0)
// HV -> 10 (2)
// VH -> 01 (1)
// VV -> 11 (3)
inline int approach(uint64_t steps, uint16_t length) {
    return steps >> (length - 2) & 3;
}

inline int longApproach(LongWalk *walk, uint16_t lastTwo) {
    if(walk->length == 0) {
        return lastTwo;
    } else if (walk->length == 1) {
        return (walk->steps[0] << 1) | (lastTwo >> 1);
    } else {
        return approach(walk->steps.back(), walk->length & 63);
    }
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
inline bool canEscape(int approach, int x, int y, int minX, int maxX, int minY, int maxY, bool reverse) {   
    return (x == minX && (y & 1 ^ reverse) || x == maxX && !(y & 1 ^ reverse)) && (approach != 2)
            || (y == minY && (x & 1 ^ reverse) || y == maxY && !(x & 1 ^ reverse)) && (approach != 1);
}


// Can some valid walk go from the given point (one of the endpoints of the walk we're testing)
// to the bounding box of the walk (minX to maxX, minY to maxY)? Or do all walks originating at
// the given point die out?
bool extendable(uint64_t jail_steps, uint16_t jail_length, int startX, int startY, bool reverse) {
    // if(jail_steps == 0b100011111111111000000000001111111110000000) {
    //     std::cout << toBinary(jail_steps, jail_length) << std::endl;
    // }
    // HHHHHHHVVVVVVVVVVHHHHHHHHHHVVVVVVVVVVHHHV

    if(jail_length < 10) {
        return true;
    }

    int minX = 0, maxX = 0, minY = 0, maxY = 0;
    getBoundingBox(jail_steps, jail_length, minX, maxX, minY, maxY);


    int lastTwo = approach(jail_steps, jail_length);
    if(reverse) {
        lastTwo = ((jail_steps & 1) << 1) | ((jail_steps >> 1) & 1);
    }

    // Can escape with a single step from the end of the jail, hopefully a good number fall into this case.
    if(canEscape(lastTwo, startX, startY, minX, maxX, minY, maxY, reverse)) {
        return true;
    }

    std::deque<LongWalk> escapes;
    escapes.emplace_back();

    while(!escapes.empty()) {
        LongWalk *current = &escapes.front();
        int a = longApproach(current, lastTwo);

        // Horizontal Step.
        if(a != 2) {
            LongWalk h = current->horizontalStep();
            int x = startX;
            int y = startY;
            getLongEndPoint(&h, x, y, reverse);
            if(longNoLoop(&h, startX, startY, x, y, reverse) && noIntersection(jail_steps, jail_length, x, y)) {
                int a = longApproach(&h, lastTwo);
                if(canEscape(a, x, y, minX, maxX, minY, maxY, reverse)) {
                    return true;
                } else {
                    escapes.push_back(h);
                }
            }
        }

        // Vertical Step.
        if(a != 1) {
            LongWalk v = current->verticalStep();
            int x = startX;
            int y = startY;
            getLongEndPoint(&v, x, y, reverse);
            if(longNoLoop(&v, startX, startY, x, y, reverse) && noIntersection(jail_steps, jail_length, x, y)) {
                int a = longApproach(&v, lastTwo);
                if(canEscape(a, x, y, minX, maxX, minY, maxY, reverse)) {
                    return true;
                } else {
                    escapes.push_back(v);
                }
            }
        }
        escapes.pop_front();
    }
    // All walks died, could not escape.
    std::cout << toBinary(jail_steps, jail_length) << std::endl;
    return false;
}



uint64_t run(int n)
{
    std::deque<State> walks;
    // H start.
    walks.emplace_back(1, 0);

    uint64_t count = 0;

    while(!walks.empty()) {
        State *current = &walks.front();
        // std::cout << "\nParent: " << toBinary(current->steps, current->length) << std::endl;
        // Horizontal Step.
        if(approach(current->steps, current->length) != 2 || current->length < 2) {
            uint16_t length = current->length + 1;
            uint64_t steps = current->steps;
            // std::cout << "H: " << toBinary(steps, length) << std::endl;

            int x = 0, y = 0, sx = 0, sy = 0;
            getEndPoint(steps, length, x, y, false);
            // std::cout << x << " " << y << " " << sx << " " << sy << std::endl;
            if(noLoop(steps, length-12, sx, sy, x, y, false)) {
                // if(length == 25) {
                    // std::cout << "\nJail: " << toBinary(steps, length) << std::endl;
                // }

                if(extendable(steps, length, x, y, false) && extendable(steps, length, 0, 0, true)) {
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

        // Vertical Step.
       if(approach(current->steps, current->length) != 1 || current->length < 2) {
            uint16_t length = current->length + 1;
            uint64_t steps = current->steps | (1ULL << current->length);
            // std::cout << "V: " << toBinary(steps, length) << std::endl;


            int x = 0, y = 0, sx = 0, sy = 0;
            getEndPoint(steps, length, x, y, false);
            // std::cout << x << " " << y << " " << sx << " " << sy << std::endl;
            if(noLoop(steps, length-12, sx, sy, x, y, false)) {
                // if(length == 25) {
                    // std::cout << "\nJail: " << toBinary(steps, length) << std::endl;
                // }
                if(extendable(steps, length, x, y, false) && extendable(steps, length, 0, 0, true)) {
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
        walks.pop_front();
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
    // extendable(0b10001111111111100000000011111111100000, 38, -1, 1, false);
    // std::cout << "Done!" << std::endl;
    return 0;
}