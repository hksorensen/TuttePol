#pragma once

enum POLY_TYPES { POLY_UNDEF, POLY_SUM, POLY_TERM, POLY_MULTITERM };

#include <stdio.h>

class Polynomial
{
public:
	Polynomial(POLY_TYPES type, Polynomial* p1, Polynomial* p2);
	Polynomial(POLY_TYPES type, uint64 coef, int xpower, int ypower);
	Polynomial(uint64 coef, int xpower, int ypower);
	Polynomial(uint64* coefs, int minx, int miny, int maxx, int maxy, int xpower=0);
public:
	~Polynomial(void);

public:
	bool Print(char* filename);
	virtual bool Print(FILE* fp);

	Polynomial* Simplify();

	Polynomial* Clone();
	void UpdateCoefficients(uint64* coefs);

	void MultiplyByX();

	void SetHeight();

public:
	Polynomial* m_p1;
	Polynomial* m_p2;
	uint64 m_coef;
	int m_xpower;
	int m_ypower;
	long int m_height;
	uint64* m_coefs;

private:
	int m_maxx;
	int m_maxy;
	int m_minx;
	int m_miny;
public:
	uint64 AddCoefficient(int x, int y, uint64 coef);
	bool SetCoefficient(int x, int y, uint64 coef);
	uint64 GetCoefficient(int x, int y);
	POLY_TYPES m_type;
};
