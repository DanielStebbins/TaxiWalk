#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>

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

// int shortStraights(uint64_t steps, uint16_t length) {
//     int count = 0;
//     int streak = 0;
//     for(int i = 0; i < length; i++) {
//         int current = (steps >> i) & 1;
//         if(current == streak) {
//             ++count;
//             if(count > 4) {
//                 return 0;
//             }
//         } else {
//             count = 1;
//             streak = current;
//         }
//     }
//     return 1;
// }

int equalSteps(uint64_t steps, uint16_t length) {
    int xStep = 1;
    int yStep = 1;
    // int directions[4] = {0};
    int right = 0;
    int left = 0;
    int up = 0;
    int down = 0;
    for(int i = 0; i < length; ++i)
    {
        if(steps & 1)
        {
            // directions[0] += (yStep == 1);
            // directions[1] += (yStep == -1);
            up += (yStep == 1);
            down += (yStep == -1);
            xStep = -xStep;
        }
        else
        {
            // directions[2] += (xStep == 1);
            // directions[3] += (xStep == -1);
            right += (xStep == 1);
            left += (xStep == -1);
            yStep = -yStep;
        }
        steps >>= 1;
    }
    // bool test1 = right == left || right == up || right == down || left == up || left == down || up == down;
    // std::sort(directions, directions + (sizeof(directions) / sizeof(directions[0])));
    // bool test2 = directions[0] == directions[1] || directions[2] == directions[3];

    // if(test1 != test2) {
    //     std::cout << "PRE:" << up << " " << down << " " << right << " " << left << " " << std::endl;
    //     std::cout << "POST:" << directions[0] << " " << directions[1] << " " << directions[2] << " " << directions[3] << " " << std::endl;
    // }
    // return directions[0] == directions[1] || directions[2] == directions[3];

    return right == left || right == up || right == down || left == up || left == down || up == down;

    // return !right || !left || !up || !down;
}

bool canEqualSteps(uint64_t steps, uint16_t length, int N)
{
    int xStep = 1;
    int yStep = 1;
    int right = 0;
    int left = 0;
    int up = 0;
    int down = 0;
    // uint64_t temp = steps;
    for(int i = 0; i < length; ++i)
    {
        if(steps & 1)
        {
            up += (yStep == 1);
            down += (yStep == -1);
            xStep = -xStep;
        }
        else
        {
            right += (xStep == 1);
            left += (xStep == -1);
            yStep = -yStep;
        }
        steps >>= 1;
    }
    // std::cout << "PRE:" << up << " " << down << " " << right << " " << left << " " << std::endl;
    int remaining = N - length;
    // bool out = abs(right - left) <= remaining || abs(right - up) <= remaining|| abs(right - down) <= remaining || abs(left - up) <= remaining || abs(left - down) <= remaining || abs(up - down) <= remaining;
    // std::cout << toBinary(temp, length) << " has " << remaining << ". Can make equal? " << out << std::endl;
    // if(!out && remaining > 3) {
    //     std::cout << "Gasp!" << std::endl;
    // }
    return abs(right - left) <= remaining || abs(right - up) <= remaining|| abs(right - down) <= remaining || abs(left - up) <= remaining || abs(left - down) <= remaining || abs(up - down) <= remaining;
}

int parity(uint64_t steps, uint16_t length) {
    int xStep = 1;
    int yStep = 1;
    int right = 0;
    int left = 0;
    int up = 0;
    int down = 0;
    for(int i = 0; i < length; ++i)
    {
        if(steps & 1)
        {
            up += (yStep == 1);
            down += (yStep == -1);   
            xStep = -xStep;
        }
        else
        {
            right += (xStep == 1);
            left += (xStep == -1);    
            yStep = -yStep;
        }
        steps >>= 1;
    }
    up &= 1;
    down &= 1;
    right &= 1;
    left &= 1;
    // Seems supermultiplicative, but a potential good lower bound.
    return right == left && up == down;

    // return right == left && up == down || right == up && left == down || right == down && left == up;
}

// TAXI WALKS THAT CAN REACH THE ORIGIN IF GIVEN ANOTHER N STEPS.
bool canDoublePolygon(int endX, int endY, uint64_t steps, uint16_t length, int N, std::vector<int> const &stepsToOrigin)
{
    // Enough steps remaining to reach the origin. If given another N after the end?
    // int required = stepsToOrigin[approach(steps, length) * 40401 + (endX + 100) * 201 + endY + 100];
    // std::cout << toBinary(steps, length) << " needs " << required << " and has " << (N<<1)-length << std::endl;
    return stepsToOrigin[approach(steps, length) * 40401 + (endX + 100) * 201 + endY + 100] <= (N<<1)-length;
    // return stepsToOrigin[approach(steps, length) * 40401 + (endX + 100) * 201 + endY + 100] <= N;
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

uint64_t islandTaxi(int N)
{
    std::vector<int> stepsToOrigin = getStepsToOrigin();

    std::vector<State> untreated;
    untreated.reserve(91355000);

    // Start at H, length 1.
    untreated.emplace_back(1, 0);

    uint64_t count = 0;

    // int hi = N >> 1;
    // int lo = -(N >> 1);

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
            // if(x > lo && x < hi && y > lo && y < hi && noLoop(steps, length, x, y)) {
            if(noLoop(steps, length, x, y) && (N - length > 3 || canEqualSteps(steps, length, N))) {
            // if(noLoop(steps, length, x, y)) {
                if(length == N) {
                    // This walk is finished.
                    // std::cout << toBinary(steps, length) << std::endl;
                    // count += equalSteps(steps, length);
                    // count += parity(steps, length);
                    // count += canDoublePolygon(x, y, steps, length, N, stepsToOrigin);
                    ++count;
                }
                else {
                // else if(can_parity(steps, length, N)) {
                    untreated.emplace_back(length, steps);
                }
            }
        }

        // Vertical Step.
        if(approach(current.steps, current.length) != 2 || current.length < 2)
        // TO EXCLUDE TURN IN FIRST 2 STEPS.
        // if(approach(current.steps, current.length) != 2 && current.length >= 2)
        {
            uint16_t length = current.length + 1;
            uint64_t steps = current.steps | (1ULL << current.length);

            int x = 0, y = 0;
            getPoint(steps, length, x, y);
            // if(x > lo && x < hi && y > lo && y < hi && noLoop(steps, length, x, y)) {
            if(noLoop(steps, length, x, y) && (N - length > 3 || canEqualSteps(steps, length, N))) {
            // if(noLoop(steps, length, x, y)) {
                if(length == N) {
                    // This walk is finished.
                    // std::cout << toBinary(steps, length) << std::endl;
                    // count += equalSteps(steps, length);
                    // count += parity(steps, length);
                    // count += canDoublePolygon(x, y, steps, length, N, stepsToOrigin);
                    ++count;
                }
                else {
                // else if(can_parity(steps, length, N)) {
                    untreated.emplace_back(length, steps);
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
        std::cout << "\nn=" << n << ": " << islandTaxi(n) << std::endl;
        // std::cout << islandTaxi(n) << std::endl;
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