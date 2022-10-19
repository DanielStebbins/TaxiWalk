#include <iostream>
#include <list>

void getPoint(uint64_t steps, uint16_t length, int &x, int &y)
{
    int xStep = 1;
    int yStep = 1;
    for(int i = 0; i < length; i++)
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
    }
}

int main() {
    int x = 0;
    int y = 0;
    getPoint(0b0000000000000000000000000000000000000000000000000000000000000010, 10, x, y);
    std::cout << x << " " << y << std::endl;
    return 0;
}


