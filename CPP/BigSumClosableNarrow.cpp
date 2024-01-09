#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <climits>
#include <deque>
#include <algorithm>

std::string toBinary(uint64_t steps, uint16_t length)
{
    if(length == 0) {
        return "Origin";
    }
    std::string binary;
    for(uint16_t i = 0; i < length; i++)
    {
        if((steps >> (length - 1 - i)) & 1)
        {
            binary += "1";
        }
        else
        {
            binary += "0";
        }
    }
    return binary;
}

void getEndPoint64(uint64_t steps, uint16_t length, int &x, int &y) {
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
    return (x == minX && (y & 1) || x == maxX && !(y & 1)) && (approach != 2)
            || (y == minY && (x & 1) || y == maxY && !(x & 1)) && (approach != 1);
}

// Returns the parity of this section.
int narrow64(uint64_t steps, uint16_t length, int &x1, int &y1, int &x2, int &y2, int x3, int y3, int x4, int y4) {
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
            // The parity of the number of steps taken gives the parity of the point (x1, y1).
            return i & 1;
        }
    }
    return -1;
}




// Bits go right to left, segments go left to right.
const uint64_t max_bit = 1ULL << 63;
struct BigSum {
    std::vector<uint64_t> segments;
    
    BigSum(uint64_t value = 0) {
        segments.push_back(value);
        if(segments[0] & max_bit) {
            segments[0] &= ~max_bit;
            segments.push_back(1);
            std::cout << "BigSum created with carry." << std::endl;
        }
    }

    void operator=(const BigSum &other) {
		segments = other.segments;
	}
 
	void operator=(uint64_t value) {
		segments.clear();
		segments.push_back(value);
        if(segments[0] & max_bit) {
            segments[0] &= ~max_bit;
            segments.push_back(1);
            std::cout << "BigSum assigned with carry." << std::endl;
        }
	}

    BigSum operator+(const BigSum &other) const {
        BigSum result = other;
        for(int i = 0, carry = 0; i < std::max(segments.size(), other.segments.size()) || carry; ++i) {
            if(i == result.segments.size()) {
                result.segments.push_back(0);
            }
            uint64_t previous_value = result.segments[i];
            result.segments[i] += carry + (i < segments.size() ? segments[i] : 0);
            carry = result.segments[i] < previous_value;
        }
        return result;
	}

    void operator+=(const BigSum &other) {
		*this = *this + other;
	}

    operator bool() {
        return segments.size() > 1 || (segments.size() == 1 && segments[0] != 0ULL);
    }

    std::string to_string() const {
        if(segments.empty()) {
            return "Empty!";
        } else {
            std::string out = "";
            for(int i = segments.size() - 1; i >= 0; --i) {
			    out += toBinary(segments[i], 64);
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

    friend std::ostream& operator<<(std::ostream &stream, const BigSum &bigsum) {
        stream << bigsum.to_string();
        return stream;
	}
};


struct LongWalk {
    BigSum var1; // Steps and count1.
    BigSum var2; // Length/narrowCount/parity and count2. Parity uses leftmost bit, narrowCount uses the next 7 bits, length uses rightmost 56 bits. Never longer than 1 u64 in the walk phase.

    // No steps, they will get added either with = or + later.
    LongWalk() {
        var1 = 0ULL;
        var2 = 0ULL;
    }

    LongWalk(uint64_t steps, uint64_t length, uint64_t parity, uint64_t narrowCount):
        var1(steps) {
            var2 = (parity << 63) + (narrowCount << 56) + length;
        }

    void operator=(const LongWalk &other) {
		var1 = other.var1;
        var2 = other.var2;
	}

    // Bitpacking convenience functions for var2.
    inline std::vector<uint64_t> *steps() { return &var1.segments; }
    inline uint64_t lazyLength() { return var2.segments[0]; } // Only use when immediately & 63 or some other small number.
    inline uint64_t getLength() { return var2.segments[0] & 0x00FFFFFFFFFFFFFFULL; }
    inline uint8_t getNarrowCount() { return (var2.segments[0] >> 56) & 0x7F; }
    inline uint8_t getParity() { return var2.segments[0] >> 63; }
    inline void setLength(uint64_t length) { var2.segments[0] = (var2.segments[0] & 0xFF00000000000000ULL) + length; }
    inline void setNarrowCount(uint64_t narrowCount) { var2.segments[0] = (var2.segments[0] & 0x80FFFFFFFFFFFFFFULL) + (narrowCount << 56); }
    inline void setParity(uint64_t parity) { var2.segments[0] = (parity << 63) + (var2.segments[0] & 0x7FFFFFFFFFFFFFFFULL); }

    LongWalk horizontalStep() {
        LongWalk result = *this;
        uint64_t length = getLength();
        if(length && (length & 63) == 0) {
            result.var1.segments.push_back(0);
        }
        result.var2 += 1;
        return result;
    }

    LongWalk verticalStep() {
        LongWalk result = *this;
        uint64_t length = getLength();
        int m = length & 63;
        // std::cout << "V length: " << length << " " << m << std::endl;
        // std::cout << result.steps()->size() << std::endl;
        if(length && !m) {
            result.steps()->push_back(1);
        } else {
            result.steps()->back() |= 1ULL << m;  
        }
        result.var2 += 1;
        // std::cout << "V step: " << (*result.steps())[0] << " " << result.getLength() << " " << toBinary((*result.steps())[0], result.getLength()) << std::endl;
        return result;
    }

    void getEndPoint(int &x, int &y) {
        if(var1.segments.empty()) {
            x = 0;
            y = 0;
        } else {
            // std::cout << steps()->size() << std::endl;
            for(int i = 0; i < steps()->size() - 1; i++) {
                // std::cout << "More endpoint calls" << std::endl;
                getEndPoint64((*steps())[i], 64, x, y);
            }
            // std::cout << steps()->back() << " " << (lazyLength() & 63) << " " << x << "," << y << std::endl;
            getEndPoint64(steps()->back(), lazyLength() & 63, x, y);
        }
    }

    bool noLoop(int endX, int endY) {
        bool flag = true;
        int i = 0, x = 0, y = 0;
        while(flag && i < steps()->size() - 1) {
            flag = noLoop64((*steps())[i], 64, x, y, endX, endY);
            ++i;
        }
        return flag && noLoop64(steps()->back(), (lazyLength() & 63) - 12, x, y, endX, endY);
    }

    inline int approach() {
        return approach64(steps()->back(), lazyLength() & 63);
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

    // 0 for none found, 1 for odd start vertex, 2 for even start vertex (the vertex closest to the start of the walk).
    // At this level, return the first one. In the rare case there are several per x3y3 x4y4, they will still be caught.
    int narrowParityEnd(int x3, int y3, int x4, int y4) {
        if(getLength() < 10) {
            return -1;
        }
        int length = 0;
        int index = 0;
        int xStep = 1;
        int yStep = 1;
        int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
        while(length + 64 < getLength() - 10) {
            int parity = narrow64((*steps())[index], 64, x1, y1, x2, y2, x3, y3, x4, y4);
            if(parity != -1) {
                return parity;
            }
            length += 64;
            index++;
        }
        // It's not possible to get within 1 step of yourself in 10 steps. The last 2 steps must be omited or they detect themselves.
        int m = (getLength() - 10) & 63;
        return narrow64((*steps())[index], m, x1, y1, x2, y2, x3, y3, x4, y4);
    }

    // For when a single step is added to the end of a walk whose parity is known.
    bool consistentNarrow(int x3, int y3, int x4, int y4) {
        int newParity = narrowParityEnd(x3, y3, x4, y4);
        if(newParity != -1) {
            if(getNarrowCount() && getParity() != newParity) {
                return false;
            } else {
                setParity(newParity);
                setNarrowCount(getNarrowCount() + 1);
            }
        }
        return true;
    }

    // For determining whether to decrement the narrowCount while reducing (cutting steps off the front of a walk).
    // Reduce is only used on jails, so walks of length < 64 (single segment assumption).
    bool firstStepNarrow() {
        if(getLength() < 10) {
            return false;
        }
        uint64_t tempSteps = (*steps())[0];
        int x1 = 0, y1 = 0, x2 = !(tempSteps & 1), y2 = !x2;
        tempSteps >>= 1;
        int xStep = 1 - ((y2 & 1) << 1);
        int yStep = 1 - ((x2 & 1) << 1);
        int x3 = x2, y3 = y2;
        if(tempSteps & 1) {
            y3 += yStep;
            xStep = -xStep;
        } else {
            x3 += xStep;
            yStep = -yStep;
        }
        tempSteps >>= 1;
        int x4 = x3, y4 = y3;

        for(int i = 2; i < getLength(); i++) {
            x3 = x4;
            y3 = y4;
            if(tempSteps & 1) {
                y4 += yStep;
                xStep = -xStep;
            } else {
                x4 += xStep;
                yStep = -yStep;
            }
            tempSteps >>= 1;

            if(abs(x4 - x1) + abs(y4 - y1) == 1 && abs(x3 - x2) + abs(y3 - y2) == 1) {
                // The parity of the number of steps taken gives the parity of the point (x1, y1).
                return true;
            }
        }
        return false;
    }

    // Narrow structures must be created by unoccupied odd vertices. Therefore, they must be outside the final contour.
    // Odd v1 -> must be closed clockwise (more right turns). Even v1 -> must be closed counterclockwise (more left turns).
    bool narrowExcluded() {
        if(!getNarrowCount()) {
            // No narrow.
            return true;
        }

        int leftTurns = 0;
        int rightTurns = 0;
        bool up = true;
        bool right = true;
        int prevStep = -1;
        for(int i = 0; i < steps()->size() - 1; i++) {
            uint64_t tempSteps = (*steps())[i];
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
        int m = lazyLength() & 63;
        uint64_t tempSteps = steps()->back();
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
        int step = (*steps())[0] & 1;
        bool diff = up ^ right;
        rightTurns += (!prevStep && step && diff) || ((prevStep == 1) && !step && !diff);
        leftTurns += (!prevStep && step && !diff) || ((prevStep == 1) && !step && diff);

        // Parity 1 -> v1 odd and must be clockwise, parity 0 -> v1 even must be counterclockwise.
        return getParity() == (rightTurns > leftTurns);
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


    // Can some valid walk go from the given point (one of the endpoints of the walk we're testing)
    // to the bounding box of the walk (minX to maxX, minY to maxY)? Or do all walks originating at
    // the given point die out?
    // Returns 0 if not extendable (boxed), 1 if extendable (escapes bounding box), 2 if encountered the goal point.
    int extendable(int startX, int startY) {
        if(getLength() < 10) {
            return 2;
        }

        LongWalk jailBackup = *this;
        // Fips the walk. Start at origin only if called from reverse call or polygon, reversing a polygon does nothing.
        if(startX == 0 && startY == 0) {
            // Jail is never longer than 64 steps.
            // CAN I SET STEPS LIKE THIS?
            reverse();
            if(getNarrowCount() && (lazyLength() & 1) == 0) {
                setParity(!getParity());
            }
            getEndPoint(startX, startY);
        }

        int minX = 0, maxX = 0, minY = 0, maxY = 0;
        getBoundingBox(minX, maxX, minY, maxY);

        int firstTwoFlipped = (((*steps())[0] & 1) << 1) | (((*steps())[0] >> 1) & 1);
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
                bool consistent = h.consistentNarrow(prevX, prevY, x, y);
                if(x == 0 && y == 0 && ha != 2 - (firstTwoFlipped >> 1) && firstTwoFlipped != 2 && consistent && h.narrowExcluded()) {
                    *this = jailBackup;
                    return 2;
                }
                else if(h.noLoop(x, y) && consistent) {
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
                bool consistent = v.consistentNarrow(prevX, prevY, x, y);
                if(x == 0 && y == 0 && va != 2 - (firstTwoFlipped >> 1) && firstTwoFlipped != 1 && consistent && v.narrowExcluded()) {
                    *this = jailBackup;
                    return 2;
                }
                else if(v.noLoop(x, y) && consistent) {
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
        uint64_t parity = getParity();

        // std::cout << "Reducing: " << toBinary(tempSteps, length) << " with endpoint: " << endX << ", " << endY << std::endl;

        while(stepsToOrigin[approach64(tempSteps, length) * 40401 + (endX + 100) * 201 + endY + 100] > n - length) {
            // About to remove a narrow.
            if(firstStepNarrow()) {
                setNarrowCount(getNarrowCount() - 1);
            }
            
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
            parity = !parity; // Parity flips when you remove a step from the beginning.

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
        setParity(parity);
    }

    std::string to_string() const {
        if(var1.segments.empty()) {
            return "Empty!";
        } else {
            std::string out = "";
            for(int i = var1.segments.size() - 1; i >= 0; --i) {
			    out += toBinary(var1.segments[i], 64);
            }

            // Avoid leading 0s.
            int i = 0;
            while(i < out.length() && out[i] == '0') {
                ++i;
            }
            return out.substr(i);
        }
    }

    friend std::ostream& operator<<(std::ostream &stream, const LongWalk &longwalk) {
        stream << longwalk.to_string();
        return stream;
	}
};

struct State {
    LongWalk walk; // The walk (steps, length, parity), then the sum counts.
    State *children[2]{};

    State(uint64_t steps, uint64_t length, bool parity, uint8_t narrowCount):
        walk(steps, length, parity, narrowCount), children{nullptr,nullptr} {}

    State(LongWalk walk):
        walk(walk), children{nullptr,nullptr} {}
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
    states.reserve(91355000);
    states.emplace_back(0, 0, 0, 0);

    uint64_t untreated = 0;

    while(untreated < states.size()) {
        // std::cout << toBinary((*states[untreated].walk.steps())[0], states[untreated].walk.getLength()) << std::endl;
        // Horizontal Step.
        int prevX = 0, prevY = 0;
        states[untreated].walk.getEndPoint(prevX, prevY);
        if(states[untreated].walk.approach() != 2 || states[untreated].walk.getLength() < 2) {
            // uint16_t length = states[untreated].walk.getLength() + 1;
            // uint64_t steps = states[untreated].walk.steps()[0];
            LongWalk h = states[untreated].walk.horizontalStep();
            // std::cout << "H: " << toBinary((*h.steps())[0], h.getLength()) << std::endl;

            int x = 0, y = 0;
            h.getEndPoint(x, y);
            if(h.noLoop(x, y)) {
                // std::cout << "No Loop" << std::endl;
                h.reduce(x, y, n, stepsToOrigin);
                // std::cout << "After Reduce: " << toBinary((*h.steps())[0], h.getLength()) << std::endl;
                // std::cout << "here 1" << std::endl;
                
                State parent = states[0];
                uint64_t tempSteps = (*h.steps())[0];
                for(int i = 0; i < h.getLength() - 1; ++i) {
                    // std::cout << toBinary((*parent.walk.steps())[0], parent.walk.getLength()) << std::endl;
                    parent = *parent.children[tempSteps & 1];
                    tempSteps >>= 1;
                }

                // std::cout << "here 3" << std::endl;

                if(!parent.children[tempSteps]) {
                    // std::cout << "Added" << std::endl;
                    states.emplace_back(h);
                    states[untreated].children[0] = &states[states.size() - 1];
                } else {
                    states[untreated].children[0] = parent.children[tempSteps];
                }
            }
        }

        // Vertical Step.
        if(states[untreated].walk.approach() != 1 || states[untreated].walk.getLength() < 2) {
            // uint16_t length = states[untreated].walk.getLength() + 1;
            // uint64_t steps = states[untreated].walk.steps()[0] | (1ULL << states[untreated].walk.getLength());
            LongWalk v = states[untreated].walk.verticalStep();
            // std::cout << "V: " << toBinary((*v.steps())[0], v.getLength()) << std::endl;
            int x = 0, y = 0;
            v.getEndPoint(x, y);
            if(v.noLoop(x, y)) { // && v.consistentNarrow(prevX, prevY, x, y)
                // std::cout << "No Loop" << std::endl;
                v.reduce(x, y, n, stepsToOrigin);
                // std::cout << "After Reduce: " << toBinary((*v.steps())[0], v.getLength()) << std::endl;

                State parent = states[0];
                uint64_t tempSteps = (*v.steps())[0];
                for(int i = 0; i < v.getLength() - 1; i++) {
                    parent = *parent.children[tempSteps & 1];
                    tempSteps >>= 1;
                }
                if(!parent.children[tempSteps]) {
                    // std::cout << "Added" << std::endl;
                    states.emplace_back(v);
                    states[untreated].children[1] = &states[states.size() - 1];
                } else {
                    states[untreated].children[1] = parent.children[tempSteps];
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
    std::vector<State> automaton = makeAutomaton(automaton_size);

    std::cout << "Automaton Generated." << std::endl;

    // Sets "H" to current count 1.
    automaton[1].walk.var1 = 1;

    // Ends one step early, because on the final loop there's no need to move var2 to var1.
    for(int n = 2; n < num_iterations; ++n) {
        for(auto & state : automaton) {
            if(state.walk.var1) {
                if(state.children[0]) {
                    (*state.children[0]).walk.var2 += state.walk.var1;
                }
                if(state.children[1]) {
                    (*state.children[1]).walk.var2 += state.walk.var1;
                }
            }
        }
        for(auto & state : automaton) {
            state.walk.var1 = state.walk.var2;
            state.walk.var2 = 0;
        }

        // if((n + 1) % 50 == 0) {
            // std::cout << "Completed iteration " << n + 1 << " of " << num_iterations << "." << std::endl;
            // BigSum taxiWalks = 0;
            // for(auto & state : automaton)
            // {
            //     if(state.var1)
            //     {
            //         if(state.children[0])
            //         {
            //             taxiWalks += state.var1;
            //         }
            //         if(state.children[1])
            //         {
            //             taxiWalks += state.var1;
            //         }
            //     }
            // }
            // std::cout << "A=" << automaton_size << ", I=" << n + 1 << ": " << taxiWalks << '0' << std::endl;
        // }
    }
    // return 0;

    std::cout << "Computing final sum..." << std::endl;

    BigSum taxiWalks = 0;
    for(auto & state : automaton) {
        if(state.walk.var1) {
            if(state.children[0]) {
                taxiWalks += state.walk.var1;
            }
            if(state.children[1]) {
                taxiWalks += state.walk.var1;
            }
        }
    }
    return taxiWalks;
}

void run(int automaton_size, int num_iterations) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    BigSum t = taxi(automaton_size, num_iterations);

    // << '0' << is to "multiply by 2". Only works for binary outputs.
    std::cout << "A=" << automaton_size << ", I=" << num_iterations << ": " << t << '0' << std::endl;

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    double totalTime = (double)(end - begin).count() / 1000000000.0;
    std::cout << "Total Time: " << totalTime << std::endl;
}

int main(int argc, char *argv[]) {
    // Automaton size must not be more than 63 because the automaton generator still uses only 64 bits of each state's variables.
    if(argc == 2) {
        int N = atoi(argv[1]);
        run(N, N);
    } else if(argc == 3) {
        int automaton_size = atoi(argv[1]);
        int num_iterations = atoi(argv[2]);
        run(automaton_size, num_iterations);
    }
    return 0;
}