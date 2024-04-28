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

// HH -> 00 (0)
// HV -> 10 (2)
// VH -> 01 (1)
// VV -> 11 (3)
inline int approach64(uint64_t steps, uint16_t length) {
    return steps >> (length - 2) & 3;
}

void getEndPoint64(uint64_t steps, uint16_t length, int &x, int &y) {
    // Step direction impacted by the starting point (Manhattan Lattice).
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

bool noLoop64(uint64_t steps, int limit, int &x, int &y, int endX, int endY) {
    int xStep = 1;
    int yStep = 1;
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

bool noNarrow64(uint64_t steps, uint16_t length, int &x1, int &y1, int &x2, int &y2, int x3, int y3, int x4, int y4) {
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

// Returns true if the walk is closed clockwise, false otherwise. Assumes the walk is closed and reads left to right.
bool clockwise(uint64_t steps, uint16_t length) {
    int xStep = 1, yStep = 1;
    int left = 0, right = 0;

    // Turn at the origin.
    // bool first = steps & 1;
    // bool last = (steps >> (length - 1)) & 1;
    // if(first != last) { 
    //     left += first; // Vertical start must be left turn.
    //     right += last; // Vertical end must be right turn.
    // }

    bool prev = (steps >> (length - 1)) & 1;
    for(int i = 0; i < length; i++) {
        bool step = steps & 1;
        if(prev != step) {
            if(prev ^ (xStep > 0) ^ (yStep > 0)) {
                right += 1;
            } else {
                left += 1;
            }
        }

        if(step) {
            xStep = -xStep;
        } else {
            yStep = -yStep;
        }
        steps >>= 1;
    }
    if(right != left + 4 && left != right + 4) {
        std::cout << "Clockwise fail for: " << toBinary64(steps, length) << ". Showed " << right << " rights and " << left << " lefts." << std::endl;
    }
    return right == left + 4;
}

// Returns false if you cannot draw a polygon made of same-partiy vertices inside this walk (contour). True otherwise.
bool noVertexPolygon(uint64_t steps, uint16_t length) {
    clockwise(steps, length);
    return false;
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

    inline int approach() {
        if(steps.size() == 1 || lastSegmentLength() >= 2) {
            return approach64(steps.back(), lastSegmentLength());
        } else {
            return ((steps.back() & 1) << 1) + ((steps[steps.size() - 2] >> 63) & 1);
        }
    }

    void getEndPoint(int &x, int &y) {
        if(steps.empty()) {
            x = 0;
            y = 0;
        } else {
            for(int i = 0; i < steps.size() - 1; i++) {
                getEndPoint64(steps[i], 64, x, y);
            }
            getEndPoint64(steps.back(), lastSegmentLength(), x, y);
        }
    }

    bool noLoop(int x, int y, int endX, int endY) {
        bool flag = true;
        int i = 0;
        while(flag && i < steps.size() - 1) {
            flag = noLoop64(steps[i], 64, x, y, endX, endY);
            ++i;
        }
        return flag && noLoop64(steps.back(), lastSegmentLength() - 12, x, y, endX, endY);
    }

    bool noNarrow(int x3, int y3, int x4, int y4) {
        if(length < 10) {
            return true;
        }
        int length = 0;
        int index = 0;
        int xStep = 1;
        int yStep = 1;
        int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
        while(length + 64 < length - 10) {
            if(!noNarrow64(steps[index], 64, x1, y1, x2, y2, x3, y3, x4, y4)) {
                return false;
            }
            length += 64;
            index++;
        }
        // It's not possible to get within 1 step of yourself in 10 steps. The last 2 steps must be omited or they detect themselves.
        int m = (length - 10) & 63;
        bool noNarrowFlag = noNarrow64(steps[index], m, x1, y1, x2, y2, x3, y3, x4, y4);
        return noNarrowFlag;
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

std::vector<int> getStepsToOrigin() {
    std::ifstream ifs(R"(C:\Users\danrs\Documents\GitHub\TaxiWalk\CPP\StepsToNarrowAtOrigin.txt)");
    std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    std::stringstream stream(content);
    std::vector<int> stepsToOrigin;
    int num;
    while(stream >> num) {
        stepsToOrigin.push_back(num);
    }
    return stepsToOrigin;
}


void run(int n) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::vector<int> stepsToOrigin = getStepsToOrigin();
    std::vector<State> walks;

    walks.emplace_back(0, 0);
    uint64_t counts[100] = {0};

    while(!walks.empty()) {
        State current = walks.back();
        std::cout << toBinary64(current.steps, current.length) << std::endl;
        walks.pop_back();
        int prevX = 0, prevY = 0;
        getEndPoint64(current.steps, current.length, prevX, prevY);

        // Horizontal Step.
        if(approach64(current.steps, current.length) != 2 || current.length < 2) {
            uint16_t length = current.length + 1;
            uint64_t steps = current.steps;
            int x = 0, y = 0, _x = 0, _y = 0;
            getEndPoint64(steps, length, x, y);
            LongWalk walk;
            walk.steps.push_back(steps);
            walk.length = length;
            bool noNarrowFlag = walk.noNarrow(prevX, prevY, x, y);
            if(noNarrowFlag) {
                int firstTwoFlipped = ((steps & 1) << 1) | ((steps >> 1) & 1);
                // if(noLoop64(steps, length-12, _x, _y, x, y) && stepsToOrigin[walk.approach() * 40401 + (x + 100) * 201 + y + 100] <= n - length) {
                //     walks.emplace_back(steps, length);
                // } else if(x == 0 && y == 0 && walk.approach() != 2 - (firstTwoFlipped >> 1) && firstTwoFlipped != 2) { // Will loop (at origin, since loops only occur at the last point).
                if(noLoop64(steps, length-12, _x, _y, x, y) && length < n) {
                    walks.emplace_back(steps, length);
                } else if(x == 0 && y == 0) { // Will loop (at origin, since loops only occur at the last point).    
                    counts[length >> 2]++;
                    if(noVertexPolygon(steps, length)) {
                        std::cout << "No Vertex Polygon for: " << walk.toBinary() << std::endl;
                    }
                }
            }
        }

        // Vertical Step.
        if(approach64(current.steps, current.length) != 1 || current.length < 2) {
            uint16_t length = current.length + 1;
            uint64_t steps = current.steps | (1ULL << current.length);

            int x = 0, y = 0, _x = 0, _y = 0;
            getEndPoint64(steps, length, x, y);
            LongWalk walk;
            walk.steps.push_back(steps);
            walk.length = length;
            bool noNarrowFlag = walk.noNarrow(prevX, prevY, x, y);
            if(noNarrowFlag) {
                int firstTwoFlipped = ((steps & 1) << 1) | ((steps >> 1) & 1);
                // if(noLoop64(steps, length-12, _x, _y, x, y) && stepsToOrigin[walk.approach() * 40401 + (x + 100) * 201 + y + 100] <= n - length) {
                //     walks.emplace_back(steps, length);
                // } else if(x == 0 && y == 0 && walk.approach() != 2 - (firstTwoFlipped >> 1) && firstTwoFlipped != 1) { // Will loop (at origin, since loops only occur at the last point).
                if(noLoop64(steps, length-12, _x, _y, x, y) && length < n) {
                    walks.emplace_back(steps, length);
                } else if(x == 0 && y == 0) { // Will loop (at origin, since loops only occur at the last point).    
                    counts[length >> 2]++;
                    if(noVertexPolygon(steps, length)) {
                        std::cout << "No Vertex Polygon for: " << walk.toBinary() << std::endl;
                    }
                }
            }
        }
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::ofstream file("out.txt", std::ios_base::app);
    for(int i = 12; i <= n; i += 4) {
        std::cout << "N=" << i << ": " << counts[i >> 2] << std::endl;
        file << "N=" << i << ": " << counts[i >> 2] << std::endl;
    }
    double totalTime = (double)(end - begin).count() / 1000000000.0;
    std::cout << "Total Time: " << totalTime << std::endl;
    file.close();
}

int main(int argc, char *argv[]) {
    if(argc == 2) {
        int N = atoi(argv[1]);
        run(N);
    }
    return 0;
}