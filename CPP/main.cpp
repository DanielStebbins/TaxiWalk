#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
//#include <bitset>
#include <chrono>

struct State {
    uint16_t length;
    uint64_t steps;
    uint64_t childIndices[2]{};

    State(uint16_t length, uint64_t steps):
        length(length), steps(steps), childIndices{ULLONG_MAX,ULLONG_MAX} {}
};

//std::string toBinary(uint64_t n, uint16_t len)
//{
//    if(len == 0) {
//        return "Origin";
//    }
//    std::string binary;
//    for(uint16_t i = 0; i < len; i++)
//    {
//        if((n >> i) & 1)
//        {
//            binary += "V";
//        }
//        else
//        {
//            binary += "H";
//        }
//    }
//    return binary;
//}

//struct Indices {
//    uint64_t state;
//    uint64_t parent;
//
//    Indices(uint64_t state, uint64_t parent):
//        state(state), parent(parent) {}
//};

void getPoint(uint64_t steps, uint16_t length, int &x, int &y)
{
    int xStep = 1;
    int yStep = 1;
    for(int i = 0; i < length; ++i)
    {
        // Try removing if statements.
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
//    std::cout << length - 1 << " " << toBinary(steps >> (length - 1), 32) << std::endl;
    return (int) ((steps >> (length - 2) & 1) * 2 + (steps >> (length - 1)));
}

void reduce(uint64_t &steps, uint16_t &length, int endX, int endY, int n, std::vector<int> const &stepsToOrigin)
{
    while(stepsToOrigin[approach(steps, length) * 40401 + (endX + 100) * 201 + endY + 100] > n - length)
    {
        if(steps & 1)
        {
            endX = -endX;
            endY -= 1;
        }
        else
        {
            endX -= 1;
            endY = -endY;
        }
        steps >>= 1;
        --length;

        if(steps & 1)
        {
//            std::cout << "Flip" << std::endl;
            steps ^= (1ULL << length) - 1;
            int temp = endX;
            endX = endY;
            endY = temp;
        }
//        std::cout << toBinary(steps, length) << std::endl;
//        std::cout << endX << " " << endY << std::endl;
//        std::cout << approach(steps, length) << std::endl;
//        std::cout << approach(steps, length) * 40401 + (endX + 100) * 201 + endY + 100 << std::endl;
    }

    // If first step is now vertical, flip to horizontal.
    if(steps & 1)
    {
        steps ^= (1ULL << length) - 1;
    }
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

uint64_t taxi(int n)
{
    std::vector<int> stepsToOrigin = getStepsToOrigin();

    std::vector<State> states;
    states.reserve(100);
    states.emplace_back(0, 0);

    uint64_t untreated = 0;

    while(untreated < states.size())
    {
//        State* start = &states.at(untreated);
//        ++untreated;
//        std::cout << std::endl << "Start: " << toBinary(states[untreated].steps, states[untreated].length) << std::endl;
        if(approach(states[untreated].steps, states[untreated].length) != 1 || states[untreated].length < 2)
        {
            uint64_t steps = states[untreated].steps;
            uint16_t length = states[untreated].length + 1;
//            std::cout << "H: " << toBinary(steps, length) << " (length " << length << ")" << std::endl;
            int x = 0, y = 0;
            getPoint(steps, length, x, y);
//            std::cout << "x=" << x << ", y=" << y << std::endl;
            if(noLoop(steps, length, x, y))
            {
                reduce(steps, length, x, y, n, stepsToOrigin);
//                std::cout << "Reduce: " << std::bitset<64>(steps) << " (length " << length << ")" << std::endl;
                State parent = states[0];
                uint64_t tempSteps = steps;
//                std::cout << "length: " << length << std::endl;
                for(int i = 0; i < length - 1; ++i)
                {
//                    std::cout << "Parent Index: " << parent.childIndices[tempSteps & 1] << std::endl;
                    parent = states[parent.childIndices[tempSteps & 1]];
//                    std::cout << "Parent: " << std::bitset<64>(parent.steps) << " (length " << parent.length << ")" << std::endl;
                    tempSteps >>= 1;
                }
//                std::cout << "Parent: " << std::bitset<64>(parent.steps) << " (length " << parent.length << ")" << std::endl;
                if(parent.childIndices[tempSteps] == ULLONG_MAX)
                {
                    states[untreated].childIndices[0] = states.size();
                    states.emplace_back(length, steps);
                }
                else
                {
//                    std::cout << "Horizontal Index Reuse" << std::endl;
                    states[untreated].childIndices[0] = parent.childIndices[tempSteps];
                }
            }
        }

        if(approach(states[untreated].steps, states[untreated].length) != 2 || states[untreated].length < 2)
        {
            uint64_t steps = states[untreated].steps | (1ULL << states[untreated].length);
            uint16_t length = states[untreated].length + 1;
//            std::cout << "V: " << toBinary(steps, length) << std::endl;
            int x = 0, y = 0;
            getPoint(steps, length, x, y);
//            std::cout << "x=" << x << ", y=" << y << std::endl;
            if(noLoop(steps, length, x, y))
            {
                reduce(steps, length, x, y, n, stepsToOrigin);
//                std::cout << "Reduce: " << std::bitset<64>(steps) << " (length " << length << ")" << std::endl;
                State parent = states[0];
                uint64_t tempSteps = steps;
//                std::cout << "length: " << length << std::endl;
                for(int i = 0; i < length - 1; i++)
                {
//                    std::cout << "Parent Index: " << parent.childIndices[tempSteps & 1] << std::endl;
                    parent = states[parent.childIndices[tempSteps & 1]];
//                    std::cout << "Parent: " << std::bitset<64>(parent.steps) << " (length " << parent.length << ")" << std::endl;
                    tempSteps >>= 1;
                }
//                std::cout << "Parent: " << std::bitset<64>(parent.steps) << " (length " << parent.length << ")" << std::endl;
                if(parent.childIndices[tempSteps] == ULLONG_MAX)
                {
                    states[untreated].childIndices[1] = states.size();
//                    std::cout << "Vertical New State " << states[untreated].steps << std::endl;
                    states.emplace_back(length, steps);
                }
                else
                {
//                    std::cout << "Vertical Index Reuse" << std::endl;
                    states[untreated].childIndices[1] = parent.childIndices[tempSteps];
                }
            }
        }
//        std::cout << "End" << std::endl;
//        std::cout << states[untreated].steps << std::endl;
//        std::cout << "Steps: " << toBinary(states[untreated].steps, states[untreated].length) << std::endl;
//        std::cout << "Start Child Indices: " << states[untreated].childIndices[0] << " " << states[untreated].childIndices[1] << std::endl;
//        std::cout << "Size: " << states.size() << std::endl;
        ++untreated;
    }

//    for(auto & state : states) {
//        std::cout << toBinary(state.steps, state.length) << std::endl;
//    }

    return states.size();
}

int main()
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::cout << taxi(47) << std::endl;
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference (sec) = " <<  std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() /1000000.0  <<std::endl;
    return 0;
}


