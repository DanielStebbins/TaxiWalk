#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <bitset>

struct State {
    uint16_t length;
    uint64_t steps;
    uint64_t index;
    uint64_t childIndices[2]{};

    State(uint16_t length, uint64_t steps, uint64_t index):
        length(length), steps(steps), index(index), childIndices{ULLONG_MAX,ULLONG_MAX} {}
};

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
            steps ^= (1L << length) - 1;
            int temp = endX;
            endX = endY;
            endY = temp;
        }
    }

    // If first step is now vertical, flip to horizontal.
    if((steps & 1) == 1)
    {
        steps ^= (1L << length) - 1;
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

uint64_t taxi(int n)
{
    std::vector<int> stepsToOrigin = getStepsToOrigin();

    std::vector<State> states;
    states.reserve(100);
    states.emplace_back(0, 0, 0);

    uint64_t untreated = 0;

    while(untreated != states.size())
    {
//        State* start = &states.at(untreated);
        ++untreated;
        std::cout << std::endl << "Start: " << toBinary(start->steps, start->length) << std::endl;
        if(approach(start->steps, start->length) != 1 || start->length < 2)
        {
            uint64_t steps = start->steps;
            uint16_t length = start->length + 1;
//            std::cout << "H: " << std::bitset<64>(steps) << " (length " << length << ")" << std::endl;
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
                    start->childIndices[0] = states.size();
                    states.emplace_back(length, steps, states.size());
                }
                else
                {
                    std::cout << "Horizontal Index Reuse" << std::endl;
                    std::cout << tempSteps << std::endl;
                    std::cout << parent.childIndices[tempSteps] << std::endl;
                    std::cout << states[parent.childIndices[tempSteps]].index << std::endl;
                    start->childIndices[0] = parent.childIndices[tempSteps];
                    std::cout << start->childIndices[0] << std::endl;
                }
            }
        }

        if(approach(start->steps, start->length) != 2 || start->length < 2)
        {
            uint64_t steps = start->steps | (1L << start->length);
            uint16_t length = start->length + 1;
//            std::cout << "V: " << std::bitset<64>(steps) << " (length " << length << ")" << std::endl;
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
                    start->childIndices[1] = states.size();
                    std::cout << "Vertical New State " << start->steps << std::endl;
                    states.emplace_back(length, steps, states.size());
                }
                else
                {
                    std::cout << "Vertical Index Reuse" << std::endl;
                    start->childIndices[1] = parent.childIndices[tempSteps];
                }
            }
        }
        std::cout << "End" << std::endl;
        std::cout << start->steps << std::endl;
        std::cout << "Steps: " << toBinary(start->steps, start->length) << std::endl;
        std::cout << "Start Child Indices: " << start->childIndices[0] << " " << start->childIndices[1] << std::endl;
        std::cout << "Size: " << states.size() << std::endl;
    }

    for(auto & state : states) {
        std::cout << toBinary(state.steps, state.length) << std::endl;
    }

    return states.size();
}

int main()
{
    // Crashes for n > 23. Off by 1 for 19 and 23.
    std::cout << taxi(27) << std::endl;
    return 0;
}


