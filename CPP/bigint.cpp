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
#include <climits>

#include <bits/stdc++.h>

// Based on https://codeforces.com/blog/entry/22566.
// Must be a power of 2 for the % base to & (base - 1) trick to work.
// const uint64_t base = 1ULL << 62;
// const uint64_t base_exponent = 62;

// // Number of base-10 digits in 1ULL << 62 ???
// const int base_digits = 62;

// const int base = 1000000000;
// const int base_digits = 9;
// struct BigUnsigned {
//     std::vector<int> parts;

//     BigUnsigned(int initial) {
// 		*this = initial;
// 	}

//     void operator=(int v) {
// 		parts.clear();
// 		for (; v > 0; v = v / base)
// 			parts.push_back(v % base);
// 	}

//     BigUnsigned operator+(const BigUnsigned &v) const {
//         BigUnsigned res = v;

//         for (int i = 0, carry = 0; i < std::max(parts.size(), v.parts.size()) || carry; ++i) {
//             if (i == res.parts.size())
//                 res.parts.push_back(0);
//             res.parts[i] += carry + (i < parts.size() ? parts[i] : 0);
//             carry = res.parts[i] >= base;
//             if (carry)
//                 res.parts[i] -= base;
//         }
//         return res;
// 	}

//     void operator+=(const BigUnsigned &v) {
// 		*this = *this + v;
// 	}

//     bool isZero() const {
// 		return parts.empty() || (parts.size() == 1 && !parts[0]);
// 	}

//     // BigUnsigned operator+(const uint64_t &v) const {
//     //     BigUnsigned res = v;

//     //     for (int i = 0, carry = 0; i < parts.size() || carry; ++i) {
//     //         if (i == res.parts.size())
//     //             res.parts.push_back(0);
//     //         res.parts[i] += carry + (i < parts.size() ? parts[i] : 0);
//     //         carry = res.parts[i] >= base;
//     //         if (carry)
//     //             res.parts[i] -= base;
//     //     }
//     //     return res;
// 	// }

//     // void operator+=(const uint64_t &v) {
// 	// 	*this = *this + v;
// 	// }

//     std::string to_string(){
// 		std::stringstream ss;
// 		ss << *this;
// 		std::string s;
// 		ss >> s;
// 		return s;
// 	}

// 	friend std::ostream& operator<<(std::ostream &stream, const BigUnsigned &v) {
// 		stream << (v.parts.empty() ? 0 : v.parts.back());
// 		for (int i = v.parts.size() - 2; i >= 0; --i)
// 			stream << std::setw(base_digits) << std::setfill('0') << v.parts[i];
// 		return stream;
// 	}
// };

// Broken :(
using namespace std;

const int base = 1000000000;
const int base_digits = 9; 
struct bigint {
	vector<int> a;
	int sign;
	/*<arpa>*/
	int size(){
		if(a.empty())return 0;
		int ans=(a.size()-1)*base_digits;
		int ca=a.back();
		while(ca)
			ans++,ca/=10;
		return ans;
	}
	bigint operator ^(const bigint &v){
		bigint ans=1,a=*this,b=v;
		while(!b.isZero()){
			if(b%2)
				ans*=a;
			a*=a,b/=2;
		}
		return ans;
	}
	string to_string(){
		stringstream ss;
		ss << *this;
		string s;
		ss >> s;
		return s;
	}
	int sumof(){
		string s = to_string();
		int ans = 0;
		for(auto c : s)  ans += c - '0';
		return ans;
	}
	/*</arpa>*/
	bigint() :
		sign(1) {
	}
 
	bigint(long long v) {
		*this = v;
	}
 
	bigint(const string &s) {
		read(s);
	}
 
	void operator=(const bigint &v) {
		sign = v.sign;
		a = v.a;
	}
 
	void operator=(long long v) {
		sign = 1;
		a.clear();
		if (v < 0)
			sign = -1, v = -v;
		for (; v > 0; v = v / base)
			a.push_back(v % base);
	}
 
	bigint operator+(const bigint &v) const {
		if (sign == v.sign) {
			bigint res = v;
 
			for (int i = 0, carry = 0; i < (int) max(a.size(), v.a.size()) || carry; ++i) {
				if (i == (int) res.a.size())
					res.a.push_back(0);
				res.a[i] += carry + (i < (int) a.size() ? a[i] : 0);
				carry = res.a[i] >= base;
				if (carry)
					res.a[i] -= base;
			}
			return res;
		}
		return *this - (-v);
	}
 
	bigint operator-(const bigint &v) const {
		if (sign == v.sign) {
			if (abs() >= v.abs()) {
				bigint res = *this;
				for (int i = 0, carry = 0; i < (int) v.a.size() || carry; ++i) {
					res.a[i] -= carry + (i < (int) v.a.size() ? v.a[i] : 0);
					carry = res.a[i] < 0;
					if (carry)
						res.a[i] += base;
				}
				res.trim();
				return res;
			}
			return -(v - *this);
		}
		return *this + (-v);
	}
 
	void operator*=(int v) {
		if (v < 0)
			sign = -sign, v = -v;
		for (int i = 0, carry = 0; i < (int) a.size() || carry; ++i) {
			if (i == (int) a.size())
				a.push_back(0);
			long long cur = a[i] * (long long) v + carry;
			carry = (int) (cur / base);
			a[i] = (int) (cur % base);
			//asm("divl %%ecx" : "=a"(carry), "=d"(a[i]) : "A"(cur), "c"(base));
		}
		trim();
	}
 
	bigint operator*(int v) const {
		bigint res = *this;
		res *= v;
		return res;
	}
 
	void operator*=(long long v) {
		if (v < 0)
			sign = -sign, v = -v;
		if(v > base){
			*this = *this * (v / base) * base + *this * (v % base);
			return ;
		}
		for (int i = 0, carry = 0; i < (int) a.size() || carry; ++i) {
			if (i == (int) a.size())
				a.push_back(0);
			long long cur = a[i] * (long long) v + carry;
			carry = (int) (cur / base);
			a[i] = (int) (cur % base);
			//asm("divl %%ecx" : "=a"(carry), "=d"(a[i]) : "A"(cur), "c"(base));
		}
		trim();
	}
 
	bigint operator*(long long v) const {
		bigint res = *this;
		res *= v;
		return res;
	}
 
	friend pair<bigint, bigint> divmod(const bigint &a1, const bigint &b1) {
		int norm = base / (b1.a.back() + 1);
		bigint a = a1.abs() * norm;
		bigint b = b1.abs() * norm;
		bigint q, r;
		q.a.resize(a.a.size());
 
		for (int i = a.a.size() - 1; i >= 0; i--) {
			r *= base;
			r += a.a[i];
			int s1 = r.a.size() <= b.a.size() ? 0 : r.a[b.a.size()];
			int s2 = r.a.size() <= b.a.size() - 1 ? 0 : r.a[b.a.size() - 1];
			int d = ((long long) base * s1 + s2) / b.a.back();
			r -= b * d;
			while (r < 0)
				r += b, --d;
			q.a[i] = d;
		}
 
		q.sign = a1.sign * b1.sign;
		r.sign = a1.sign;
		q.trim();
		r.trim();
		return make_pair(q, r / norm);
	}
 
	bigint operator/(const bigint &v) const {
		return divmod(*this, v).first;
	}
 
	bigint operator%(const bigint &v) const {
		return divmod(*this, v).second;
	}
 
	void operator/=(int v) {
		if (v < 0)
			sign = -sign, v = -v;
		for (int i = (int) a.size() - 1, rem = 0; i >= 0; --i) {
			long long cur = a[i] + rem * (long long) base;
			a[i] = (int) (cur / v);
			rem = (int) (cur % v);
		}
		trim();
	}
 
	bigint operator/(int v) const {
		bigint res = *this;
		res /= v;
		return res;
	}
 
	int operator%(int v) const {
		if (v < 0)
			v = -v;
		int m = 0;
		for (int i = a.size() - 1; i >= 0; --i)
			m = (a[i] + m * (long long) base) % v;
		return m * sign;
	}
 
	void operator+=(const bigint &v) {
		*this = *this + v;
	}
	void operator-=(const bigint &v) {
		*this = *this - v;
	}
	void operator*=(const bigint &v) {
		*this = *this * v;
	}
	void operator/=(const bigint &v) {
		*this = *this / v;
	}
 
	bool operator<(const bigint &v) const {
		if (sign != v.sign)
			return sign < v.sign;
		if (a.size() != v.a.size())
			return a.size() * sign < v.a.size() * v.sign;
		for (int i = a.size() - 1; i >= 0; i--)
			if (a[i] != v.a[i])
				return a[i] * sign < v.a[i] * sign;
		return false;
	}
 
	bool operator>(const bigint &v) const {
		return v < *this;
	}
	bool operator<=(const bigint &v) const {
		return !(v < *this);
	}
	bool operator>=(const bigint &v) const {
		return !(*this < v);
	}
	bool operator==(const bigint &v) const {
		return !(*this < v) && !(v < *this);
	}
	bool operator!=(const bigint &v) const {
		return *this < v || v < *this;
	}
 
	void trim() {
		while (!a.empty() && !a.back())
			a.pop_back();
		if (a.empty())
			sign = 1;
	}
 
	bool isZero() const {
		return a.empty() || (a.size() == 1 && !a[0]);
	}
 
	bigint operator-() const {
		bigint res = *this;
		res.sign = -sign;
		return res;
	}
 
	bigint abs() const {
		bigint res = *this;
		res.sign *= res.sign;
		return res;
	}
 
	long long longValue() const {
		long long res = 0;
		for (int i = a.size() - 1; i >= 0; i--)
			res = res * base + a[i];
		return res * sign;
	}
 
	friend bigint gcd(const bigint &a, const bigint &b) {
		return b.isZero() ? a : gcd(b, a % b);
	}
	friend bigint lcm(const bigint &a, const bigint &b) {
		return a / gcd(a, b) * b;
	}
 
	void read(const string &s) {
		sign = 1;
		a.clear();
		int pos = 0;
		while (pos < (int) s.size() && (s[pos] == '-' || s[pos] == '+')) {
			if (s[pos] == '-')
				sign = -sign;
			++pos;
		}
		for (int i = s.size() - 1; i >= pos; i -= base_digits) {
			int x = 0;
			for (int j = max(pos, i - base_digits + 1); j <= i; j++)
				x = x * 10 + s[j] - '0';
			a.push_back(x);
		}
		trim();
	}
 
	friend istream& operator>>(istream &stream, bigint &v) {
		string s;
		stream >> s;
		v.read(s);
		return stream;
	}
 
	friend ostream& operator<<(ostream &stream, const bigint &v) {
		if (v.sign == -1)
			stream << '-';
		stream << (v.a.empty() ? 0 : v.a.back());
		for (int i = (int) v.a.size() - 2; i >= 0; --i)
			stream << setw(base_digits) << setfill('0') << v.a[i];
		return stream;
	}
 
	static vector<int> convert_base(const vector<int> &a, int old_digits, int new_digits) {
		vector<long long> p(max(old_digits, new_digits) + 1);
		p[0] = 1;
		for (int i = 1; i < (int) p.size(); i++)
			p[i] = p[i - 1] * 10;
		vector<int> res;
		long long cur = 0;
		int cur_digits = 0;
		for (int i = 0; i < (int) a.size(); i++) {
			cur += a[i] * p[cur_digits];
			cur_digits += old_digits;
			while (cur_digits >= new_digits) {
				res.push_back(int(cur % p[new_digits]));
				cur /= p[new_digits];
				cur_digits -= new_digits;
			}
		}
		res.push_back((int) cur);
		while (!res.empty() && !res.back())
			res.pop_back();
		return res;
	}
 
	typedef vector<long long> vll;
 
	static vll karatsubaMultiply(const vll &a, const vll &b) {
		int n = a.size();
		vll res(n + n);
		if (n <= 32) {
			for (int i = 0; i < n; i++)
				for (int j = 0; j < n; j++)
					res[i + j] += a[i] * b[j];
			return res;
		}
 
		int k = n >> 1;
		vll a1(a.begin(), a.begin() + k);
		vll a2(a.begin() + k, a.end());
		vll b1(b.begin(), b.begin() + k);
		vll b2(b.begin() + k, b.end());
 
		vll a1b1 = karatsubaMultiply(a1, b1);
		vll a2b2 = karatsubaMultiply(a2, b2);
 
		for (int i = 0; i < k; i++)
			a2[i] += a1[i];
		for (int i = 0; i < k; i++)
			b2[i] += b1[i];
 
		vll r = karatsubaMultiply(a2, b2);
		for (int i = 0; i < (int) a1b1.size(); i++)
			r[i] -= a1b1[i];
		for (int i = 0; i < (int) a2b2.size(); i++)
			r[i] -= a2b2[i];
 
		for (int i = 0; i < (int) r.size(); i++)
			res[i + k] += r[i];
		for (int i = 0; i < (int) a1b1.size(); i++)
			res[i] += a1b1[i];
		for (int i = 0; i < (int) a2b2.size(); i++)
			res[i + n] += a2b2[i];
		return res;
	}
 
	bigint operator*(const bigint &v) const {
		vector<int> a6 = convert_base(this->a, base_digits, 6);
		vector<int> b6 = convert_base(v.a, base_digits, 6);
		vll a(a6.begin(), a6.end());
		vll b(b6.begin(), b6.end());
		while (a.size() < b.size())
			a.push_back(0);
		while (b.size() < a.size())
			b.push_back(0);
		while (a.size() & (a.size() - 1))
			a.push_back(0), b.push_back(0);
		vll c = karatsubaMultiply(a, b);
		bigint res;
		res.sign = sign * v.sign;
		for (int i = 0, carry = 0; i < (int) c.size(); i++) {
			long long cur = c[i] + carry;
			res.a.push_back((int) (cur % 1000000));
			carry = (int) (cur / 1000000);
		}
		res.a = convert_base(res.a, 6, base_digits);
		res.trim();
		return res;
	}
};

struct State {
    uint64_t var1;
    uint64_t var2;
    State *children[2]{};
    bigint count1;
    bigint count2;

    State(uint16_t length, uint64_t steps):
        var1(length), var2(steps), children{nullptr,nullptr}, count1(0), count2(0) {}
};

// std::string toBinary(uint64_t n, uint16_t len)
// {
//     if(len == 0) {
//         return "Origin";
//     }
//     std::string binary;
//     for(uint16_t i = 0; i < len; i++)
//     {
//         if((n >> i) & 1)
//         {
//             binary += "V";
//         }
//         else
//         {
//             binary += "H";
//         }
//     }
//     return binary;
// }

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
        }
        ++untreated;
    }

    // Reset first 16 bytes to use when running the automaton.
    // Remove if pattern becomes separated from running count.
    // for(auto & state : states)
    // {
    //     state.var1 = 0;
    //     state.var2 = 0;
    // }
    return states;
}

uint64_t taxi(int N)
{
    std::vector<State> automaton = makeAutomaton(N);

    // Sets "H" to current count 1.
    automaton[1].count1 = 1;

    // Ends one step early, because on the final loop there's no need to move var2 to var1.
    for(int n = 2; n < 95; ++n)
    {
        for(auto & state : automaton)
        {
            if(!state.count1.isZero())
            {
                if(state.children[0])
                {
                    (*state.children[0]).count2 += state.count1;
                }
                if(state.children[1])
                {
                    (*state.children[1]).count2 += state.count1;
                }
            }
        }
        for(auto & state : automaton)
        {
            state.count1 = state.count2;
            state.count2 = 0;
        }
    }

//    std::cout <<"Number of states for length " << N << ": " << automaton.size() << std::endl;

    bigint taxiWalks = 0;
    // uint64_t taxiWalks = 0;
    for(auto & state : automaton)
    {
        if(!state.count1.isZero())
        {
            if(state.children[0])
            {
                taxiWalks += state.count1;
            }
            if(state.children[1])
            {
                taxiWalks += state.count1;
            }
        }
    }
    std::cout << taxiWalks << std::endl;
    return 0;
    // return taxiWalks << 1;
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
    // BigUnsigned x = 1ULL << 63;
    // BigUnsigned y = 4083094823042ULL;
    // x += x;
    // x += x;
    // x += x;
    // x += x;
    // std::cout << x << std::endl;
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