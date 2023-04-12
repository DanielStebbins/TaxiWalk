// Runs N=47 in 12 seconds using 0.5GB.
// Runs N=51 in 73 seconds using 2.8GB.

// Predictions (Time x1.58 (x6.2 per 4), Space x1.53 (x5.5 per 4)):
// N=55: 7m45s, 16GB.
// N=59: 48m, 87GB.
// N=63: 5h, 482GB.

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>

struct State {
    uint64_t var1;
    uint64_t var2;
    State *children[2]{};

    State(uint16_t length, uint64_t steps):
        var1(length), var2(steps), children{nullptr,nullptr} {}
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

// Used to remove the heuristic for computing the steps to the origin.
//bool canReachOrigin(uint64_t steps, uint16_t length, int endX, int endY, int n, std::vector<int> const &stepsToOrigin)
//{
////    std::cout << "can reach" << std::endl;
//    if(endX == 0 && endY == 0)
//    {
//        return true;
//    }
//    if(stepsToOrigin[approach(steps, length) * 40401 + (endX + 100) * 201 + endY + 100] > n - length)
//    {
//        return false;
//    }
//
//    // Horizontal Step.
//    bool horizontalCanReach = false;
//    bool verticalCanReach = false;
//    if(approach(steps, length) != 1 || length < 2)
//    {
//        int nextX = 0;
//        int nextY = 0;
//        getPoint(steps, length + 1, nextX, nextY);
//        if(nextX == 0 && nextY == 0)
//        {
//            return true;
//        }
//        if(noLoop(steps, length + 1, nextX, nextY))
//        {
//            horizontalCanReach = canReachOrigin(steps, length + 1, nextX, nextY, n, stepsToOrigin);
//        }
//    }
//
//    if(approach(steps, length) != 2 || length < 2)
//    {
//        uint64_t nextSteps = steps | (1ULL << length);
//        int nextX = 0;
//        int nextY = 0;
//        getPoint(nextSteps, length + 1, nextX, nextY);
//        if(nextX == 0 && nextY == 0)
//        {
//            return true;
//        }
//        if(noLoop(nextSteps, length + 1, nextX, nextY))
//        {
//            verticalCanReach = canReachOrigin(nextSteps, length + 1, nextX, nextY, n, stepsToOrigin);
//        }
//    }
//
//    return horizontalCanReach || verticalCanReach;
//}

void reduce(uint64_t &steps, uint16_t &length, int endX, int endY, int n, std::vector<int> const &stepsToOrigin)
{
//    while(!canReachOrigin(steps, length, endX, endY, n, stepsToOrigin))
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

    // If first step is vertical, flip to horizontal.
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

std::vector<State> makeAutomaton(int n)
{
    std::vector<int> stepsToOrigin = getStepsToOrigin();

    std::vector<State> states;
    states.reserve(91355000);
    states.emplace_back(0, 0);

    uint64_t untreated = 0;

    while(untreated < states.size())
    {
        // Horizontal Step.
        if(approach(states[untreated].var2, states[untreated].var1) != 1 || states[untreated].var1 < 2)
        {
            uint16_t length = states[untreated].var1 + 1;
            uint64_t steps = states[untreated].var2;

            int x = 0, y = 0;
            getPoint(steps, length, x, y);
            if(noLoop(steps, length, x, y))
            {
                reduce(steps, length, x, y, n, stepsToOrigin);

                State parent = states[0];
                uint64_t tempSteps = steps;
                for(int i = 0; i < length - 1; ++i)
                {
                    parent = *parent.children[tempSteps & 1];
                    tempSteps >>= 1;
                }

                if(!parent.children[tempSteps])
                {
                    states.emplace_back(length, steps);
                    states[untreated].children[0] = &states[states.size() - 1];
                }
                else
                {
                    states[untreated].children[0] = parent.children[tempSteps];
                }
            }
            else {
//                std::cout << toBinary(states[untreated].var2, states[untreated].var1) << std::endl;
            }
        }

        // Vertical Step.
        if(approach(states[untreated].var2, states[untreated].var1) != 2 || states[untreated].var1 < 2)
        {
            uint16_t length = states[untreated].var1 + 1;
            uint64_t steps = states[untreated].var2 | (1ULL << states[untreated].var1);

            int x = 0, y = 0;
            getPoint(steps, length, x, y);
            if(noLoop(steps, length, x, y))
            {
                reduce(steps, length, x, y, n, stepsToOrigin);

                State parent = states[0];
                uint64_t tempSteps = steps;
                for(int i = 0; i < length - 1; i++)
                {
                    parent = *parent.children[tempSteps & 1];
                    tempSteps >>= 1;
                }
                if(!parent.children[tempSteps])
                {
                    states.emplace_back(length, steps);
                    states[untreated].children[1] = &states[states.size() - 1];
                }
                else
                {
                    states[untreated].children[1] = parent.children[tempSteps];
                }
            }
            else {
//                std::cout << toBinary(states[untreated].var2, states[untreated].var1) << std::endl;
            }
        }
        ++untreated;
    }

    // Reset first 16 bytes to use when running the automaton.
    for(auto & state : states)
    {
        state.var1 = 0;
        state.var2 = 0;
    }
    return states;
}

uint64_t taxi(int N)
{
    std::vector<State> automaton = makeAutomaton(N);

    // Sets "H" to current count 1.
    automaton[1].var1 = 1;

    // Ends one step early, because on the final loop there's no need to move var2 to var1.
    for(int n = 2; n < N; ++n)
    {
        for(auto & state : automaton)
        {
            if(state.var1)
            {
                if(state.children[0])
                {
                    (*state.children[0]).var2 += state.var1;
                }
                if(state.children[1])
                {
                    (*state.children[1]).var2 += state.var1;
                }
            }
        }
        for(auto & state : automaton)
        {
            state.var1 = state.var2;
            state.var2 = 0;
        }
    }

//    std::cout <<"Number of states for length " << N << ": " << automaton.size() << std::endl;

    uint64_t taxiWalks = 0;
    for(auto & state : automaton)
    {
        if(state.var1)
        {
            if(state.children[0])
            {
                taxiWalks += state.var1;
            }
            if(state.children[1])
            {
                taxiWalks += state.var1;
            }
        }
    }
    return taxiWalks * 2;
}

void upTo(int start, int stop)
{
    for(int n = start; n <= stop; n += 4)
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        std::cout << "\nn=" << n << ": " << taxi(n) << std::endl;
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

    int n = 43;
    upTo(15, n);

    return 0;
}