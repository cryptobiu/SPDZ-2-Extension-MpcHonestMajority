#pragma once

#include <gmp.h>

class Mersenne127
{
	mpz_t m_value;
public:
	Mersenne127(const char * value = NULL);
	Mersenne127(const mpz_t value);
	Mersenne127(const Mersenne127 & other);
	Mersenne127(int);
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

    const mpz_t * get_mpz_t() const { return &m_value; }
    void set_mpz_t(const mpz_t * value) { mpz_set(m_value, *value); }

    friend std::ostream& operator << (std::ostream& s, const Mersenne127 & m127);
    friend std::istream& operator >> (std::istream& s, Mersenne127 & m127);

    static const char M127[];
};
