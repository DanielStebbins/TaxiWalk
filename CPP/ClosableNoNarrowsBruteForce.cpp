#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <deque>

std::string toBinary64(uint64_t steps, uint16_t length) {
    if(length == 0) {
        return "Origin";
    }
    std::string binary;
    for(uint16_t i = 0; i < length; i++)
    {
        if((steps >> (length - 1 - i)) & 1) {
            binary += "1";
        } else {
            binary += "0";
        }
    }
    return binary;
}

struct State {
    uint64_t length;
    uint64_t steps;
    
    State(uint64_t steps, uint16_t length):
        steps(steps), length(length) {}
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

    LongWalk(uint64_t s, uint16_t length):
        length(length) {
            steps.push_back(s);
        }

    void operator=(const LongWalk &other) {
		steps = other.steps;
        length = other.length;
	}

    LongWalk horizontalStep() {
        LongWalk result = *this;
        if(lastSegmentLength() == 64) {
            result.steps.push_back(0);
        }
        result.length++;
        return result;
    }

    LongWalk verticalStep() {
        LongWalk result = *this;
        int m = lastSegmentLength();
        if(m) {
            result.steps.back() |= 1ULL << m;
        } else {
            result.steps.push_back(1);
        }
        result.length++;
        return result;
    }

    inline uint64_t lastSegmentLength() { 
        return length - (64 * (steps.size() - 1));
    }

    // Each segment is right to left, but the segments stack left to right (last steps are in the leftmost bits of the rightmost u64).
    const std::string toBinary() {
        if(steps.empty()) {
            return "Origin";
        } else {
            std::string binary;
            binary.append(toBinary64(steps.back(), length - (64 * (steps.size() - 1))));
            for(int i = steps.size() - 2; i >= 0; i--) {
                binary.append(toBinary64(steps[i], 64));
            }
            return binary;
        }
    }
};

void getEndPoint(uint64_t steps, uint16_t length, int &x, int &y) {
    // Step direction impacted by the starting point (Manhattan Lattice).
    int xStep = 1 - ((y & 1) << 1);
    int yStep = 1 - ((x & 1) << 1);
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

void getLongEndPoint(LongWalk *walk, int &x, int &y) {
    if(walk->steps.empty()) {
        x = 0;
        y = 0;
    } else {
        for(int i = 0; i < walk->steps.size() - 1; i++) {
            getEndPoint(walk->steps[i], 64, x, y);
        }
        getEndPoint(walk->steps.back(), walk->lastSegmentLength(), x, y);
    }
}


bool noLoop(uint64_t steps, int limit, int &x, int &y, int endX, int endY) {
    int xStep = 1 - ((y & 1) << 1);
    int yStep = 1 - ((x & 1) << 1);
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

bool longNoLoop(LongWalk *walk, int x, int y, int endX, int endY) {
    bool flag = true;
    int i = 0;
    while(flag && i < walk->steps.size() - 1) {
        flag = noLoop(walk->steps[i], 64, x, y, endX, endY);
        ++i;
    }
    return flag && noLoop(walk->steps.back(), walk->lastSegmentLength() - 12, x, y, endX, endY);
}

// HH -> 00 (0)
// HV -> 10 (2)
// VH -> 01 (1)
// VV -> 11 (3)
inline int approach(uint64_t steps, uint16_t length) {
    return steps >> (length - 2) & 3;
}

inline int longApproach(LongWalk *walk) {
    return approach(walk->steps.back(), walk->lastSegmentLength());
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
inline bool canEscape(int approach, int x, int y, int minX, int maxX, int minY, int maxY) {
    // std::cout << approach << " " << x << " " << y << " " << minX << " " << maxX << " " << minY << " " << maxY << std::endl;
    return (x == minX && (y & 1) || x == maxX && !(y & 1)) && (approach != 2)
            || (y == minY && (x & 1) || y == maxY && !(x & 1)) && (approach != 1);
}

bool noNarrowHelper(uint64_t steps, uint16_t length, int &x1, int &y1, int &x2, int &y2, int x3, int y3, int x4, int y4) {
    int xStep = 1;
    int yStep = 1;
    for(int i = 0; i < length; i++) {
        x1 = x2;
        y1 = y2;
        if(steps & 1) {
            y2 += yStep;
            xStep = -xStep;
        } else {
            x2 += xStep;
            yStep = -yStep;
        }
        steps >>= 1;

        if(abs(x4 - x1) + abs(y4 - y1) == 1 && abs(x3 - x2) + abs(y3 - y2) == 1) {
            return false;
        }
    }
    return true;
}

bool noNarrow(LongWalk *toCheck, int x3, int y3, int x4, int y4) {
    if(toCheck->length < 10) {
        return true;
    }
    int length = 0;
    int index = 0;
    int xStep = 1;
    int yStep = 1;
    int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    while(length + 64 < toCheck->length - 10) {
        if(!noNarrowHelper(toCheck->steps[index], 64, x1, y1, x2, y2, x3, y3, x4, y4)) {
            return false;
        }
        length += 64;
        index++;
    }
    // It's not possible to get within 1 step of yourself in 10 steps. The last 2 steps must be omited or they detect themselves.
    int m = (toCheck->length - 10) & 63;
    bool noNarrowFlag = noNarrowHelper(toCheck->steps[index], m, x1, y1, x2, y2, x3, y3, x4, y4);
    // if(!noNarrowFlag) {
    //     std::cout << toBinary(toCheck->steps[0], toCheck->length) << std::endl;
    // }
    return noNarrowFlag;
}

// Flips the bits of a walk for the backwards extendability check.
uint64_t reverse(uint64_t steps, uint16_t length) {
    uint64_t tempSteps = 0ULL;
    for(int i = 0; i < length; i++) {
        tempSteps <<= 1;
        tempSteps |= (steps >> i) & 1;
    }
    return tempSteps;
}


// Can some valid walk go from the given point (one of the endpoints of the walk we're testing)
// to the bounding box of the walk (minX to maxX, minY to maxY)? Or do all walks originating at
// the given point die out?
// Returns 0 if not extendable (boxed), 1 if extendable (escapes bounding box), 2 if encountered the goal point.
int extendable(LongWalk *jail, int startX, int startY) {
    if(jail->length < 10) {
        return 2;
    }

    LongWalk jailBackup = *jail;
    // Fips the walk. Start at origin only if called from reverse call or polygon, reversing a polygon does nothing.
    if(startX == 0 && startY == 0) {
        // Jail is never longer than 64 steps.
        jail->steps[0] = reverse(jail->steps[0], jail->length);
        getEndPoint(jail->steps[0], jail->length, startX, startY);
    }



    int minX = 0, maxX = 0, minY = 0, maxY = 0;
    getBoundingBox(jail->steps[0], jail->length, minX, maxX, minY, maxY);

    int firstTwoFlipped = ((jail->steps[0] & 1) << 1) | ((jail->steps[0] >> 1) & 1);
    int lastTwo = approach(jail->steps[0], jail->length);

    // Can escape with a single step from the end of the jail, hopefully a good number fall into this case.
    if(canEscape(lastTwo, startX, startY, minX, maxX, minY, maxY)) {
        *jail = jailBackup;
        return 1;
    }

    std::deque<LongWalk> escapes;
    escapes.push_back(*jail);
    while(!escapes.empty()) {
        LongWalk *current = &escapes.front();
        int a = longApproach(current);
        int prevX = 0;
        int prevY = 0;
        getLongEndPoint(current, prevX, prevY);

        // Horizontal Step.
        if(a != 2) {
            LongWalk h = current->horizontalStep();
            // std::cout << h.toBinary() << std::endl;
            int ha = longApproach(&h);
            int x = 0;
            int y = 0;
            getLongEndPoint(&h, x, y);
            bool noNarrowFlag = noNarrow(&h, prevX, prevY, x, y);
            if(x == 0 && y == 0 && ha != 2 - (firstTwoFlipped >> 1) && firstTwoFlipped != 2 && noNarrowFlag) {
                *jail = jailBackup;
                // std::cout << toBinary(h.steps[0], h.length) << std::endl;
                return 2;
            }
            else if(longNoLoop(&h, 0, 0, x, y) && noNarrowFlag) {
                if(canEscape(ha, x, y, minX, maxX, minY, maxY)) {
                    *jail = jailBackup;
                    return 1;
                } else {
                    escapes.push_back(h);
                }
            }
        }

        // Vertical Step.
        if(a != 1) {
            LongWalk v = current->verticalStep();
            // std::cout << v.toBinary() << std::endl;
            int va = longApproach(&v);
            int x = 0;
            int y = 0;
            getLongEndPoint(&v, x, y);
            bool noNarrowFlag = noNarrow(&v, prevX, prevY, x, y);
            if(x == 0 && y == 0 && va != 2 - (firstTwoFlipped >> 1) && firstTwoFlipped != 1 && noNarrowFlag) {
                *jail = jailBackup;
                // std::cout << toBinary(v.steps[0], v.length) << std::endl;
                return 2;
            }
            else if(longNoLoop(&v, 0, 0, x, y) && noNarrowFlag) {
                if(canEscape(va, x, y, minX, maxX, minY, maxY)) {
                    *jail = jailBackup;
                    return 1;
                } else {
                    escapes.push_back(v);
                }
            }
        }
        escapes.pop_front();
    }
    // All walks died, could not escape.
    *jail = jailBackup;
    return 0;
}



void run(int n) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::vector<State> walks;
    // H start.
    walks.emplace_back(0, 1);
    uint64_t counts[100] = {0};
    counts[1] = 1;
    uint64_t count = 0;
    // uint64_t forward2Count = 0;

    // std::ofstream f;
    // f.open("brute.txt");

    while(!walks.empty()) {
        State current = walks.back();
        walks.pop_back();
        int prevX = 0, prevY = 0;
        getEndPoint(current.steps, current.length, prevX, prevY);

        // Horizontal Step.
        if(approach(current.steps, current.length) != 2 || current.length < 2) {
            uint16_t length = current.length + 1;
            uint64_t steps = current.steps;
            int x = 0, y = 0, _x = 0, _y = 0;
            getEndPoint(steps, length, x, y);
            LongWalk walk;
            walk.steps.push_back(steps);
            walk.length = length;
            bool noNarrowFlag = noNarrow(&walk, prevX, prevY, x, y);
            if(noLoop(steps, length-12, _x, _y, x, y) && noNarrowFlag) {
                int forward_extendible = extendable(&walk, x, y);
                // if(forward_extendible == 2) {
                //     forward2Count++;
                // }
                if(forward_extendible == 2 || (forward_extendible && extendable(&walk, 0, 0))) {
                    if(length != n) {
                        walks.emplace_back(steps, length);
                    }
                    counts[length]++;
                    // if(length == n) {
                    //     f << toBinary64(steps, length) << std::endl;
                    // }
                }
            }
        }

        // Vertical Step.
       if(approach(current.steps, current.length) != 1 || current.length < 2) {
            uint16_t length = current.length + 1;
            uint64_t steps = current.steps | (1ULL << current.length);

            int x = 0, y = 0, _x = 0, _y = 0;
            getEndPoint(steps, length, x, y);
            LongWalk walk;
            walk.steps.push_back(steps);
            walk.length = length;
            bool noNarrowFlag = noNarrow(&walk, prevX, prevY, x, y);
            if(noLoop(steps, length-12, _x, _y, x, y) && noNarrowFlag) {
                int forward_extendible = extendable(&walk, x, y);
                // if(forward_extendible == 2) {
                //     forward2Count++;
                // }
                if(forward_extendible == 2 || (forward_extendible && extendable(&walk, 0, 0))) {
                    if(length != n) {
                        walks.emplace_back(steps, length);
                    }
                    counts[length]++;
                    // if(length == n) {
                    //     f << toBinary64(steps, length) << std::endl;
                    // }
                }
            }
        }
    }
    // f.close();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    for(int i = 1; i <= n; i++) {
        std::cout << "N=" << i << ": " << (counts[i] << 1) << std::endl;
    }
    double totalTime = (double)(end - begin).count() / 1000000000.0;
    std::cout << "Total Time: " << totalTime << std::endl;
    // std::cout << forward2Count << std::endl;
}

int main(int argc, char *argv[]) {
    if(argc == 2)
    {
        int N = atoi(argv[1]);
        run(N);
    }
    return 0;

    // LongWalk walk(0b000000001111111110011000000011111110, 36);
    // int x = 0, y = 0;
    // getEndPoint(walk.steps[0], walk.length, x, y);
    // std::cout << extendable(&walk, 0, 0) << std::endl;
}