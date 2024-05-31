#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <deque>

struct Walk {
    uint64_t steps;
    uint16_t length;

    // No steps, they will get added either with = or + later.
    Walk() {
        steps = 0ULL;
        length = 0ULL;
    }

    Walk(uint64_t steps, uint64_t length):
        steps(steps), length(length) {}

    void operator=(const Walk &other) {
		steps = other.steps;
        length = other.length;
	}

    // Each segment is right to left, but the segments stack left to right (last steps are in the leftmost bits of the rightmost u64).
    const std::string toBinary() {
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

    Walk horizontalStep() {
        Walk result = *this;
        result.length += 1;
        return result;
    }

    Walk verticalStep() {
        Walk result = *this;
        result.steps |= 1ULL << length;  
        result.length += 1;
        return result;
    }

    void getEndPoint(int &x, int &y) {
        int xStep = 1;
        int yStep = 1;
        uint64_t tempSteps = steps;
        for(int i = 0; i < length; i++) {
            if(tempSteps & 1) {
                y += yStep;
                xStep = -xStep;
            } else {
                x += xStep;
                yStep = -yStep;
            }
            tempSteps >>= 1;
        }
    }

    bool noLoop(int endX, int endY) {
        int x = 0, y = 0;
        int xStep = 1, yStep = 1;
        bool noLoop = x != endX || y != endY;
        int i = 0;
        uint64_t tempSteps = steps;
        while(noLoop && i < length - 12) {
            if(tempSteps & 1) {
                y += yStep;
                xStep = -xStep;
            } else {
                x += xStep;
                yStep = -yStep;
            }
            noLoop = x != endX || y != endY;
            tempSteps >>= 1;
            ++i;
        }
        return noLoop;
    }

    // HH -> 00 (0)
    // HV -> 10 (2)
    // VH -> 01 (1)
    // VV -> 11 (3)
    inline int approach() {
        return (steps >> (length - 2)) & 3;
    }

    bool noNarrow(int x3, int y3, int x4, int y4) {
        if(length < 10) {
            return true;
        }
        int x1 = 0, y1 = 0;
        int x2 = 0, y2 = 0;
        int xStep = 1, yStep = 1;
        uint64_t tempSteps = steps;
        for(int i = 0; i < length - 10; i++) {
            x1 = x2;
            y1 = y2;
            if(tempSteps & 1) {
                y2 += yStep;
                xStep = -xStep;
            } else {
                x2 += xStep;
                yStep = -yStep;
            }
            tempSteps >>= 1;

            if(abs(x4 - x1) + abs(y4 - y1) == 1 && abs(x3 - x2) + abs(y3 - y2) == 1) {
                return false;
            }
        }
        return true;
    }

    // Returns true if the walk is closed clockwise, false otherwise. Assumes the walk is closed.
    bool clockwise() {
        int xStep = 1, yStep = 1;
        int left = 0, right = 0;
        bool prev = (steps >> (length - 1)) & 1;
        uint64_t tempSteps = steps;
        for(int i = 0; i < length; i++) {
            bool step = tempSteps & 1;
            if(prev != step) { // Turn.
                if(prev ^ (xStep > 0) ^ (yStep > 0)) { // Up -> Right is a right turn. Flipping step signs changes it; so does starting horizontal.
                    right++;
                } else {
                    left++;
                }
            }
            if(step) {
                xStep = -xStep;
            } else {
                yStep = -yStep;
            }
            tempSteps >>= 1;
            prev = step;
        }
        if(right != left + 4 && left != right + 4) {
            std::cout << "Clockwise Error! " << toBinary() << " has " << left << " left turns and " << right << " right turns." << std::endl;
        }
        return right == left + 4;
    }

    // Produces the coordinates of the hard-core vertex associated with the given corner (inside the contour). Lower-left coordinates (shift right and up by 1/2).
    void vertexCoords(int &vx, int &vy, int x1, int y1, int x2, int y2, int x3, int y3, bool prev, bool cw) {
        int offset = 256; // I need round towards negative infinity, not zero (for lower-left corner coordinates).
        int halfOffset = offset >> 1;
        bool right = prev ^ !(y2 & 1) ^ !(x2 & 1);
        bool pointingIn = right != cw;

        vx = (x1 + x3 + offset) / 2 - halfOffset;
        vy = (y1 + y3 + offset) / 2 - halfOffset;
        if(pointingIn) {
            // One offset is zero in each of the following sums.
            vx -= (x1 - x2) + (x3 - x2);
            vy -= (y1 - y2) + (y3 - y2);
        }
    }

    // Returns false if there is some consistent-parity vertex set hugging the inside of the polygon, true otherwise. Assumes the walk is closed.
    bool noVertexPolygon() {
        bool cw = clockwise();
        int x1 = -1, y1 = 0;
        int x2 = 0, y2 = 0;
        int x3 = 0, y3 = 0;
        int xStep = 1, yStep = 1;
        bool prev = (steps >> (length - 1)) & 1;
        if(prev) {
            x1 = 0;
            y1 = -1;
        }
        
        // Vertex coordinates, first vertex and current vertex.
        int v1x = 100, v1y = 100;
        int vx = 0, vy = 0;

        // The number of edges in the hard-core Z2 between vertices. Should be length minus 4 then divided by 2.
        int vertexStepCount = 0;

        uint64_t tempSteps = steps;
        for(int i = 0; i < length; i++) {
            bool step = tempSteps & 1;
            if(step) {
                y3 += yStep;
                xStep = -xStep;
            } else {
                x3 += xStep;
                yStep = -yStep;
            }

            if(prev != step) {
                int prevX = vx, prevY = vy;
                vertexCoords(vx, vy, x1, y1, x2, y2, x3, y3, prev, cw);
                if(v1x == 100) {
                    v1x = vx;
                    v1y = vy;
                } else {
                    int dx = abs(vx - prevX);
                    int dy = abs(vy - prevY);

                    // Must travel in a straight line (1 dimension changing), and an even number of steps.
                    if((dx != 0 && dy != 0) || (dx + dy) & 1) { 
                        return true;
                    }
                    vertexStepCount += (dx + dy) / 2;
                }
            }

            x1 = x2;
            y1 = y2;
            x2 = x3;
            y2 = y3;
            prev = step;
            tempSteps >>= 1;
        }

        // Final vertex to first vertex:
        int dx = abs(v1x - vx);
        int dy = abs(v1y - vy);
        if((dx != 0 && dy != 0) || (dx + dy) & 1) { 
            return true;
        }
        vertexStepCount += (dx + dy) / 2;

        // Enforcing length hypothesis.
        return vertexStepCount != (length - 4) / 2;
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
    std::vector<Walk> walks;

    walks.emplace_back(0, 0);
    uint64_t counts[65] = {0};

    while(!walks.empty()) {
        Walk current = walks.back();
        walks.pop_back();
        int prevX = 0, prevY = 0;
        current.getEndPoint(prevX, prevY);

        // Horizontal Step.
        if(current.approach() != 2 || current.length < 2) {
            Walk h = current.horizontalStep();
            int x = 0, y = 0;
            h.getEndPoint(x, y);
            bool noNarrowFlag = h.noNarrow(prevX, prevY, x, y);
            if(noNarrowFlag) {
                int firstTwoFlipped = ((h.steps & 1) << 1) | ((h.steps >> 1) & 1);
                if(h.noLoop(x, y) && stepsToOrigin[h.approach() * 40401 + (x + 100) * 201 + y + 100] <= n - h.length) {
                    walks.push_back(h);
                } else if(x == 0 && y == 0 && h.approach() != 2 - (firstTwoFlipped >> 1) && firstTwoFlipped != 2) { // Will loop (at origin, since loops only occur at the last point).  
                    counts[h.length >> 2]++;
                    if(h.noVertexPolygon()) {
                        std::cout << "No Vertex Polygon for: " << h.toBinary() << std::endl;
                    }
                    std::cout << h.toBinary() << std::endl;
                }
            }
        }

        // Vertical Step.
        if(current.approach() != 1 || current.length < 2) {
            Walk v = current.verticalStep();
            int x = 0, y = 0;
            v.getEndPoint(x, y);
            bool noNarrowFlag = v.noNarrow(prevX, prevY, x, y);
            if(noNarrowFlag) {
                int firstTwoFlipped = ((v.steps & 1) << 1) | ((v.steps >> 1) & 1);
                if(v.noLoop(x, y) && stepsToOrigin[v.approach() * 40401 + (x + 100) * 201 + y + 100] <= n - v.length) {
                    walks.push_back(v);
                } else if(x == 0 && y == 0 && v.approach() != 2 - (firstTwoFlipped >> 1) && firstTwoFlipped != 1) { // Will loop (at origin, since loops only occur at the last point).
                    counts[v.length >> 2]++;
                    if(v.noVertexPolygon()) {
                        std::cout << "No Vertex Polygon for: " << v.toBinary() << std::endl;
                    }
                    std::cout << v.toBinary() << std::endl;
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