#ifndef BIG_DECIMAL_H
#define BIG_DECIMAL_H

#include <iostream>

using digit = signed char;  // Cifra: jednobajtni ceo broj
#ifdef PROF_TEST
using count = int;          // Broj cifara: kompatibilno s potpisom rmd(int*)
#else
using count = long long;    // Broj cifara: najširi dostupan ceo broj (64b → 9×10¹⁸)
#endif

class BigDecimal {
public:
	/*---------------------* 
	 *  Osnovne operacije  *
	 *---------------------*/

	// Stvaranje i uništavanje
	BigDecimal(const char* num);
	BigDecimal(const BigDecimal& other);
	BigDecimal(BigDecimal&& other);
	~BigDecimal();

	// Zabrana dodele
	BigDecimal& operator=(const BigDecimal&) = delete;

	// Pomeranje decimalne tačke
	BigDecimal shl(count n)          const;
	BigDecimal shr(count n)          const;
	BigDecimal rmd(count* n=nullptr) const;

	// Sabiranje i oduzimanje
	BigDecimal add(const BigDecimal* other) const;
	BigDecimal sub(const BigDecimal* other) const;

	// Poređenje
	bool greater(const BigDecimal* other) const;
	bool less   (const BigDecimal* other) const;
	bool equals (const BigDecimal* other) const;

	// Apsolutna vrednost
	BigDecimal abs() const;

	// Ispisivanje
	friend std::ostream& operator<<(std::ostream& os, const BigDecimal& bd);

	/*---------------------*
	 *  Dodatne operacije  *
	 *---------------------*/

	// Dodatni konstruktori
	BigDecimal();
	BigDecimal(int num);

	// Ispitivanje znaka
	bool isNegative() const;
	bool isPositive() const;
	bool isZero()     const;

	// Suprotna vrednost
	BigDecimal neg() const;

	// Operatori za pomeranje tačke
	BigDecimal operator<<(count n) const { return shl(n); }
	BigDecimal operator>>(count n) const { return shr(n); }
	BigDecimal operator~ ()        const { return rmd();  }

	// Operatori za aritmetiku
	BigDecimal operator+(const BigDecimal& bd) const { return add(&bd); }
	BigDecimal operator-(const BigDecimal& bd) const { return sub(&bd); }
	
	// Operatori za poređenje
	bool operator> (const BigDecimal& bd) const { return greater(&bd); }
	bool operator< (const BigDecimal& bd) const { return less(&bd);    }
	bool operator==(const BigDecimal& bd) const { return equals(&bd);  }
	bool operator!=(const BigDecimal& bd) const { return !equals(&bd); }
	bool operator>=(const BigDecimal& bd) const { return greater(&bd) || equals(&bd); }
	bool operator<=(const BigDecimal& bd) const { return less(&bd)    || equals(&bd); }

	// Operatori za promenu znaka
	BigDecimal operator+() const { return abs(); }
	BigDecimal operator-() const { return neg(); }

private:
	/*------------*
	 *  Atributi  *
	 *------------*/

	char   sign;    // Znak broja ('+' ili '-')
	digit* digits;  // Niz značajnih cifara (bez decimalne tačke)
	count  length;  // Dužina niza
	count  dot;     // Mesto tačke (broj cifara ispred nje), uvek ≥1

	/*------------------*
	 *  Pomoćne metode  *
	 *------------------*/

	// Uslužni konstruktor
	BigDecimal(char sign, const digit* digits, count length, count dot);

	// Uslužne funkcije za rad sa ciframa
	static void  numToDigits(digit* dst, count len, count num);
	static count digitsToNum(const digit* src, count len);
	static void  copyDigits (digit* dst, const digit* src, count len);
	static void  copyDigits (digit* dst, const char*  src, count len);

	// Neke funkcije iz <cstring>
	static const char* strChr(const char* str, int chr);
	static size_t      strLen(const char* str);
};

#endif  // BIG_DECIMAL_H
