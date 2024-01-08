#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <deque>

std::string toBinary(uint64_t n, uint16_t len)
{
    if(len == 0) {
        return "Origin";
    }
    std::string binary;
    for(uint16_t i = 0; i < len; i++)
    {
        if((n >> i) & 1)
        {
            binary += "V";
        }
        else
        {
            binary += "H";
        }
    }
    return binary;
}

struct State {
    uint64_t length;
    uint64_t steps;
    uint8_t parity;
    
    State(uint64_t steps, uint16_t length, uint8_t parity):
        steps(steps), length(length), parity(parity) {}
};


// Bits go right to left, segments go left to right.
const uint64_t max_bit = 1ULL << 63;
struct LongWalk {
    std::vector<uint64_t> steps;
    uint16_t length;
    uint8_t parity;

    // No steps, they will get added either with = or + later.
    LongWalk() {
        length = 0;
        parity = 0;
    }

    LongWalk(uint64_t steps, uint16_t length, uint8_t parity):
        steps(steps), length(length), parity(parity) {}

    void operator=(const LongWalk &other) {
		steps = other.steps;
        length = other.length;
        parity = other.parity;
	}

    LongWalk horizontalStep() {
        LongWalk result = *this;
        if(!(result.length & 63)) {
            result.steps.push_back(0);
        }
        result.length++;
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
        result.length++;
        return result;
    }

    LongWalk append(LongWalk other) {
        LongWalk out = *this;
        for(int i = 0; i < other.steps.size() - 1; i++) {
            uint64_t tempSteps = other.steps[i];
            for(int j = 0; j < 64; j++) {
                if(tempSteps & 1) {
                    out = out.verticalStep();
                } else {
                    out = out.horizontalStep();
                }
                tempSteps >>= 1;
            }
        }
        int m = other.length & 63;
        uint64_t tempSteps = other.steps.back();
        for(int j = 0; j < m; j++) {
            if(tempSteps & 1) {
                out = out.verticalStep();
            } else {
                out = out.horizontalStep();
            }
            tempSteps >>= 1;
        }
        return out;
    }

    LongWalk flip() {
        LongWalk temp;
        int m = length & 63;
        uint64_t last = steps.back();
        for(int j = m - 1; j >= 0; j--) {
            if((last >> j) & 1) {
                temp = temp.verticalStep();
            } else {
                temp = temp.horizontalStep();
            }
        }
        for(int i = steps.size() - 2; i >= 0; i++) {
            uint64_t tempSteps = steps[i];
            for(int j = 63; j >= 0; j--) {
                if((tempSteps >> j) & 1) {
                    temp = temp.verticalStep();
                } else {
                    temp = temp.horizontalStep();
                }
            }
        }
        return temp;
    }

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

    friend std::ostream& operator<<(std::ostream &stream, const LongWalk &longwalk) {
        stream << longwalk.to_string();
        return stream;
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
        getEndPoint(walk->steps.back(), walk->length & 63, x, y);
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
    return flag && noLoop(walk->steps.back(), (walk->length & 63) - 12, x, y, endX, endY);
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
inline bool canEscape(int approach, int x, int y, int minX, int maxX, int minY, int maxY) {   
    return (x == minX && (y & 1) || x == maxX && !(y & 1)) && (approach != 2)
            || (y == minY && (x & 1) || y == maxY && !(x & 1)) && (approach != 1);
}

LongWalk combineJailEscape(uint64_t jail_steps, uint16_t jail_length, LongWalk *escape) {
    LongWalk temp;
    temp.steps.push_back(jail_steps);
    temp.length = jail_length;
    return temp.append(*escape);
}


int shortNarrowHelper(uint64_t steps, uint16_t length, int &x1, int &y1, int &x2, int &y2, int x3, int y3, int x4, int y4) {
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
            if((x1 + y1) & 1) {
                return 1;
            } else {
                return 2;
            }
        }
    }
    return 0;
}

// 0 for none found, 1 for odd start vertex, 2 for even start vertex (the vertex closest to the start of the walk).
// At this level, return the first one. In the rare case there are several per x3y3 x4y4, they will still be caught.
int shortNarrowParity(LongWalk *toCheck, int x3, int y3, int x4, int y4) {
    if(toCheck->length < 10) {
        return 0;
    }
    int length = 0;
    int index = 0;
    int xStep = 1;
    int yStep = 1;
    int x1 = 0;
    int y1 = 0;
    int x2 = 0;
    int y2 = 0;
    while(length + 64 < toCheck->length - 10) {
        int parity = shortNarrowHelper(toCheck->steps[index], 64, x1, y1, x2, y2, x3, y3, x4, y4);
        if(parity) {
            return parity;
        }
        length += 64;
        index++;
    }
    // It's not possible to get within 1 step of yourself in 10 steps. The last 2 steps must be omited or they detect themselves.
    int m = (toCheck->length - 10) & 63;
    return shortNarrowHelper(toCheck->steps[index], m, x1, y1, x2, y2, x3, y3, x4, y4);
}

bool consistentNarrow(LongWalk *walk, int &parity) {
    if(walk->length < 10) {
        return true;
    }

    // (1)-->(2)
    // (4)<--(3)
    int xStep = 1;
    int yStep = 1;
    int x3 = 0;
    int y3 = 0;
    int x4 = 0;
    int y4 = 0;

    LongWalk toCheck;
    for(int i = 0; i < walk->steps.size() - 1; i++) {
        uint64_t tempSteps = walk->steps[i];
        for(int j = 0; j < 64; j++) {
            x3 = x4;
            y3 = y4;
            if(tempSteps & 1) {
                y4 += yStep;
                xStep = -xStep;
                toCheck = toCheck.verticalStep();
            } else {
                x4 += xStep;
                yStep = -yStep;
                toCheck = toCheck.horizontalStep();
            }
            tempSteps >>= 1;

            int newParity = shortNarrowParity(&toCheck, x3, y3, x4, y4);
            if(parity && newParity && parity != newParity) {
                return false;
            } else if(newParity) {
                parity = newParity;
            }
        }
    }
    int m = walk->length & 63;
    uint64_t tempSteps = walk->steps.back();
    for(int j = 0; j < m; j++) {
        x3 = x4;
        y3 = y4;
        if(tempSteps & 1) {
            y4 += yStep;
            xStep = -xStep;
            toCheck = toCheck.verticalStep();
        } else {
            x4 += xStep;
            yStep = -yStep;
            toCheck = toCheck.horizontalStep();
        }
        tempSteps >>= 1;

        int newParity = shortNarrowParity(&toCheck, x3, y3, x4, y4);
        if(newParity) {
            if(parity && parity != newParity) {
                return false;
            } else {
                parity = newParity;
            }
        }
    }
    return true;
}


// Narrow structures must be created by unoccupied odd vertices. Therefore, they must be outside the final contour.
// Odd v1 -> must be closed clockwise (more right turns). Even v1 -> must be closed counterclockwise (more left turns).
bool narrowExcluded(LongWalk *walk, int parity) {
    if(!parity) {
        // Zero parity, no narrow.
        return true;
    }

    int leftTurns = 0;
    int rightTurns = 0;
    bool up = true;
    bool right = true;
    int prevStep = -1;
    for(int i = 0; i < walk->steps.size() - 1; i++) {
        uint64_t tempSteps = walk->steps[i];
        for(int j = 0; j < 64; j++) {
            int step = tempSteps & 1;
            bool diff = up ^ right;
            rightTurns += (!prevStep && step && diff) || ((prevStep == 1) && !step && !diff);
            leftTurns += (!prevStep && step && !diff) || ((prevStep == 1) && !step && diff);
            prevStep = step;
            up ^= (step == 0);
            right ^= step;
            tempSteps >>= 1;
        }
    }
    int m = walk->length & 63;
    uint64_t tempSteps = walk->steps.back();
    for(int j = 0; j < m; j++) {
        int step = tempSteps & 1;
        bool diff = up ^ right;
        rightTurns += (!prevStep && step && diff) || ((prevStep == 1) && !step && !diff);
        leftTurns += (!prevStep && step && !diff) || ((prevStep == 1) && !step && diff);
        prevStep = step;
        up ^= (step == 0);
        right ^= step;
        tempSteps >>= 1;
    }

    // Last step to first step check.
    int step = walk->steps[0] & 1;
    bool diff = up ^ right;
    rightTurns += (!prevStep && step && diff) || ((prevStep == 1) && !step && !diff);
    leftTurns += (!prevStep && step && !diff) || ((prevStep == 1) && !step && diff);

    // Parity 1 -> v1 odd and must be clockwise, parity 2 -> v1 even must be counterclockwise.
    return (parity & 1) == (rightTurns > leftTurns);
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
int extendable(uint64_t jail_steps, uint16_t jail_length, int startX, int startY) {
    if(jail_length < 10) {
        return 2;
    }

    // Fips the walk. Start at origin only if called from reverse call or polygon, reversing a polygon does nothing.
    if(startX == 0 && startY == 0) {
        jail_steps = reverse(jail_steps, jail_length);
        getEndPoint(jail_steps, jail_length, startX, startY);
    }

    int minX = 0, maxX = 0, minY = 0, maxY = 0;
    getBoundingBox(jail_steps, jail_length, minX, maxX, minY, maxY);

    int firstTwoFlipped = ((jail_steps & 1) << 1) | ((jail_steps >> 1) & 1);
    int lastTwo = approach(jail_steps, jail_length);

    // Can escape with a single step from the end of the jail, hopefully a good number fall into this case.
    if(canEscape(lastTwo, startX, startY, minX, maxX, minY, maxY)) {
        return 1;
    }

    bool test = false;

    std::deque<LongWalk> escapes;
    escapes.emplace_back();
    while(!escapes.empty()) {
        LongWalk *current = &escapes.front();
        int a = longApproach(current, lastTwo);
        int prevX = startX;
        int prevY = startY;
        getLongEndPoint(current, prevX, prevY);

        // Horizontal Step.
        if(a != 2) {
            LongWalk h = current->horizontalStep();
            int ha = longApproach(&h, lastTwo);
            int x = startX;
            int y = startY;
            getLongEndPoint(&h, x, y);
            LongWalk combined = combineJailEscape(jail_steps, jail_length, &h);
            int parity = 0;
            if(x == 0 && y == 0 && ha != 2 - (firstTwoFlipped >> 1) && firstTwoFlipped != 2 && consistentNarrow(&combined, parity) && narrowExcluded(&combined, parity)) {
                return 2;
            }
            else if(longNoLoop(&h, startX, startY, x, y) && noIntersection(jail_steps, jail_length, x, y) && consistentNarrow(&combined, parity)) {
                if(canEscape(ha, x, y, minX, maxX, minY, maxY)) {
                    return 1;
                } else {
                    escapes.push_back(h);
                }
            }
        }

        // Vertical Step.
        if(a != 1) {
            LongWalk v = current->verticalStep();
            int va = longApproach(&v, lastTwo);
            int x = startX;
            int y = startY;
            getLongEndPoint(&v, x, y);
            int parity = 0;
            LongWalk combined = combineJailEscape(jail_steps, jail_length, &v);
                if(x == 0 && y == 0 && va != 2 - (firstTwoFlipped >> 1) && firstTwoFlipped != 1 && consistentNarrow(&combined, parity) && narrowExcluded(&combined, parity)) {
                    return 2;
                }
                else if(longNoLoop(&v, startX, startY, x, y) && noIntersection(jail_steps, jail_length, x, y) && consistentNarrow(&combined, parity)) {
                    if(canEscape(va, x, y, minX, maxX, minY, maxY)) {
                        return 1;
                    } else {
                        escapes.push_back(v);
                    }
                }
        }
        escapes.pop_front();
    }
    // All walks died, could not escape.
    // if(test){
    //     std::cout << "FAILED: " << toBinary(jail_steps, jail_length) << std::endl;
    // }
    return 0;
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

        // Horizontal Step.
        if(approach(current.steps, current.length) != 2 || current.length < 2) {
            uint16_t length = current.length + 1;
            uint64_t steps = current.steps;
            int x = 0, y = 0, sx = 0, sy = 0;
            getEndPoint(steps, length, x, y);
            LongWalk walk;
            walk.steps.push_back(steps);
            walk.length = length;
            int parity = 0;
            if(noLoop(steps, length-12, sx, sy, x, y) && consistentNarrow(&walk, parity)) {
                int forward_extendible = extendable(steps, length, x, y);
                if(forward_extendible == 2 || (forward_extendible && extendable(steps, length, 0, 0))) {
                    if(length != n) {
                        walks.emplace_back(length, steps);
                    } else {
                        count++;
                    }
                }
            }
        }

        // Vertical Step.
       if(approach(current.steps, current.length) != 1 || current.length < 2) {
            uint16_t length = current.length + 1;
            uint64_t steps = current.steps | (1ULL << current.length);
            int x = 0, y = 0, sx = 0, sy = 0;
            getEndPoint(steps, length, x, y);
            LongWalk walk;
            walk.steps.push_back(steps);
            walk.length = length;
            int parity = 0;
            if(noLoop(steps, length-12, sx, sy, x, y) && consistentNarrow(&walk, parity)) {
                int forward_extendible = extendable(steps, length, x, y);
                if(forward_extendible == 2 || (forward_extendible && extendable(steps, length, 0, 0))) {
                    if(length != n) {
                        walks.emplace_back(length, steps);
                    } else {
                        count++;
                    }
                }
            }
        }
    }

    return count << 1;
}

void upTo(int start, int stop) {
    for(int n = start; n <= stop; n += 1)
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        std::cout << "\nn=" << n << ": " << run(n) << std::endl;
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        double totalTime = (double)(end - begin).count() / 1000000000.0;
        std::cout << "Total Time: " << totalTime << std::endl;
    }
}

int main(int argc, char *argv[])
{
    if(argc == 2)
    {
        int N = atoi(argv[1]);
        upTo(N, N);
    }
    else if(argc == 3)
    {
        int start = atoi(argv[1]);
        int end = atoi(argv[2]);
        upTo(start, end);
    }
    return 0;
}