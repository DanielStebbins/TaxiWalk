#include <boost/multiprecision/cpp_int.hpp>
#include <iostream>

int main()
{
   using namespace boost::multiprecision;

   int128_t v = 1;

   // Do some fixed precision arithmetic:
   for(unsigned i = 1; i <= 20; ++i)
      v *= i;

   std::cout << v << std::endl; // prints 2432902008176640000 (i.e. 20!)

   // Repeat at arbitrary precision:
   cpp_int u = 1;
   for(unsigned i = 1; i <= 100; ++i)
      u *= i;

   // prints 93326215443944152681699238856266700490715968264381621468592963895217599993229915608941463976156518286253697920827223758251185210916864000000000000000000000000 (i.e. 100!)
   std::cout << u << std::endl;

   return 0;
}