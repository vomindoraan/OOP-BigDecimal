#include "BigDecimal.h"

#include <iostream>
#include <ctime>

#if defined PROF_TEST
void test();

#elif defined STUDENT_TEST
#define VALUES { \
	"0", "0.", "+0.0", ".0", "163.445", "+000163.44500", \
	"0.5", ".5", "4", "-0.", "30", "+18.11131", "-41.5", \
}

void test()
{
	std::cout << "from __future__ import print_function\n\n";

	const char* raw[] = VALUES;
	BigDecimal  arr[] = VALUES;
	constexpr auto n = sizeof arr / sizeof *arr;

	for (size_t i = 0; i < n; ++i)
		for (size_t j = 0; j < n; ++j) {
			auto& a = arr[i];
			auto& b = arr[j];
			std::cout << "print((" << a << " + "  << b << ") == " << (a + b)  << ")\n";
			std::cout << "print((" << a << " - "  << b << ") == " << (a - b)  << ")\n";
			std::cout << "print((" << a << " < "  << b << ") == " << (a < b)  << ")\n";
			std::cout << "print((" << a << " > "  << b << ") == " << (a > b)  << ")\n";
			std::cout << "print((" << a << " == " << b << ") == " << (a == b) << ")\n";
			std::cout << "print((" << a << " != " << b << ") == " << (a != b) << ")\n";
			std::cout << "print((" << a << " <= " << b << ") == " << (a <= b) << ")\n";
			std::cout << "print((" << a << " >= " << b << ") == " << (a >= b) << ")\n";
		}
}
#endif  // STUDENT_TEST

int main()
{
	auto start = std::clock();

	test();

	auto ms = (std::clock() - start) * 1000.0 / CLOCKS_PER_SEC;
	std::cout << "\nprint('Trajanje: ', " << ms << ", ' ms')\n";
}
