#include <iostream>
#include <list>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

struct State {
    uint16_t length;
    uint64_t steps;
    uint64_t horizontal;
    uint64_t vertical;

    State(uint16_t length, uint64_t steps, uint64_t h = 0, uint64_t v = 0):
            length(length), steps(steps), horizontal(h), vertical(v) {}
};

void getPoint(uint64_t steps, uint16_t length, int &x, int &y)
{
    int xStep = 1;
    int yStep = 1;
    for(int i = 0; i < length; ++i)
    {
        // Try removing if statements.
        if(steps >> i & 1)
        {
            y += yStep;
            xStep = -xStep;
        }
        else
        {
            x += xStep;
            yStep = -yStep;
        }
    }
}

bool hasLoop(uint64_t steps, uint16_t length, int endX, int endY)
{
    int x = 0;
    int xStep = 1;
    int y = 0;
    int yStep = 1;
    bool loop = x == endX && y == endY;
    int i = 0;
    while(!loop && i < length - 12)
    {
        if(steps >> i & 1)
        {
            y += yStep;
            xStep = -xStep;
        }
        else
        {
            x += xStep;
            yStep = -yStep;
        }
        loop = x == endX && y == endY;
        ++i;
    }
    return loop;
}

int approach(uint64_t steps, uint16_t length)
{
    return (int) ((steps >> (length - 2) & 1) * 2 + (steps >> (length - 1)));
}

void reduce(uint64_t &steps, uint16_t &length, int endX, int endY, int n, const int stepsToOrigin[])
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
            steps = steps ^ ((1L << length) - 1);
            int temp = endX;
            endX = endY;
            endY = temp;
        }
    }

    // If first step is now vertical, flip to horizontal.
    if((steps & 1) == 1)
    {
        steps = steps ^ ((1L << length) - 1);
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

int taxi(int n)
{
    std::vector<int> stepsToOrigin = getStepsToOrigin();

    std::vector<State> states;
    states.reserve(100);
    states.emplace_back(0, 0);

    std::list<uint64_t> untreated;
    untreated.push_back(0);

    while(!untreated.empty())
    {
        State start = states.at(untreated.front());
        untreated.pop_front();

    }

    return 0;
}

int main()
{
    std::cout << taxi(15) << std::endl;
    return 0;
}


