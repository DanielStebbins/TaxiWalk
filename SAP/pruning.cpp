#include <iostream>
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

int approach(uint64_t steps, uint16_t length) {
    return (int) ((steps >> (length - 2) & 1) * 2 + (steps >> (length - 1)));
}

bool canTaxiPolygon(uint64_t steps, uint16_t length, int N, std::vector<int> const &stepsToOrigin)
{
    int endX = 0, endY = 0;
    getPoint(steps, length, endX, endY);

    // Not enough steps remaining to reach the origin.
    if(stepsToOrigin[approach(steps, length) * 40401 + (endX + 100) * 201 + endY + 100] > N - length) {
        return false;
    }
    
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

int isTaxiPolygon(uint64_t steps, int N) {
    // Return 0 if the two turn rule is violated around the connection.
    int first = steps & 1;
    int second = (steps >> 1) & 1;
    int second_last = (steps >> (N - 2)) & 1;
    int last = (steps >> (N - 1)) & 1;
    if(last != first && (second_last != last || first != second)) {
        return 0;
    }

    // Ends on the origin -> taxi polygon. No earlier intersection guaranteed by checks on earlier walks.
    int x = 0, y = 0;
    getPoint(steps, N, x, y);
    return x == 0 && y == 0;
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

uint64_t taxiPolygon(int N)
{
    std::vector<int> stepsToOrigin = getStepsToOrigin();

    std::vector<State> untreated;
    untreated.reserve(91355000);

    // Start at H, length 1.
    untreated.emplace_back(1, 0);

    uint64_t count = 0;

    uint64_t out = 0;

    while(!untreated.empty())
    {
        struct State current = untreated.back();
        untreated.pop_back();
        
        // Horizontal Step.
        if(approach(current.steps, current.length) != 1 || current.length < 2) {
            uint16_t length = current.length + 1;
            uint64_t steps = current.steps;

            // This walk is finished, check if it's a polygon and follows the two-turn rule at the connection.
            if(length == N) {
                int is = isTaxiPolygon(steps, N);
                if(is && count == 5384903) {
                    out = steps;
                }
                count += is;
            }
            else if(canTaxiPolygon(steps, length, N, stepsToOrigin)) {
                untreated.emplace_back(length, steps);
            }
        }

        // Vertical Step.
        if(approach(current.steps, current.length) != 2 || current.length < 2)
        {
            uint16_t length = current.length + 1;
            uint64_t steps = current.steps | (1ULL << current.length);

            // This walk is finished, check if it's a polygon and follows the two-turn rule at the connection.
            if(length == N) {
                int is = isTaxiPolygon(steps, N);
                if(is && count == 5384903) {
                    out = steps;
                }
                count += is;
            }
            else {
                if(canTaxiPolygon(steps, length, N, stepsToOrigin)) {
                    untreated.emplace_back(length, steps);
                }
            }
        }
    }
    std::cout << toBinary(out, N) << std::endl;
    return count * 2;
}

void upTo(int start, int stop) {
    for(int n = start; n <= stop; n += 1)
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        std::cout << "\nn=" << n << ": " << taxiPolygon(n) << std::endl;
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