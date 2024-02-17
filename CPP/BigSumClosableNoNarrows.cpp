#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <climits>
#include <deque>
#include <algorithm>

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

void getEndPoint64(uint64_t steps, uint16_t length, int &x, int &y) {
    // Step direction impacted by the starting point (Manhattan Lattice).
    int xStep = 1 - ((y & 1) << 1);
    int yStep = 1 - ((x & 1) << 1);
    for(int i = 0; i < length; i++) {
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

// HH -> 00 (0)
// HV -> 10 (2)
// VH -> 01 (1)
// VV -> 11 (3)
inline int approach64(uint64_t steps, uint16_t length) {
    return (steps >> (length - 2)) & 3;
}

// "On the minX boundary, horizontal step goes left (-x), and taking that horizontal step doesn't break the 2-turn rule (should never be less than length 2)".
inline bool canEscape(int approach, int x, int y, int minX, int maxX, int minY, int maxY) {
    // std::cout << approach << " " << x << " " << y << " " << minX << " " << maxX << " " << minY << " " << maxY << std::endl;
    return (x == minX && (y & 1) || x == maxX && !(y & 1)) && (approach != 2)
            || (y == minY && (x & 1) || y == maxY && !(x & 1)) && (approach != 1);
}

// Checks a 64-step segment for narrows including the given points (x3,y3) and (x4,y4).
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




// Bits go right to left, segments go left to right.
const uint64_t max_bit = 1ULL << 63;
struct BigSum {
    std::vector<uint64_t> segments;
    
    BigSum(uint64_t value = 0) {
        segments.push_back(value);
    }

    void operator=(const BigSum &other) {
		segments = other.segments;
	}
 
	void operator=(uint64_t value) {
		segments.clear();
		segments.push_back(value);
	}

    BigSum operator+(const BigSum &other) const {
        BigSum result = other;
        bool carry = false;
        int i = 0;
        while(i < std::max(segments.size(), other.segments.size()) || carry) {
            if(i == result.segments.size()) {
                result.segments.push_back(0);
            }
            uint64_t previous_value = result.segments[i];
            result.segments[i] += carry + (i < segments.size() ? segments[i] : 0);
            carry = result.segments[i] < previous_value;
            i++;
        }
        return result;
	}

    void operator+=(const BigSum &other) {
		*this = *this + other;
	}

    operator bool() {
        return segments.size() > 1 || (segments.size() == 1 && segments[0] != 0ULL);
    }
};


struct LongWalk {
    BigSum var1; // Steps and count1.
    BigSum var2; // Length/narrowCount/clockwiseness and count2. Clockwise uses leftmost bit, narrowCount uses the next 7 bits, length uses rightmost 56 bits. Never longer than 1 u64 in the walk phase.

    // No steps, they will get added either with = or + later.
    LongWalk() {
        var1 = 0ULL;
        var2 = 0ULL;
    }

    LongWalk(uint64_t steps, uint64_t length):
        var1(steps), var2(length) {}

    void operator=(const LongWalk &other) {
		var1 = other.var1;
        var2 = other.var2;
	}

    // Each segment is right to left, but the segments stack left to right (last steps are in the leftmost bits of the rightmost u64).
    const std::string toBinary() {
        if(var1.segments.empty()) {
            return "Origin";
        } else {
            std::string binary;
            binary.append(toBinary64(steps()->back(), lastSegmentLength()));
            for(int i = steps()->size() - 2; i >= 0; i--) {
                binary.append(toBinary64((*steps())[i], 64));
            }
            return binary;
        }
    }

    // Bitpacking convenience functions for var2.
    inline std::vector<uint64_t> *steps() { return &var1.segments; }
    // Length 0 -> 0, length 63 -> 63, length 64 -> 64, length 100 -> 36, length 128 -> 64. Remember that the empty walk still has a segment.
    inline uint64_t lastSegmentLength() { 
        return getLength() - (64 * ((*steps()).size() - 1));
    }
    inline uint64_t getLength() { return var2.segments[0] & 0x00FFFFFFFFFFFFFFULL; }
    inline void setLength(uint64_t length) { var2.segments[0] = (var2.segments[0] & 0xFF00000000000000ULL) + length; }

    LongWalk horizontalStep() {
        LongWalk result = *this;
        uint64_t length = getLength();
        if(lastSegmentLength() == 64) {
            result.var1.segments.push_back(0);
        }
        result.var2 += 1;
        return result;
    }

    LongWalk verticalStep() {
        LongWalk result = *this;
        uint64_t length = getLength();
        int m = lastSegmentLength();
        if(m == 64) {
            result.steps()->push_back(1);
        } else {
            result.steps()->back() |= 1ULL << m;  
        }
        result.var2 += 1;
        return result;
    }

    void getEndPoint(int &x, int &y) {
        if(var1.segments.empty()) {
            x = 0;
            y = 0;
        } else {
            for(int i = 0; i < steps()->size() - 1; i++) {
                getEndPoint64((*steps())[i], 64, x, y);
            }
            getEndPoint64(steps()->back(), lastSegmentLength(), x, y);
        }
    }

    bool noLoop(int endX, int endY) {
        bool flag = true;
        int i = 0, x = 0, y = 0;
        while(flag && i < steps()->size() - 1) {
            flag = noLoop64((*steps())[i], 64, x, y, endX, endY);
            ++i;
        }
        return flag && noLoop64(steps()->back(), lastSegmentLength() - 12, x, y, endX, endY);
    }

    inline int approach() {
        return approach64(steps()->back(), lastSegmentLength());
    }

    void getBoundingBox(int &minX, int &maxX, int &minY, int &maxY) {
        // Step direction impacted by the starting point (Manhattan Lattice).
        int xStep = 1, yStep = 1;
        int x = 0, y = 0;
        uint64_t tempSteps = (*steps())[0];
        for(int i = 0; i < getLength(); ++i) {
            if(tempSteps & 1) {
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
            tempSteps >>= 1;
        }
    }

    // Flips the bits of a walk for the backwards extendability check. Assume there's only one step segment, since we only flip jails.
    void reverse() {
        uint64_t tempSteps = 0ULL;
        for(int i = 0; i < getLength(); i++) {
            tempSteps <<= 1;
            tempSteps |= ((*steps())[0] >> i) & 1;
        }
        (*steps())[0] = tempSteps;
    }

    bool noNarrow(int x3, int y3, int x4, int y4) {
        if(getLength() < 10) {
            return true;
        }
        int length = 0;
        int index = 0;
        int xStep = 1;
        int yStep = 1;
        int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
        while(length + 64 < getLength() - 10) {
            if(!noNarrowHelper((*steps())[index], 64, x1, y1, x2, y2, x3, y3, x4, y4)) {
                return false;
            }
            length += 64;
            index++;
        }
        // It's not possible to get within 1 step of yourself in 10 steps. The last 2 steps must be omited or they detect themselves.
        int m = (getLength() - 10) & 63;
        bool noNarrowFlag = noNarrowHelper((*steps())[index], m, x1, y1, x2, y2, x3, y3, x4, y4);
        // if(!noNarrowFlag) {
        //     std::cout << toBinary(toCheck->steps[0], toCheck->length) << std::endl;
        // }
        return noNarrowFlag;
    }


    // Can some valid walk go from the given point (one of the endpoints of the walk we're testing)
    // to the bounding box of the walk (minX to maxX, minY to maxY)? Or do all walks originating at the given point die out?
    // Returns 0 if not extendible (boxed), 1 if extendible (escapes bounding box), 2 if encountered the goal point.
    int extendible(int startX, int startY) {
        if(getLength() < 10) {
            return 2;
        }

        LongWalk jailBackup = *this;
        // Fips the walk. Start at origin only if called from reverse call or polygon, reversing a polygon does nothing.
        if(startX == 0 && startY == 0) {
            reverse();
            getEndPoint(startX, startY);
        }

        // std::cout << "Reversed: " << toBinary() << std::endl;

        int minX = 0, maxX = 0, minY = 0, maxY = 0;
        getBoundingBox(minX, maxX, minY, maxY);

        int firstTwoFlipped = (((*steps())[0] & 1ULL) << 1) | (((*steps())[0] >> 1) & 1ULL);
        int lastTwo = approach();

        // Can escape with a single step from the end of the jail, hopefully a good number fall into this case.
        if(canEscape(lastTwo, startX, startY, minX, maxX, minY, maxY)) {
            *this = jailBackup;
            return 1;
        }

        std::deque<LongWalk> escapes;
        escapes.push_back(*this);
        while(!escapes.empty()) {
            LongWalk *current = &escapes.front();
            // std::cout << "Escape: " << current->toBinary() << std::endl;
            int a = current->approach();
            int prevX = 0;
            int prevY = 0;
            current->getEndPoint(prevX, prevY);

            // Horizontal Step.
            if(a != 2) {
                LongWalk h = current->horizontalStep();
                int ha = h.approach();
                int x = 0;
                int y = 0;
                h.getEndPoint(x, y);
                bool noNarrowFlag = h.noNarrow(prevX, prevY, x, y);
                // First firstTwoFlipped checks that the last two agree with the first step. Second check for first 2 agreeing with last step (2-turn rule).
                if(x == 0 && y == 0 && ha != 2 - (firstTwoFlipped >> 1) && firstTwoFlipped != 2 && noNarrowFlag) {
                    *this = jailBackup;
                    // std::cout << toBinary(h.steps[0], h.length) << std::endl;
                    return 2;
                }
                else if(h.noLoop(x, y) && noNarrowFlag) {
                    if(canEscape(ha, x, y, minX, maxX, minY, maxY)) {
                        *this = jailBackup;
                        return 1;
                    } else {
                        escapes.push_back(h);
                    }
                }
            }

            // Vertical Step.
            if(a != 1) {
                LongWalk v = current->verticalStep();
                int va = v.approach();
                int x = 0;
                int y = 0;
                v.getEndPoint(x, y);
                bool noNarrowFlag = v.noNarrow(prevX, prevY, x, y);
                if(x == 0 && y == 0 && va != 2 - (firstTwoFlipped >> 1) && firstTwoFlipped != 1 && noNarrowFlag) {
                    *this = jailBackup;
                    // std::cout << toBinary(h.steps[0], h.length) << std::endl;
                    return 2;
                }
                else if(v.noLoop(x, y) && noNarrowFlag) {
                    if(canEscape(va, x, y, minX, maxX, minY, maxY)) {
                        *this = jailBackup;
                        return 1;
                    } else {
                        escapes.push_back(v);
                    }
                }
            }
            escapes.pop_front();
        }
        // All walks died, could not escape.
        *this = jailBackup;
        return 0;
    }

    void reduce(int endX, int endY, int n, std::vector<int> const &stepsToOrigin) {
        uint64_t tempSteps = (*steps())[0];
        uint64_t length = getLength();
        while(stepsToOrigin[approach64(tempSteps, length) * 40401 + (endX + 100) * 201 + endY + 100] > n - length) {
            if(tempSteps & 1) {
                endX = -endX;
                endY -= 1;
            }
            else {
                endX -= 1;
                endY = -endY;
            }

            tempSteps >>= 1;
            --length;

            if(tempSteps & 1) {
                tempSteps ^= (1ULL << length) - 1;
                int temp = endX;
                endX = endY;
                endY = temp;
            }
        }

        // If first step is vertical, flip to horizontal.
        if(tempSteps & 1) {
            tempSteps ^= (1ULL << length) - 1;
        }

        var1.segments[0] = tempSteps;
        setLength(length);
    }

    void removeStep() {
        if(getLength() > 0) {
            int lastLen = lastSegmentLength();
            if(lastLen == 1) {
                steps()->pop_back();
            } else {
                steps()->back() = steps()->back() & ~(1ULL << (lastLen - 1));
            }
            setLength(getLength() - 1);
        }
    }
};

struct State {
    LongWalk walk; // The walk (steps, length, parity), then the sum counts.
    uint64_t childIndex[2]{};

    // Zero as the default child index should be ok because no paths reduce to nothing.
    State(uint64_t steps, uint64_t length):
        walk(steps, length), childIndex{0,0} {}

    State(LongWalk walk):
        walk(walk), childIndex{0,0} {}
};

std::vector<int> getStepsToOrigin() {
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


std::vector<State> makeAutomaton(int n) {
    std::vector<int> stepsToOrigin = getStepsToOrigin();

    std::vector<State> states;
    states.reserve(91355100);
    // states.reserve(307000000);
    states.emplace_back(0, 0);

    uint64_t untreated = 0;

    while(untreated < states.size()) {
        int prevX = 0, prevY = 0;
        states[untreated].walk.getEndPoint(prevX, prevY);

        // Horizontal Step.
        if(states[untreated].walk.approach() != 2 || states[untreated].walk.getLength() < 2) {
            LongWalk h = states[untreated].walk.horizontalStep();
            int x = 0, y = 0;
            h.getEndPoint(x, y);
            if(h.noLoop(x, y) && h.noNarrow(prevX, prevY, x, y)) {
                int forward_extendable = h.extendible(x, y);
                if(forward_extendable == 2 || (forward_extendable && h.extendible(0, 0))) {
                    h.reduce(x, y, n, stepsToOrigin);
                    
                    State parent = states[0];
                    uint64_t tempSteps = (*h.steps())[0];
                    for(int i = 0; i < h.getLength() - 1; i++) {
                        parent = states[parent.childIndex[tempSteps & 1]];
                        tempSteps >>= 1;
                    }
                    if(!parent.childIndex[tempSteps]) {
                        states.emplace_back(h);
                        states[untreated].childIndex[0] = states.size() - 1;
                    } else {
                        states[untreated].childIndex[0] = parent.childIndex[tempSteps];
                    }
                }                
            }
        }

        // Vertical Step.
        if(states[untreated].walk.approach() != 1 || states[untreated].walk.getLength() < 2) {
            LongWalk v = states[untreated].walk.verticalStep();
            int x = 0, y = 0;
            v.getEndPoint(x, y);
            if(v.noLoop(x, y) && v.noNarrow(prevX, prevY, x, y)) {
                int forward_extendable = v.extendible(x, y);
                if(forward_extendable == 2 || (forward_extendable && v.extendible(0, 0))) {
                    v.reduce(x, y, n, stepsToOrigin);

                    State parent = states[0];
                    uint64_t tempSteps = (*v.steps())[0];
                    for(int i = 0; i < v.getLength() - 1; i++) {
                        parent = states[parent.childIndex[tempSteps & 1]];
                        tempSteps >>= 1;
                    }
                    if(!parent.childIndex[tempSteps]) {
                        states.emplace_back(v);
                        states[untreated].childIndex[1] = states.size() - 1;
                    } else {
                        states[untreated].childIndex[1] = parent.childIndex[tempSteps];
                    }
                }
            }
        }
        ++untreated;
    }



    // Reset each state's number variables so they're no longer pattern and length, instead automaton iteration counts.
    for(auto & state : states) {
        state.walk.var1 = 0;
        state.walk.var2 = 0;
    }
    return states;
}

BigSum taxi(int automaton_size, int num_iterations) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::vector<State> automaton = makeAutomaton(automaton_size);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    double totalTime = (double)(end - begin).count() / 1000000000.0;
    std::cout << automaton.size() << "-state automaton generated in " << totalTime << " seconds." << std::endl;

    // Sets "H" to current count 1.
    automaton[1].walk.var1 = 1;

    begin = std::chrono::steady_clock::now();

    // Ends one step early, because on the final loop there's no need to move var2 to var1.
    for(int n = 2; n < num_iterations; ++n) {
        for(auto & state : automaton) {
            if(state.walk.var1) {
                if(state.childIndex[0]) {
                    automaton[state.childIndex[0]].walk.var2 += state.walk.var1;
                }
                if(state.childIndex[1]) {
                    automaton[state.childIndex[1]].walk.var2 += state.walk.var1;
                }
            }
        }
        for(auto & state : automaton) {
            state.walk.var1 = state.walk.var2;
            state.walk.var2 = 0;
        }

        int i = 100;
        if((n + 1) % i == 0) {
            BigSum taxiWalks = 0;
            for(auto & state : automaton) {
                if(state.walk.var1) {
                    if(state.childIndex[0]) {
                        taxiWalks += state.walk.var1;
                    }
                    if(state.childIndex[1]) {
                        taxiWalks += state.walk.var1;
                    }
                }
            }
            end = std::chrono::steady_clock::now();
            double totalTime = (double)(end - begin).count() / 1000000000.0;
            std::cout << "Completed iteration " << n + 1 << " of " << num_iterations << ". This group of " << i << " took " << totalTime << " seconds." << std::endl;
            std::cout << "A=" << automaton_size << ", I=" << n + 1 << ": " << taxiWalks << '0' << std::endl;
            begin = end;
        }
    }
    // return 0;

    std::cout << "Computing final sum..." << std::endl;

    BigSum taxiWalks = 0;
    for(auto & state : automaton) {
        if(state.walk.var1) {
            if(state.childIndex[0]) {
                taxiWalks += state.walk.var1;
            }
            if(state.childIndex[1]) {
                taxiWalks += state.walk.var1;
            }
        }
    }
    return taxiWalks;
}


uint64_t bruteForce(int N)
{
    std::vector<LongWalk> walks;
    // H start.
    walks.emplace_back(0, 1);
    // uint64_t forward2Count = 0;
    uint64_t count = 0;

    // std::ofstream f;
    // f.open("bigSum.txt");

    while(!walks.empty()) {
        LongWalk current = walks.back();
        // std::cout << current.toBinary() << std::endl;
        walks.pop_back();
        int prevX = 0, prevY = 0;
        current.getEndPoint(prevX, prevY);

        // Horizontal Step.
        if(current.approach() != 2 || current.getLength() < 2) {
            int x = 0, y = 0;
            LongWalk h = current.horizontalStep();
            h.getEndPoint(x, y);
            bool noNarrowFlag = h.noNarrow(prevX, prevY, x, y);
            if(h.noLoop(x, y) && noNarrowFlag) {
                int forward_extendible = h.extendible(x, y);
                // if(forward_extendible == 2) {
                //     forward2Count++;
                // }
                if(forward_extendible == 2 || (forward_extendible && h.extendible(0, 0))) {
                    if(h.getLength() != N) {
                        walks.push_back(h);
                    } else {
                        count++;
                        // f << h.toBinary() << std::endl;
                    }
                }
            }
        }

        // Vertical Step.
        if(current.approach() != 1 || current.getLength() < 2) {
            int x = 0, y = 0;
            LongWalk v = current.verticalStep();
            v.getEndPoint(x, y);
            bool noNarrowFlag = v.noNarrow(prevX, prevY, x, y);
            if(v.noLoop(x, y) && noNarrowFlag) {
                int forward_extendible = v.extendible(x, y);
                // if(forward_extendible == 2) {
                //     forward2Count++;
                // }
                if(forward_extendible == 2 || (forward_extendible && v.extendible(0, 0))) {
                    if(v.getLength() != N) {
                        walks.push_back(v);
                    } else {
                        count++;
                        // f << v.toBinary() << std::endl;
                    }
                }
            }
        }
    }

    // f.close();
    // std::cout << forward2Count << std::endl;

    return count << 1;
}

void run(int automaton_size, int num_iterations) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    if(automaton_size > 0) {
        BigSum t = taxi(automaton_size, num_iterations);
        // << '0' << is to "multiply by 2". Only works for binary outputs.
        std::cout << "A=" << automaton_size << ", I=" << num_iterations << ": " << t << '0' << std::endl;
    } else {
        std::cout << num_iterations << ": " << bruteForce(num_iterations) << std::endl;
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    double totalTime = (double)(end - begin).count() / 1000000000.0;
    std::cout << "Total Time: " << totalTime << std::endl;
}



int main(int argc, char *argv[]) {
    if(argc == 2) {
        int N = atoi(argv[1]);
        run(N, N);
    } else if(argc == 3) {
        int automaton_size = atoi(argv[1]);
        int num_iterations = atoi(argv[2]);
        run(automaton_size, num_iterations);
    }
    return 0;



    // LongWalk walk(0b000000001111111110011000000011111110, 36);
    // int x = 0, y = 0;
    // walk.getEndPoint(x, y);
    // std::cout << walk.extendible(0, 0) << std::endl;
}