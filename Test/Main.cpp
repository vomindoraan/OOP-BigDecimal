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
#define SEP "    "

void test()
{
	const char* raw[] = VALUES;
	BigDecimal  arr[] = VALUES;
	constexpr auto n = sizeof arr / sizeof *arr;

	for (const auto& bd : arr)
		std::cout << bd << " ";
	std::cout << "\n";

	for (size_t i = 0; i < n; ++i)
		for (size_t j = 0; j < n; ++j) {
			std::cout << "\n" << raw[i] << ", " << raw[j] << "\n";
			auto& a = arr[i];
			auto& b = arr[j];
			std::cout << "a+b: " << (a + b)  << SEP;
			std::cout << "a-b: " << (a - b)  << SEP;
			std::cout << "a>b: " << (a > b)  << SEP;
			std::cout << "a<b: " << (a < b)  << SEP;
			std::cout << "a=b: " << (a == b) << SEP;
			std::cout << "a®4: " << (a << 4) << ", b®4: " << (b << 4) << SEP;
			std::cout << "a¯4: " << (a >> 4) << ", b¯4: " << (b >> 4) << SEP;
			std::cout << "~a: "  << ~a << ", ~b: "  << ~b << SEP;
			std::cout << "|a|: " << +a << ", |b|: " << +b << SEP;
			std::cout << "-a: "  << -a << ", -b: "  << -b << "\n";
		}

	BigDecimal zero;
	BigDecimal fourtyTwo = 42;
	std::cout << zero << " " << fourtyTwo << " " << (zero <= fourtyTwo) << "\n";
	std::cout << (zero == 0) << " " << (fourtyTwo >= 0) << "\n";
}
#endif  // STUDENT_TEST

int main()
{
	auto start = std::clock();

	test();

	auto ms = (std::clock() - start) * 1000.0 / CLOCKS_PER_SEC;
	std::cout << "\nTrajanje: " << ms << " ms\n";
}
