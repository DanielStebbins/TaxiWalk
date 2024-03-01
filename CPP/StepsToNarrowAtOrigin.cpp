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
    int xStep = 1 - (!(y & 1) << 1);
    int yStep = 1 - (!(x & 1) << 1);
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
    int xStep = 1 - (!(y & 1) << 1);
    int yStep = 1 - (!(x & 1) << 1);
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
    int xStep = 1 - (!(y1 & 1) << 1);
    int yStep = 1 - (!(x1 & 1) << 1);
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

    std::string to_string() const {
        if(segments.empty()) {
            return "Empty!";
        } else {
            std::string out = "";
            for(int i = segments.size() - 1; i >= 0; --i) {
			    out += toBinary64(segments[i], 64);
            }

            // Avoid leading 0s.
            int i = 0;
            while(i < out.length() && out[i] == '0') {
                ++i;
            }
            return out.substr(i);
        }
    }

    friend std::ostream& operator<<(std::ostream &stream, const BigSum &bigsum) {
        stream << bigsum.to_string();
        return stream;
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
        if(!var1.segments.empty()) {
            for(int i = 0; i < steps()->size() - 1; i++) {
                getEndPoint64((*steps())[i], 64, x, y);
            }
            getEndPoint64(steps()->back(), lastSegmentLength(), x, y);
        }
    }

    bool noLoop(int startX, int startY, int endX, int endY) {
        bool flag = true;
        int i = 0, x = startX, y = startY;
        while(flag && i < steps()->size() - 1) {
            flag = noLoop64((*steps())[i], 64, x, y, endX, endY);
            ++i;
        }
        return flag && noLoop64(steps()->back(), lastSegmentLength() - 12, x, y, endX, endY);
    }

    inline int approach() {
        if(steps()->size() == 1 || lastSegmentLength() >= 2) {
            return approach64(steps()->back(), lastSegmentLength());
        } else {
            return ((steps()->back() & 1) << 1) + (((*steps())[steps()->size() - 2] >> 63) & 1);
        }
    }

    void getBoundingBox(int startX, int startY, int &minX, int &maxX, int &minY, int &maxY) {
        // Step direction impacted by the starting point (Manhattan Lattice).
        int xStep = 1 - (!(startY & 1) << 1);
        int yStep = 1 - (!(startX & 1) << 1);
        int x = startX, y = startY;
        minX = startX;
        maxX = startX;
        minY = startY;
        maxY = startY;
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

    bool noNarrow(int startX, int startY, int x3, int y3, int x4, int y4) {
        if(getLength() < 10) {
            return true;
        }
        int length = 0;
        int index = 0;
        int xStep = 1;
        int yStep = 1;
        int x1 = startX, y1 = startY, x2 = startX, y2 = startY;
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
    // Returns 0 if not extendable (boxed), 1 if extendable (escapes bounding box), 2 if encountered the goal point.
    int extendable(int jailStartX, int jailStartY, int escapeStartX, int escapeStartY) {
        if(getLength() < 10) {
            return 2;
        }

        LongWalk jailBackup = *this;
        // Fips the walk. Start at origin only if called from reverse call or polygon, reversing a polygon does nothing.
        if(jailStartX == escapeStartX && jailStartY == escapeStartY) {
            reverse();
            getEndPoint(escapeStartX, escapeStartY);
        }

        // std::cout << "Reversed: " << toBinary() << std::endl;

        int minX = 0, maxX = 0, minY = 0, maxY = 0;
        getBoundingBox(jailStartX, jailStartY, minX, maxX, minY, maxY);

        int firstTwoFlipped = (((*steps())[0] & 1ULL) << 1) | (((*steps())[0] >> 1) & 1ULL);
        int lastTwo = approach();

        // Can escape with a single step from the end of the jail, hopefully a good number fall into this case.
        if(canEscape(lastTwo, escapeStartX, escapeStartY, minX, maxX, minY, maxY)) {
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
                bool noNarrowFlag = h.noNarrow(jailStartX, jailStartY, prevX, prevY, x, y);
                // First firstTwoFlipped checks that the last two agree with the first step. Second check for first 2 agreeing with last step (2-turn rule).
                if(x == 0 && y == 0 && ha != 2 - (firstTwoFlipped >> 1) && firstTwoFlipped != 2 && noNarrowFlag) {
                    *this = jailBackup;
                    // std::cout << toBinary(h.steps[0], h.length) << std::endl;
                    return 2;
                }
                else if(h.noLoop(jailStartX, jailStartY, x, y) && noNarrowFlag) {
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
                bool noNarrowFlag = v.noNarrow(jailStartX, jailStartY, prevX, prevY, x, y);
                if(x == 0 && y == 0 && va != 2 - (firstTwoFlipped >> 1) && firstTwoFlipped != 1 && noNarrowFlag) {
                    *this = jailBackup;
                    // std::cout << toBinary(h.steps[0], h.length) << std::endl;
                    return 2;
                }
                else if(v.noLoop(jailStartX, jailStartY, x, y) && noNarrowFlag) {
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
};

void floodFill(int startX, int startY, std::vector<int> &steps) {
    std::cout << "Starting with " << startX << ", " << startY << std::endl;
    int seen = 0;
    std::deque<LongWalk> walks;
    // Empty walk to start.
    walks.emplace_back(0, 0);

    steps[0 * 40401 + (startX + 100) * 201 + startY + 100] = 0;
    steps[1 * 40401 + (startX + 100) * 201 + startY + 100] = 0;
    steps[2 * 40401 + (startX + 100) * 201 + startY + 100] = 0;
    steps[3 * 40401 + (startX + 100) * 201 + startY + 100] = 0;

    while(seen < 161600 && !walks.empty()) {
        LongWalk current = walks.front();
        // if(current.getLength() < 10) {
        //     std::cout << current.toBinary() << std::endl;
        // }
        // if(startX == 0 && startY == 1) {
        //     std::cout << seen << " " << current.getLength() << " " << walks.size() << std::endl;
        // }

        if(seen > 10) {
            // std::cout << seen << " " << walks.size() << " " << current.toBinary() << std::endl;
            if(walks.size() == 1) {
                for(int a = 0; a <= 3; a++) {
                    for(int x = -100; x <= 100; x++) {
                        for(int y = -100; y <= 100; y++) {
                            if(steps[a * 40401 + (x + 100) * 201 + y + 100] == INT_MAX && abs(x) < 98 && abs(y) < 98) {
                                std::cout << a << ":(" << x << "," << y << ") ";
                            }
                        }
                    }
                }
                std::cout << std::endl;
            }
        }
        walks.pop_front();
        int prevX = startX, prevY = startY;
        current.getEndPoint(prevX, prevY);

        // Horizontal Step.
        if(current.approach() != 2 || current.getLength() < 2) {
            int x = startX, y = startY;
            LongWalk h = current.horizontalStep();
            // if(seen == 157780) {
            //     std::cout << "H: " << h.toBinary() << std::endl;
            // }
            
            h.getEndPoint(x, y);
            if(current.steps()->back() == 0b1110001111111111111111111111111111000000000000000000000000001110 && current.getLength() == 64) {
                std::cout << "H: " << h.toBinary() << " " << x << " " << y << std::endl;
            }
            if(x == -5 && y == 2) {
                std::cout << h.approach() << " " << h.toBinary() << std::endl;
            }
            if(-100 <= x && x <= 100 && -100 <= y && y <= 100) {
                bool noNarrowFlag = h.noNarrow(startX, startY, prevX, prevY, x, y);
                if(current.steps()->back() == 0b1110001111111111111111111111111111000000000000000000000000001110 && current.getLength() == 64) {
                    std::cout << "Narrow? " << (1 - (int) noNarrowFlag) << std::endl;
                }
                if(steps[h.approach() * 40401 + (x + 100) * 201 + y + 100] > h.getLength()) {
                    if(steps[h.approach() * 40401 + (x + 100) * 201 + y + 100] == INT_MAX) {
                        seen++;
                    }
                    walks.emplace_back(h);
                    steps[h.approach() * 40401 + (x + 100) * 201 + y + 100] = h.getLength();
                    if(x == -5 && y == 2) {
                        std::cout << "Accepted " << h.approach() << " " << h.toBinary() << std::endl;
                    }
                }
            }
        }

        // Vertical Step.
        if(current.approach() != 1 || current.getLength() < 2) {
            int x = startX, y = startY;
            LongWalk v = current.verticalStep();
            // if(seen == 157780) {
            //     std::cout << "V: " << v.toBinary() << std::endl;
            // }
            v.getEndPoint(x, y);
            if(current.steps()->back() == 0b1110001111111111111111111111111111000000000000000000000000001110 && current.getLength() == 64) {
                std::cout << "V: " << v.toBinary() << " " << x << " " << y << std::endl;
            }
            if(x == -5 && y == 2) {
                std::cout << v.approach() << " " << v.toBinary() << std::endl;
            }
            if(-100 <= x && x <= 100 && -100 <= y && y <= 100) {
                bool noNarrowFlag = v.noNarrow(startX, startY, prevX, prevY, x, y);
                if(current.steps()->back() == 0b1110001111111111111111111111111111000000000000000000000000001110 && current.getLength() == 64) {
                    std::cout << "Narrow? " << (1 - (int) noNarrowFlag) << std::endl;
                }
                if(steps[v.approach() * 40401 + (x + 100) * 201 + y + 100] > v.getLength()) {
                    if(steps[v.approach() * 40401 + (x + 100) * 201 + y + 100] == INT_MAX) {
                        seen++;
                    }
                    steps[v.approach() * 40401 + (x + 100) * 201 + y + 100] = v.getLength();
                    walks.emplace_back(v);
                    if(x == -5 && y == 2) {
                        std::cout << "Accepted " << v.approach() << " " << v.toBinary() << std::endl;
                    }
                }
            }
        }
    }
}

// TODO: APPROACH BASED ON THE WALK THAT WOULD END AT A GIVEN POINT, NOT THE PATH TO GET THERE FROM THE ORIGIN.

// HH -> 00 (0)
// HV -> 10 (2)
// VH -> 01 (1)
// VV -> 11 (3)
int main(int argc, char *argv[]) {
    std::vector<int> steps(161604, INT_MAX);

    // If you can reach any of these points, you have value (you may produce a narrow before the "closable" check) and should not be reduced.
    // We ignore the possibility of narrows at (1,0) and (-1,0) because we only enumerate walks that start H.
    floodFill(0, 0, steps);
    floodFill(0, 1, steps);
    floodFill(0, -1, steps);

    std::ofstream file("StepsToNarrowAtOrigin.txt");
    for(const auto &step : steps) {
        file << step << " ";
    }
    file.close();
    std::cout << "Done!" << std::endl;
    return 0;
}