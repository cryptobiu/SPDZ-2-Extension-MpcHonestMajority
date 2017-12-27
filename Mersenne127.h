#pragma once

#include <gmp.h>

class Mersenne127
{
	mpz_t m_value;
public:
	Mersenne127(const char * value = NULL);
	Mersenne127(const mpz_t value);
	Mersenne127(const Mersenne127 & other);
	~Mersenne127();

    const Mersenne127 & operator = (const Mersenne127 & other);
    bool operator == (const Mersenne127 & other) const;
    bool operator != (const Mersenne127 & other) const;

    Mersenne127 operator + (const Mersenne127 & rha) const;
    Mersenne127 operator - (const Mersenne127 & rha) const;
    Mersenne127 operator * (const Mersenne127 & rha) const;
    Mersenne127 operator / (const Mersenne127 & rha) const;

    const Mersenne127 & operator += (const Mersenne127 & rha);
    const Mersenne127 & operator *= (const Mersenne127 & rha);

    static const char M127[];
};
