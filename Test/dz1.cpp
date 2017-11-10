// An envelope header file containing include directives for all students' headers.
#include "dz1.h"
#include <iostream>

using namespace std;

#ifdef PROF_TEST

void test(){
	BigDecimal a("2315.434\0");// a = 2315.434
	BigDecimal b("-135\0"); // b = -135
	BigDecimal c = a.shr(4); // c = 23154340
	BigDecimal d = b.shl(1); // d = -13.5
	int n;
	BigDecimal e = a.rmd(&n); // e = 2315434, n = 3
	BigDecimal f = d.add(&e); // f = 2315420.5
	BigDecimal g = a.sub(&c); // g = -23152024.566
	if(e.greater(&g))
		cout << "TACNO" << endl;
	else
		cout << "NETACNO" << endl;
	
	if(e.less(&g))
		cout << "NETACNO" << endl;
	else
		cout << "TACNO" << endl;

	if(!g.equals(&g)) cout << "GRESKA";
	BigDecimal h = b.abs();

	cout << "a = " << a << endl << "b = " << b << " c = " << c << endl;
	cout << "d = " << d << " e = " << e << " f = " << f << " g = " << g << endl;
	cout << "h = " << h << endl;
	cout << "n = " << n << endl;
	system("pause");
}

#endif