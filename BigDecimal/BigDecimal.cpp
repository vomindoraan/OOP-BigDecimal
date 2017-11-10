#include "BigDecimal.h"

#include <algorithm>  // max, min
#include <stdexcept>  // invalid_argument
#include <cmath>      // abs, log10

/*---------------------------*
 *  Stvaranje i uništavanje  *
 *---------------------------*/

// Pravi podrazumevani veliki broj (nula)
BigDecimal::BigDecimal() :
	sign('+'), digits(new digit[1]), length(1), dot(1)
{
	*digits = 0;
}

// Pravi veliki broj od celog broja
BigDecimal::BigDecimal(int num) :
	sign(num < 0 ? '-' : '+')
{
	num = std::abs(num);
	// broj cifara num = ⌊log₁₀(num)⌋ + 1
	length = dot = (num != 0) ? static_cast<count>(std::log10(num))+1 : 1;

	digits = new digit[length];
	numToDigits(digits, length, num);
}

// Pravi veliki broj od znakovnog niza (zanemaruje vodeće i prateće nule)
BigDecimal::BigDecimal(const char* num)
{
	if (num == nullptr)
		throw std::invalid_argument("num cannot be null");

	length = strLen(num);
	if (length == 0)
		throw std::invalid_argument("num cannot be an empty string");

	// Određuje znak
	sign = (*num == '-') ? '-' : '+';

	// Izostavlja znak i vodeće nule
	for (; strChr("+-0", *num); ++num, --length);

	// Nalazi tačku
	auto* pdot = strChr(num, '.');
	if (pdot)
		--length;  // Ako postoji, izostaviće se iz niza
	else
		pdot = num + length;  // Inače se podrazumeva na kraju broja

	// Izostavlja prateće nule
	auto* numEnd = num + strLen(num) - 1;
	for (auto* p = numEnd; p > pdot && *p == '0'; --p, --length);

	// Određuje broj cifara pre tačke
	dot = pdot - num;

	// Prepisuje cifre
	if (dot == 0) {
		// Ako nema cifara pre tačke, treba umetnuti 0 na početak
		digits = new digit[length+1];
		*digits = 0;
		copyDigits(digits+1, num+1, length);
		++length, ++dot;  // Uvećava dužinu za tu jednu nulu
	} else {
		digits = new digit[length];
		copyDigits(digits,     num,       dot);         // Cifre pre tačke
		copyDigits(digits+dot, num+dot+1, length-dot);  // Cifre posle tačke
	}

	// Sprečava -0
	if (isZero())
		sign = '+';
}

// Uslužni konstruktor: pravi veliki broj prepisivanjem datih podataka
BigDecimal::BigDecimal(char sign, const digit* digits, count length, count dot) :
	sign(sign), digits(new digit[length]), length(length), dot(dot)
{
	copyDigits(this->digits, digits, length);
	if (isZero())
		this->sign = '+';
}

// Pravi veliki broj kopiranjem
BigDecimal::BigDecimal(const BigDecimal& other) :
	BigDecimal(other.sign, other.digits, other.length, other.dot)  // Uslužni
{}

// Pravi veliki broj premeštanjem
BigDecimal::BigDecimal(BigDecimal&& other) :
	sign(other.sign), digits(other.digits), length(other.length), dot(other.dot)
{
	other.digits = nullptr;
}

// Uništava veliki broj
BigDecimal::~BigDecimal()
{
	delete[] digits;
	digits = nullptr;
	length = dot = 0;
}

/*-----------------------------*
 *  Pomeranje decimalne tačke  *
 *-----------------------------*/

// Vraća novi broj sa tačkom pomerenom ulevo za n mesta
BigDecimal BigDecimal::shl(count n) const
{
	if (n < 0)
		return shr(-n);

	if (n == 0 || isZero())
		return *this;

	auto rdot = dot - n;

	// Ako se broj završava nulom, treba ukloniti pratećih ≤n nula
	const auto* end = digits + length - 1;
	auto rlength = length;
	for (; *end == 0 && n > 0; --end, --n, --rlength);

	// Ako se tačka pomera samo unutar postojećih cifara, odmah vrati novi
	if (rdot > 0)
		return BigDecimal(sign, digits, rlength, rdot);

	// U suprotnom treba proširiti niz nulama s leve strane
	auto zeros = -rdot + 1;
	auto* rdigits = new digit[rlength+zeros];  // Pravi pomoćni niz

	// Upisuje vodeće nule na početak
	for (count i = 0; i < zeros; ++i)
		rdigits[i] = 0;

	// Prepisuje postojeće cifre
	copyDigits(rdigits+zeros, digits, rlength);

	auto&& result = BigDecimal(sign, rdigits, rlength+zeros, 1);
	delete[] rdigits;  // Briše pomoćni niz
	return result;
}

// Vraća novi broj sa tačkom pomerenom udesno za n mesta
BigDecimal BigDecimal::shr(count n) const
{
	if (n < 0)
		return shl(-n);

	if (n == 0 || isZero())
		return *this;

	// Ako broj počinje nulom, treba ukloniti vodećih ≤n nula
	const auto* start = digits;
	auto rlength = length;
	for (; *start == 0 && n > 0; ++start, --n, --rlength);

	auto rdot = dot + n;
	auto* rdigits = new digit[std::max(rlength, rdot)];  // Pravi pomoćni niz

	// Prepisuje postojeće cifre
	copyDigits(rdigits, start, rlength);

	// Ako je pomereno dalje od poslednje cifre, dopisuje prateće nule na kraj
	for (; rlength < rdot; ++rlength)
		rdigits[rlength] = 0;
	
	auto&& result = BigDecimal(sign, rdigits, rlength, rdot);
	delete[] rdigits;  // Briše pomoćni niz
	return result;
}

// Vraća novi broj sa uklonjenom tačkom (pamti za koliko je pomerena udesno u n)
BigDecimal BigDecimal::rmd(count* n) const
{
	auto oldLength = length, oldDot = dot;
	auto scale = length - dot;  // Broj decimala iza tačke
	auto&& result = shr(scale);

	if (n != nullptr)
		*n = oldLength - result.length + result.dot - oldDot;
	return result;
}

/*--------------------------*
 *  Sabiranje i oduzimanje  *
 *--------------------------*/

// Zajednička priprema podataka za sabiranje i oduzimanje
#define PREPARE(this, other)                            \
	count tn, on;                                       \
	auto trmd = this->rmd(&tn), ormd = other->rmd(&on); \
                                                        \
    /* Poravnava oba broja na najnižu cifru */          \
	auto n = std::max(tn, on);                          \
	auto tshr = trmd.shr(n-tn), oshr = ormd.shr(n-on);  \
                                                        \
	/* Podaci rezultujućeg broja */                     \
	auto rlength = std::max(tshr.length, oshr.length);  \
	auto rdot = std::max(this->dot, other->dot);        \
	auto rdigits = new digit[rlength];                  \
	digit carry = 0;                                    \

// Sabira drugi broj sa ovim i vraća zbir kao novi broj
BigDecimal BigDecimal::add(const BigDecimal* other) const
{
	if (isZero())
		return *other;
	if (other->isZero())
		return *this;

	char rsign;
	if (isPositive() && other->isPositive())
		rsign = '+';
	else if (isNegative() && other->isNegative())
		rsign = '-';
	else {
		auto ta = this->abs(), oa = other->abs();
		auto&& result = ta.greater(&oa) ? ta.sub(&oa) : oa.sub(&ta);
		rsign = ta.greater(&oa) ? this->sign : other->sign;
		return BigDecimal(rsign, result.digits, result.length, result.dot);
	}

	PREPARE(this, other)

	// Popunjava rezultujući niz sabirajući cifre otpozadi i pamteći prenos
	for (count t = tshr.length-1, o = oshr.length-1, r = rlength-1;
	     r >= 0; --t, --o, --r)
	{
		auto sum = (t >= 0 ? tshr.digits[t] : 0)
		         + (o >= 0 ? oshr.digits[o] : 0)
		         + carry;
		rdigits[r] = sum % 10;
		carry      = sum / 10;
	}

	// Ako je ostao prenos, treba proširiti niz za još jednu cifru
	if (carry > 0) {
		auto* old = rdigits;
		rdigits = new digit[rlength+1];
		*rdigits = carry;
		copyDigits(rdigits+1, old, rlength);
		++rlength, ++rdot;  // Uvećava dužinu za tu jednu cifru
		delete[] old;
	}

	auto&& result = BigDecimal(rsign, rdigits, rlength, rlength);
	delete[] rdigits;
	return result.shl(n);  // Pomeranje ulevo vraća tačku na mesto
}

// Oduzima drugi broj od ovog i vraća razliku kao novi broj
BigDecimal BigDecimal::sub(const BigDecimal* other) const
{
	if (isZero())
		return other->neg();
	if (other->isZero())
		return *this;

	auto ta = this->abs(), oa = other->abs();
	if (this->sign != other->sign) {
		auto&& result = ta.add(&oa);
		return BigDecimal(this->sign, result.digits, result.length, result.dot);
	}
	if (oa.greater(&ta)) {
		auto&& result = oa.sub(&ta);
		return BigDecimal(other->sign, result.digits, result.length, result.dot);
	}

	PREPARE(this, other)

	// Popunjava rezultujući niz oduzimajući cifre otpozadi i pamteći prenos
	for (count t = tshr.length-1, o = oshr.length-1, r = rlength-1;
	     r >= 0; --t, --o, --r)
	{
		auto sum = (t >= 0 ? tshr.digits[t] : 0)
		         - (o >= 0 ? oshr.digits[o] : 0)
		         - carry;
		rdigits[r] = (sum + 10) % 10;
		carry      =  sum < 0;
	}

	// Uklanja vodeće nule
	auto *start = rdigits;
	for (; *start == 0 && rlength > 1; ++start, --rlength, --rdot);

	auto&& result = BigDecimal(this->sign, start, rlength, rlength);
	delete[] rdigits;
	return result.shl(n);  // Pomeranje ulevo vraća tačku na mesto
}

/*-------------*
 *  Poređenje  *
 *-------------*/

// Ispituje da li je broj veći od drugog
bool BigDecimal::greater(const BigDecimal *other) const
{
	return !less(other) && !equals(other);
}

// Ispituje da li je broj manji od drugog
bool BigDecimal::less(const BigDecimal* other) const
{
	if ((isPositive() || isZero()) && (other->isNegative() || other->isZero()))
		return false;
	if (isNegative() && (other->isPositive() || other->isZero()) ||
			isZero() &&  other->isPositive())
		return true;

	if (dot > other->dot)
		return isNegative();  // Negativan ima više cifara → manji je
	if (dot < other->dot)
		return isPositive();  // Pozitivan ima manje cifara → manji je

	for (count i = 0; i < std::min(length, other->length); ++i) {
		auto td = this->digits[i], od = other->digits[i];
		if (td > od)
			return isNegative();  // Negativan ima veću cifru → manji je
		if (td < od)
			return isPositive();  // Pozitivan ima manju cifru → manji je
	}
	return false;
}

// Ispituje da li je broj jednak drugom
bool BigDecimal::equals(const BigDecimal* other) const
{
	if (sign != other->sign || length != other->length || dot != other->dot)
		return false;

	for (count i = 0; i < length; ++i)
		if (digits[i] != other->digits[i])
			return false;

	return true;
}

/*---------------------*
 *  Ispitivanje znaka  *
 *---------------------*/

// Ispituje da li je broj negativan
bool BigDecimal::isNegative() const
{
	return sign == '-';
}

// Ispituje da li je broj pozitivan
bool BigDecimal::isPositive() const
{
	return sign == '+' && !isZero();
}

// Ispituje da li je broj nula
bool BigDecimal::isZero() const
{
	return length == 1 && *digits == 0;
}

/*-----------------*
 *  Promena znaka  *
 *-----------------*/

// Vraća apsolutnu vrednost kao novi broj
BigDecimal BigDecimal::abs() const
{
	return BigDecimal('+', digits, length, dot);
}

// Vraća vrednost sa suprotnim znakom kao novi broj
BigDecimal BigDecimal::neg() const
{
	return BigDecimal(isPositive() ? '-' : '+', digits, length, dot);
}

/*---------------*
 *  Ispisivanje  *
 *---------------*/

// Ispisuje veliki broj na izlazni tok po formatu: [-]ceo[.razlomljen]
std::ostream& operator<<(std::ostream& os, const BigDecimal& bd)
{
	if (bd.isNegative())
		os << '-';
	for (count i = 0; i < bd.length; ++i) {
		if (i == bd.dot)
			os << (i == 0 ? "0." : ".");
		os << static_cast<char>(bd.digits[i] + '0');
	}
	return os;
}

/*--------------------------------------*
 *  Uslužne funkcije za rad sa ciframa  *
 *--------------------------------------*/

// Pretvara ceo broj u niz cifara
void BigDecimal::numToDigits(digit* dst, count len, count num)
{
	for (count i = len-1; i >= 0; --i) {
		dst[i] = num % 10;
		num /= 10;
	}
}

// Pretvara niz cifara u ceo broj
count BigDecimal::digitsToNum(const digit* src, count len)
{
	count num = 0;
	for (count i = 0; i < len; ++i)
		num = num * 10 + src[i];
	return num;
}

// Prepisuje cifre iz jednog niza cifara u drugi
void BigDecimal::copyDigits(digit* dst, const digit* src, count len)
{
	for (count i = 0; i < len; ++i)
		dst[i] = src[i];
}

// Prepisuje cifre iz stringa u niz cifara, uz pretvaranje znakova
void BigDecimal::copyDigits(digit* dst, const char* src, count len)
{
	for (count i = 0; i < len; ++i)
		dst[i] = src[i] - '0';
}

/*------------------------------*
 *  Neke funkcije iz <cstring>  *
 *------------------------------*/

// Traži prvo pojavljivanje znaka u stringu
const char* BigDecimal::strChr(const char* str, int chr)
{
	for (; *str && *str != chr; ++str);
	return *str ? str : nullptr;
}

// Vraća dužinu stringa
size_t BigDecimal::strLen(const char* str)
{
	size_t n = 0;
	for (; *str; ++n, ++str);
	return n;
}
