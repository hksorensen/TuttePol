//*\noindent\verb!// $Header: /home/cvs/cvsroot/MSVC/TuttePol/Polynomial.cpp,v 1.19 2006/12/21 00:42:14 hkragh Exp $!

#include "StdAfx.h"
#include "Polynomial.h"
#include <stdlib.h>
#include <string.h>			// Required for memcpy
#include <stdio.h>

#define GB_TRIGGER 25

extern char global_maxx;
extern char global_maxy;

bool PrintTerm(FILE* fp, uint64 coef, int xpower, int ypower)
{
	if (coef==0) { return false; }
	else if (coef==1) { }
	else { fprintf(fp, "%I64Lu", coef); }
	
	if (xpower==0) { }
	else if (xpower==1) { fprintf(fp, "x"); }
	else if (xpower<10) { fprintf(fp, "x^%d", xpower); }
	else { fprintf(fp, "x^{%d}", xpower); }
	
	if (ypower==0) { }
	else if (ypower==1) { fprintf(fp, "y"); }
	else if (ypower<10) { fprintf(fp, "y^%d", ypower); }
	else { fprintf(fp, "y^{%d}", ypower); }
	return true;
}

bool GBC(int height) {
	if (0) {
		return (height % GB_TRIGGER == GB_TRIGGER-1);
	} else {
		return (height>=20);
	}
}

Polynomial::Polynomial(POLY_TYPES type, Polynomial* p1, Polynomial* p2)
{
	m_type=type;
	m_coefs=NULL;

	if (GBC(p1->m_height)) {
		fprintf(stderr, ".");
		Polynomial* q = p1->Simplify();
		delete p1;
		p1 = q;
	}
	if (GBC(p2->m_height)) {
		fprintf(stderr, ".");
		Polynomial* q = p2->Simplify();
		delete p2;
		p2 = q;
	}

	m_p1 = p1;
	m_p2 = p2;
	SetHeight();
}

Polynomial::Polynomial(POLY_TYPES type, uint64 coef, int xpower, int ypower)
{
	m_type=POLY_TERM;
	m_p1 = NULL;
	m_p2 = NULL;
	m_coef = coef;
	m_coefs=NULL;
	m_xpower = xpower;
	m_ypower = ypower;
	m_height=0;
}

Polynomial::Polynomial(uint64 coef, int xpower, int ypower)
{
	m_type=POLY_TERM;
	m_p1 = NULL;
	m_p2 = NULL;
	m_coef = coef;
	m_coefs=NULL;
	m_xpower = xpower;
	m_ypower = ypower;
	m_height=0;
}

Polynomial::Polynomial(uint64* coefs, int minx, int miny, int maxx, int maxy, int xpower)
{
	m_type = POLY_MULTITERM;
	m_coefs = coefs;
	m_p1 = NULL;
	m_p2 = NULL;
	m_coef = 0;
	m_xpower = xpower;
	m_ypower = 0;
	m_height=0;
	m_minx = minx;
	m_miny = miny;
	m_maxx = maxx;
	m_maxy = maxy;
}

Polynomial::~Polynomial(void)
{
	if (m_p1) { delete m_p1; m_p1 = NULL; }
	if (m_p2) { delete m_p2; m_p2 = NULL; }
	if (m_coefs != NULL) { free(m_coefs); m_coefs=NULL; }
}

bool Polynomial::Print(char* filename)
{
	FILE* fp = fopen(filename, "w");
	if (fp == NULL) {
		fprintf(stderr, "Error opening file %s\n", filename);
		return false;
	}
	Print(fp);
	fclose(fp);
	return true;
}

bool Polynomial::Print(FILE* fp)
{
	if (m_type == POLY_SUM) {
		m_p1->Print(fp);
		fprintf(fp, "\n+");
		m_p2->Print(fp);
	} else if (m_type == POLY_TERM) {
		return PrintTerm(fp, m_coef, m_xpower, m_ypower);
	} else if (m_type == POLY_MULTITERM) {
		bool notfirst = false;
		for (int x=m_maxx; x>=m_minx; x--) {
			for (int y=m_maxy; y>=m_miny; y--) {
				uint64 coef = this->GetCoefficient(x,y);
				if ((coef!=0) && (notfirst)) {
					fprintf (fp, "\n+");
				}
				notfirst = notfirst | PrintTerm(fp, coef, x, y);
			}
		}
	}
	return true;
}

int update_maxx;
int update_maxy;
int update_minx;
int update_miny;

Polynomial* Polynomial::Simplify()
{
	uint64* coefs = (uint64*)calloc((global_maxx+1)*(global_maxy+1),sizeof(uint64));
	update_maxx = update_maxy = 0;
	update_minx = global_maxx+1;
	update_miny = global_maxy+1;

	UpdateCoefficients(coefs);

	uint64* ac = (uint64*)calloc((update_maxx-update_minx+1)*(update_maxy-update_miny+1),sizeof(uint64));
	for (int x=update_minx; x<=update_maxx; x++) {
		for (int y=update_miny; y<=update_maxy; y++) {
			ac[(x-update_minx)*(update_maxy-update_miny+1)+(y-update_miny)] = coefs[x*(global_maxy+1)+y];
		}
	}

	free(coefs); 
	coefs=NULL;
	return new Polynomial(ac, update_minx, update_miny, update_maxx, update_maxy, 0);
}

void Polynomial::UpdateCoefficients(uint64* coefs)
{
	if (m_type == POLY_SUM) {
		m_p1->UpdateCoefficients(coefs);
		m_p2->UpdateCoefficients(coefs);
	} else if (m_type == POLY_TERM) {
		coefs[m_xpower*(global_maxy+1)+m_ypower]+=m_coef;
		update_maxx = __max(update_maxx, m_xpower);
		update_maxy = __max(update_maxy, m_ypower);
		update_minx = __min(update_minx, m_xpower);
		update_miny = __min(update_miny, m_ypower);
	} else if (m_type == POLY_MULTITERM) {
		for (int x=m_minx; x<=m_maxx; x++) {
			for (int y=m_miny; y<=m_maxy; y++) {
				if (this->GetCoefficient(x,y) != 0) {
					// Potential overrun if invariants fail
					if ( (x+m_xpower<0) || (x+m_xpower>global_maxx) || (y<0) || (y>global_maxy) ) {
						fprintf (stderr, "Overrun in UpdateCoefficients: %d,%d - %d - %d,%d - %d,%d,%d,%d\n",
							x, y,
							m_xpower,
							global_maxx, global_maxy,
							m_minx, m_maxx, m_miny, m_maxy);
					}
					// x+m_xpower to implement MultiplyByX for MULTITERM
					coefs[(x+m_xpower)*(global_maxy+1)+y] += this->GetCoefficient(x,y);
					update_maxx = __max(update_maxx, x+m_xpower);
					update_maxy = __max(update_maxy, y);
					update_minx = __min(update_minx, x+m_xpower);
					update_miny = __min(update_miny, y);
				}
			}
		}
	}
	else {
		// This is an error!
		fprintf(stderr, "Error in UpdateCoefficient\n");
	}
}

Polynomial* Polynomial::Clone()
{
	if (m_type == POLY_TERM) {
		return new Polynomial(m_coef, m_xpower, m_ypower);
	} else if (m_type == POLY_MULTITERM) {
		uint64* newcoefs = (uint64*)malloc((m_maxx-m_minx+1)*(m_maxy-m_miny+1)*sizeof(uint64));
		memcpy(newcoefs, m_coefs, (m_maxx-m_minx+1)*(m_maxy-m_miny+1)*sizeof(uint64));
		return new Polynomial(newcoefs, this->m_minx, this->m_miny, this->m_maxx, this->m_maxy, this->m_xpower);
	} else {
		Polynomial* p1 = m_p1->Clone();
		Polynomial* p2 = m_p2->Clone();
		return new Polynomial(m_type, p1, p2);
	}
}

void Polynomial::MultiplyByX()
{
	if (m_type == POLY_TERM) {
		m_xpower++;
	} 
	else if (m_type == POLY_SUM) {
		m_p1->MultiplyByX();
		m_p2->MultiplyByX();
	}
	else if (m_type == POLY_MULTITERM) {
		m_xpower++;
	}
}

void Polynomial::SetHeight()
{
	if (m_p1->m_height>m_p2->m_height) {
		m_height = m_p1->m_height+1;
	} else {
		m_height = m_p2->m_height+1;
	} 
}

uint64 Polynomial::GetCoefficient(int x, int y)
{
	if (this->m_type != POLY_MULTITERM) {
		fprintf (stderr, "GetCoefficient called on non-multi-term (%d,%d)\n", x, y);
		return -1;
	}
	if ( (x>m_maxx) || (x<m_minx) || (y>m_maxy) || (y<m_miny) ) {
		return 0;
	} else {
		return m_coefs[(x-m_minx)*(m_maxy-m_miny+1)+(y-m_miny)];
	}
}

bool Polynomial::SetCoefficient(int x, int y, uint64 coef)
{
	if ( (x>m_maxx) || (x<m_minx) || (y>m_maxy) || (y<m_miny) ) {
		return false;
	} else {
		m_coefs[(x-m_minx)*(m_maxy-m_miny+1)+(y-m_miny)] = coef;
		return true;
	}
}

uint64 Polynomial::AddCoefficient(int x, int y, uint64 coef)
{
	if ( (x>m_maxx) || (x<m_minx) || (y>m_maxy) || (y<m_miny) ) {
		return 0;
	} else {
		m_coefs[(x-m_minx)*(m_maxy-m_miny+1)+(y-m_miny)] += coef;
		return m_coefs[(x-m_minx)*(m_maxy-m_miny+1)+(y-m_miny)];
	}
}
