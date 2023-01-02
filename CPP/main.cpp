#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
//#include <bitset>
#include <chrono>

// Timers
long long getPointTime = 0;
long long noLoopTime = 0;
long long approachTime = 0;
long long reduceTime = 0;
long long updateAutomatonTime = 0;

struct State {
    uint16_t length;
    uint64_t steps;
    uint64_t childIndices[2]{};

    State(uint16_t length, uint64_t steps):
        length(length), steps(steps), childIndices{ULLONG_MAX,ULLONG_MAX} {}
};

void getPoint(uint64_t steps, uint16_t length, int &x, int &y)
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
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
    getPointTime += (std::chrono::steady_clock::now() - begin).count();
}

bool noLoop(uint64_t steps, uint16_t length, int endX, int endY)
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
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
    noLoopTime += (std::chrono::steady_clock::now() - begin).count();
    return noLoop;
}

int approach(uint64_t steps, uint16_t length)
{
//    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    int temp = (int) ((steps >> (length - 2) & 1) * 2 + (steps >> (length - 1)));
//    approachTime += (std::chrono::steady_clock::now() - begin).count();
    return temp;
}

void reduce(uint64_t &steps, uint16_t &length, int endX, int endY, int n, std::vector<int> const &stepsToOrigin)
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
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
            steps ^= (1ULL << length) - 1;
            int temp = endX;
            endX = endY;
            endY = temp;
        }
    }

    // If first step is now vertical, flip to horizontal.
    if(steps & 1)
    {
        steps ^= (1ULL << length) - 1;
    }
    reduceTime += (std::chrono::steady_clock::now() - begin).count();
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
    states.reserve(16704000);
    states.emplace_back(0, 0);

    uint64_t untreated = 0;

    while(untreated < states.size())
    {
        if(approach(states[untreated].steps, states[untreated].length) != 1 || states[untreated].length < 2)
        {
            uint64_t steps = states[untreated].steps;
            uint16_t length = states[untreated].length + 1;
            int x = 0, y = 0;
            getPoint(steps, length, x, y);
            if(noLoop(steps, length, x, y))
            {
                reduce(steps, length, x, y, n, stepsToOrigin);

                std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
                State parent = states[0];
                uint64_t tempSteps = steps;
                for(int i = 0; i < length - 1; ++i)
                {
                    parent = states[parent.childIndices[tempSteps & 1]];
                    tempSteps >>= 1;
                }

                if(parent.childIndices[tempSteps] == ULLONG_MAX)
                {
                    states[untreated].childIndices[0] = states.size();
                    states.emplace_back(length, steps);
                }
                else
                {
                    states[untreated].childIndices[0] = parent.childIndices[tempSteps];
                }
                updateAutomatonTime += (std::chrono::steady_clock::now() - begin).count();
            }
        }

        if(approach(states[untreated].steps, states[untreated].length) != 2 || states[untreated].length < 2)
        {
            uint64_t steps = states[untreated].steps | (1ULL << states[untreated].length);
            uint16_t length = states[untreated].length + 1;
            int x = 0, y = 0;
            getPoint(steps, length, x, y);
            if(noLoop(steps, length, x, y))
            {
                reduce(steps, length, x, y, n, stepsToOrigin);

                std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
                State parent = states[0];
                uint64_t tempSteps = steps;
                for(int i = 0; i < length - 1; i++)
                {
                    parent = states[parent.childIndices[tempSteps & 1]];
                    tempSteps >>= 1;
                }
                if(parent.childIndices[tempSteps] == ULLONG_MAX)
                {
                    states[untreated].childIndices[1] = states.size();
                    states.emplace_back(length, steps);
                }
                else
                {
                    states[untreated].childIndices[1] = parent.childIndices[tempSteps];
                }
                updateAutomatonTime += (std::chrono::steady_clock::now() - begin).count();
            }
        }
        ++untreated;
    }
    return states.size();
}

int main()
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::cout << taxi(47) << std::endl;
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    double totalTime = (double)(end - begin).count() / 1000000000.0;
    double getPoint = (double) getPointTime / 1000000000.0;
    double noLoop = (double) noLoopTime / 1000000000.0;
    double approach = (double) approachTime / 1000000000.0;
    double reduce = (double) reduceTime / 1000000000.0;
    double updateAutomaton = (double) updateAutomatonTime / 1000000000.0;
    double sumTime = (double) (getPointTime + noLoopTime + approachTime + reduceTime + updateAutomatonTime) / 1000000000.0;
    std::cout << "Total Time: " << totalTime << std::endl;
    std::cout << "Get Point Time: " << getPoint << " (" << (getPoint / totalTime * 100.0) << "%)" << std::endl;
    std::cout << "No Loop Time: " << noLoop << " (" << (noLoop / totalTime * 100.0) << "%)" << std::endl;
    std::cout << "Approach Time: " << approach << " (" << (approach / totalTime * 100.0) << "%)" << std::endl;
    std::cout << "Reduce Time: " << reduce << " (" << (reduce / totalTime * 100.0) << "%)" << std::endl;
    std::cout << "Update Automaton Time: " << updateAutomaton << " (" << (updateAutomaton / totalTime * 100.0) << "%)" << std::endl;
    std::cout << "Sum of Times: " << sumTime << " (" << (sumTime / totalTime * 100.0) << "%)" << std::endl;
    return 0;
}

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