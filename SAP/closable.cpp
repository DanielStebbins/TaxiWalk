#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <deque>

struct State {
    uint16_t length;
    uint64_t steps;

    State(uint16_t l, uint64_t s):
        length(l), steps(s) {}
};

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

void getPoint(uint64_t steps, uint16_t length, int &x, int &y)
{
    int xStep = 1;
    int yStep = 1;
    for(int i = 0; i < length; ++i)
    {
        if(steps & 1)
        {
            y += yStep;
            xStep = -xStep;
        }
        else
        {
            x += xStep;
            yStep = -yStep;
        }
        steps >>= 1;
    }
}

void getPointAndBoundingBox(uint64_t steps, uint16_t length, int &x, int &y, int &minX, int &maxX, int &minY, int &maxY) {
    int xStep = 1;
    int yStep = 1;
    for(int i = 0; i < length; ++i)
    {
        if(steps & 1)
        {
            y += yStep;
            xStep = -xStep;
            minY = std::min(y, minY);
            maxY = std::max(y, maxY);
        }
        else
        {
            x += xStep;
            yStep = -yStep;
            minX = std::min(x, minX);
            maxX = std::max(x, maxX);
        }
        steps >>= 1;
    }
}

// bool leavingBoundingBox(uint64_t steps, uint16_t length, int x, int y, int minX, int maxX, int minY, int maxY) {
//     return ((x == minX || x == maxX) && (((steps >> length) & 1) == 0)) ||((y == minY || y == maxY) && ((steps >> length) & 1));
// }

bool noLoop(uint64_t steps, uint16_t length, int endX, int endY)
{
    int x = 0;
    int xStep = 1;
    int y = 0;
    int yStep = 1;
    bool noLoop = x != endX || y != endY;
    int i = 0;
    while(noLoop && i < length - 12)
    {
        if(steps & 1)
        {
            y += yStep;
            xStep = -xStep;
        }
        else
        {
            x += xStep;
            yStep = -yStep;
        }
        noLoop = x != endX || y != endY;
        steps >>= 1;
        ++i;
    }
    return noLoop;
}

int approach(uint64_t steps, uint16_t length)
{
    return (int) ((steps >> (length - 2) & 1) * 2 + (steps >> (length - 1)));
}

std::vector<int> getStepsToOrigin()
{
    std::ifstream ifs(R"(C:\Users\danrs\Documents\GitHub\TaxiWalk\CPP\StepsToOrigin.txt)");
    std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    std::stringstream stream(content);
    std::vector<int> stepsToOrigin;
    int num;
    while(stream >> num)
    {
        stepsToOrigin.push_back(num);
    }
    return stepsToOrigin;
}

int trueStepsToOrigin(uint64_t steps, uint16_t length) {
    // Hand-checked, cannot reach the origin even given infinite steps due to origin being boxed (endpoint boxed leads to all walks dying so no extra check.)
    // if(steps == 0b0111110001110) {
    //     return 0;
    // }
    // std::cout << toBinary(steps, length) << std::endl;
    
    std::deque<State> untreated;
    untreated.emplace_back(length, steps);
    int current_length = length;
    // This should be the max number of steps to return to the origin for any walk that is not boxed.
    int max_length = 2 * length + 12;

    while(!untreated.empty() && current_length <= max_length) {
        // std::cout << current_length << std::endl;
        struct State current = untreated.front();
        untreated.pop_front();
        
        // Horizontal Step.
        if(approach(current.steps, current.length) != 1 || current.length < 2) {
            uint16_t length = current.length + 1;
            uint64_t steps = current.steps;
            current_length = length;
            

            int x = 0, y = 0;
            getPoint(steps, length, x, y);
            if(x == 0 && y == 0) {
                // Connecting the end of the "loop" to the beginning and checking the two-turn rule.
                int first = steps & 1;
                int second = (steps >> 1) & 1;
                int second_last = (steps >> (length - 2)) & 1;
                int last = (steps >> (length - 1)) & 1;
                if(last == first || (second_last == last && first == second)) {
                    return length;
                }
                // std::cout << toBinary(steps, length) << std::endl;
            }
            else if(noLoop(steps, length, x, y)) {
                untreated.emplace_back(length, steps);
            }
        }

        // Vertical Step.
        if(approach(current.steps, current.length) != 2 || current.length < 2)
        {
            uint16_t length = current.length + 1;
            uint64_t steps = current.steps | (1ULL << current.length);
            current_length = length;

            int x = 0, y = 0;
            getPoint(steps, length, x, y);
            if(x == 0 && y == 0) {
                // Connecting the end of the "loop" to the beginning and checking the two-turn rule.
                int first = steps & 1;
                int second = (steps >> 1) & 1;
                int second_last = (steps >> (length - 2)) & 1;
                int last = (steps >> (length - 1)) & 1;
                if(last == first || (second_last == last && first == second)) {
                    return length;
                }
                // std::cout << toBinary(steps, length) << std::endl;
            }
            else if(noLoop(steps, length, x, y)) {
                untreated.emplace_back(length, steps);
            }
        }
    }
    if(untreated.empty()) {
        std::cout << "END BOXED: " << toBinary(steps, length) << std::endl;
    } else {
        std::cout << "ORIGIN BOXED: " << toBinary(steps, length) << std::endl;
    }
    return 0;
}


int stepsToBoundingBox(uint64_t steps, uint16_t length) {
    // Hand-checked, cannot reach the origin even given infinite steps due to origin being boxed (endpoint boxed leads to all walks dying so no extra check.)
    // if(steps == 0b0111110001110) {
    //     return 0;
    // }
    // std::cout << toBinary(steps, length) << std::endl;
    
    std::deque<State> untreated;
    untreated.emplace_back(length, steps);
    int current_length = length;
    // This should be the max number of steps to return to the origin for any walk that is not boxed.
    int max_length = 2 * length + 12;

    while(!untreated.empty() && current_length <= max_length) {
        // std::cout << current_length << std::endl;
        struct State current = untreated.front();
        untreated.pop_front();
        
        // Horizontal Step.
        if(approach(current.steps, current.length) != 1 || current.length < 2) {
            uint16_t length = current.length + 1;
            uint64_t steps = current.steps;
            current_length = length;
            

            int x = 0, y = 0, minX = 0, maxX = 0, minY = 0, maxY = 0;
            getPointAndBoundingBox(steps, length, x, y, minX, maxX, minY, maxY);
            if(noLoop(steps, length, x, y)) {
                int lastStep = (steps >> (length - 1)) & 1;
                if(((x == minX || x == maxX) && lastStep == 0) || ((y == minY || y == maxY) && lastStep == 1)) {
                    return length;
                } else {
                    untreated.emplace_back(length, steps);
                }
            }
        }

        // Vertical Step.
        if(approach(current.steps, current.length) != 2 || current.length < 2)
        {
            uint16_t length = current.length + 1;
            uint64_t steps = current.steps | (1ULL << current.length);
            current_length = length;

            int x = 0, y = 0, minX = 0, maxX = 0, minY = 0, maxY = 0;
            getPointAndBoundingBox(steps, length, x, y, minX, maxX, minY, maxY);
            if(noLoop(steps, length, x, y)) {
                int lastStep = (steps >> (length - 1)) & 1;
                if(((x == minX || x == maxX) && lastStep == 0) || ((y == minY || y == maxY) && lastStep == 1)) {
                    return length;
                } else {
                    untreated.emplace_back(length, steps);
                }
            }
        }
    }
    // if(untreated.empty()) {
    //     std::cout << "END BOXED: " << toBinary(steps, length) << std::endl;
    // }
    return 0;
}

uint64_t generate(int N)
{
    std::vector<int> stepsToOrigin = getStepsToOrigin();

    std::vector<State> untreated;
    untreated.reserve(91355000);

    // Start at H, length 1.
    untreated.emplace_back(1, 0);

    uint64_t count = 0;
    int maxStepsToOrigin = 0;
    uint64_t worstSteps = 0;

    while(!untreated.empty())
    {
        struct State current = untreated.back();
        untreated.pop_back();
        
        // Horizontal Step.
        if(approach(current.steps, current.length) != 1 || current.length < 2) {
            uint16_t length = current.length + 1;
            uint64_t steps = current.steps;

            int x = 0, y = 0;
            getPoint(steps, length, x, y);
            if(noLoop(steps, length, x, y)) {
                if(length == N) {
                    // maxStepsToOrigin = std::max(maxStepsToOrigin, trueStepsToOrigin(steps, length));
                    ++count;
                    int temp = stepsToBoundingBox(steps, length);
                    if(temp > maxStepsToOrigin) {
                        maxStepsToOrigin = temp;
                        worstSteps = steps;
                    }
                } else {
                    untreated.emplace_back(length, steps);
                }
            }
        }

        // Vertical Step.
        if(approach(current.steps, current.length) != 2 || current.length < 2)
        {
            uint16_t length = current.length + 1;
            uint64_t steps = current.steps | (1ULL << current.length);

            int x = 0, y = 0;
            getPoint(steps, length, x, y);
            if(noLoop(steps, length, x, y)) {
                if(length == N) {
                    // maxStepsToOrigin = std::max(maxStepsToOrigin, trueStepsToOrigin(steps, length));
                    ++count;
                    int temp = stepsToBoundingBox(steps, length);
                    if(temp > maxStepsToOrigin) {
                        maxStepsToOrigin = temp;
                        worstSteps = steps;
                    }
                } else {
                    untreated.emplace_back(length, steps);
                }
            }
        }
    }
    std::cout << "All can leave the bounding box in " << maxStepsToOrigin - N << " more steps."<< std::endl;
    std::cout << toBinary(worstSteps, N) << std::endl;
    return count << 1;
}

void upTo(int start, int stop) {
    for(int n = start; n <= stop; n += 1)
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        std::cout << "\nn=" << n << ": " << generate(n) << std::endl;
        // std::cout << generate(n) << std::endl;
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