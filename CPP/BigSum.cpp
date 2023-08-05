#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <climits>

std::string toBinary(uint64_t n, uint16_t len)
{
    if(len == 0) {
        return "Origin";
    }
    std::string binary;
    for(uint16_t i = 0; i < len; i++)
    {
        if((n >> (len - 1 - i)) & 1)
        {
            binary += "1";
        }
        else
        {
            binary += "0";
        }
    }
    return binary;
}

// Bits go right to left, segments go left to right.
const uint64_t max_bit = 1ULL << 63;
struct BigSum {
    std::vector<uint64_t> segments;
    
    BigSum(uint64_t value = 0) {
        segments.push_back(value);
        if(segments[0] & max_bit) {
            segments[0] &= ~max_bit;
            segments.push_back(1);
            std::cout << "BigSum created with carry." << std::endl;
        }
    }

    void operator=(const BigSum &other) {
		segments = other.segments;
	}
 
	void operator=(uint64_t value) {
		segments.clear();
		segments.push_back(value);
        if(segments[0] & max_bit) {
            segments[0] &= ~max_bit;
            segments.push_back(1);
            std::cout << "BigSum assigned with carry." << std::endl;
        }
	}

    BigSum operator+(const BigSum &other) const {
        BigSum result = other;
        for(int i = 0, carry = 0; i < std::max(segments.size(), other.segments.size()) || carry; ++i) {
            if(i == result.segments.size()) {
                result.segments.push_back(0);
            }
            // if(result.segments[i] > ULLONG_MAX - carry - (i < segments.size() ? segments[i] : 0)) {
            //     std::cout << "OVERFLOW: " << carry << " + " << segments[i] << " + " <<  other.segments[i] << std::endl;
            // }
            result.segments[i] += carry + (i < segments.size() ? segments[i] : 0);
            carry = result.segments[i] >= max_bit;
            // if(carry) {
            //     std::cout << "CARRY: " << carry << " + " << segments[i] << " + " <<  other.segments[i] << std::endl;
            // }
            result.segments[i] &= ~max_bit;
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
			    out += toBinary(segments[i], 63);
            }

            // Avoid leading 0s.
            int i = 0;
            while(i < out.length() && out[i] == '0') {
                ++i;
            }
            return out.substr(i);
            return out;
        }
    }

    friend std::ostream& operator<<(std::ostream &stream, const BigSum &bigsum) {
        stream << bigsum.to_string();
        return stream;
	}
};

struct State {
    BigSum var1;
    BigSum var2;
    State *children[2]{};

    State(uint64_t length, uint64_t steps):
        var1(length), var2(steps), children{nullptr,nullptr} {}
};



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
        if(approach(states[untreated].var2.segments[0], states[untreated].var1.segments[0]) != 1 || states[untreated].var1.segments[0] < 2)
        {
            uint16_t length = states[untreated].var1.segments[0] + 1;
            uint64_t steps = states[untreated].var2.segments[0];

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
        }

        // Vertical Step.
        if(approach(states[untreated].var2.segments[0], states[untreated].var1.segments[0]) != 2 || states[untreated].var1.segments[0] < 2)
        {
            uint16_t length = states[untreated].var1.segments[0] + 1;
            uint64_t steps = states[untreated].var2.segments[0] | (1ULL << states[untreated].var1.segments[0]);

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
        }
        ++untreated;
    }

    // Reset each state's number variables so they're no longer pattern and length, instead automaton iteration counts.
    for(auto & state : states)
    {
        state.var1 = 0;
        state.var2 = 0;
    }
    return states;
}

BigSum taxi(int automaton_size, int num_iterations)
{
    std::vector<State> automaton = makeAutomaton(automaton_size);

    std::cout << "Automaton Generated." << std::endl;

    // Sets "H" to current count 1.
    automaton[1].var1 = 1;

    // Ends one step early, because on the final loop there's no need to move var2 to var1.
    for(int n = 2; n < num_iterations; ++n)
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

        if((n + 1) % 50 == 0) {
            // std::cout << "Completed iteration " << n + 1 << " of " << num_iterations << "." << std::endl;
            BigSum taxiWalks = 0;
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
            std::cout << "A=" << automaton_size << ", I=" << n + 1 << ": " << taxiWalks << '0' << std::endl;
        }
    }
    return 0;

    // std::cout << "Computing final sum..." << std::endl;

    // BigSum taxiWalks = 0;
    // for(auto & state : automaton)
    // {
    //     if(state.var1)
    //     {
    //         if(state.children[0])
    //         {
    //             taxiWalks += state.var1;
    //         }
    //         if(state.children[1])
    //         {
    //             taxiWalks += state.var1;
    //         }
    //     }
    // }
    // return taxiWalks;
}

void run(int automaton_size, int num_iterations)
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    BigSum t = taxi(automaton_size, num_iterations);

    // << '0' << is to "multiply by 2". Only works for binary outputs.
    // std::cout << "A=" << automaton_size << ", I=" << num_iterations << ": " << t << '0' << std::endl;

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    double totalTime = (double)(end - begin).count() / 1000000000.0;
    std::cout << "Total Time: " << totalTime << std::endl;
}

int main(int argc, char *argv[])
{
    // Automaton size must not be more than 63 because the automaton generator still uses only 64 bits of each state's variables.
    if(argc == 2)
    {
        int N = atoi(argv[1]);
        run(N, N);
    }
    else if(argc == 3)
    {
        int automaton_size = atoi(argv[1]);
        int num_iterations = atoi(argv[2]);
        run(automaton_size, num_iterations);
    }
    return 0;
}