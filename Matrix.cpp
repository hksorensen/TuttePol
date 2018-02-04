//*\noindent\verb!// $Header: /home/cvs/cvsroot/MSVC/TuttePol/Matrix.cpp,v 1.22 2006/12/21 00:42:14 hkragh Exp $!

#include "StdAfx.h"
#include "Matrix.h"

#include <string.h>
#include <stdlib.h>

unsigned char BYTEINDEX[8] = { 1, 2, 4, 8, 16, 32, 64, 128};
unsigned char BYTEINDEXACC[8] = { 0, 1, 3, 7, 15, 31, 63, 127};

extern char global_maxx;
extern char global_maxy;

#include "PolyStash.h"
extern PolyStash* ps;


Matrix::Matrix(void)
{
	m_data = NULL;
	m_rows = 0;
	m_columns = 0;
}

Matrix::~Matrix(void)
{
	if (m_data != NULL) {
		free(m_data);
	}
}

Polynomial* Matrix::ComputeTuttePol() //* \label{Matrix:ComputeTuttePol}
{
	COLROWTYPE r = this->GetDimensionRows();
	COLROWTYPE k = this->GetDimensionColumns();

	if (r==1) {
		int nl = 0;
		for (COLROWTYPE j=1; j<=k; j++) {
			if (this->GetElement(j-1,0)==0) { nl++; }
		}
		Polynomial* p = new Polynomial(1,1,nl);	// $x*y^{nl}$
		for (COLROWTYPE i=nl+1; i<=r+k-1; i++) {
			p = new Polynomial(POLY_SUM, p, new Polynomial(1,0,i));		
				// $y^i$ for $i=nl+1, \dots, r+k-1$
		}
		return p; // $x*y^{nl} + y^{nl+1} + \dots + y^{r+k-1}$
	}

	if (k==1) {
		int ncl=0;
		for (COLROWTYPE j=1; j<=r; j++) {
			if (this->GetElement(0,j-1)==0) { ncl++; }
		}
		Polynomial* p = new Polynomial(1,ncl,1);	// $x^{ncl}*y$
		for (COLROWTYPE i=ncl+1; i<=r+k-1; i++) {
			p = new Polynomial(POLY_SUM, p, new Polynomial(1,i,0));		
				// $x^i$ for $i=ncl+1, \dots, r+k-1$
		}
		return p;
	}

	STASHINDEX key = this->ComputeStashIndex(1);

	if ( ps->CheckStash(key) ) {
		return ps->GetPoly(key);
	}

	COLROWTYPE j=1;
	while ((j<=k) && (this->GetElement(j-1,0)==0)) { j++; }

	if (j==k+1) {
		Matrix B;
		B.ComputePluckMatrix(this,1,0);	// Remove first row
		// $x*TuttePol(B)$
		Polynomial* p = B.ComputeTuttePol();
		p->MultiplyByX();
		return ps->SetStash(key, p);
	}

	if (j==k) {
		Matrix B;
		B.ComputePluckMatrix(this,1,0);	// Remove first row
		Matrix C;
		C.ComputePluckMatrix(this,1,k);	// Remove first row and $k$'th (=$j$'th) column
		Polynomial* p = C.ComputeTuttePol();
		p->MultiplyByX();
		return ps->SetStash(key, new Polynomial(POLY_SUM, B.ComputeTuttePol(), p));
	}


	bool nc=true;
	for (COLROWTYPE jj=j+1; jj<=k; jj++) {
		if (this->GetElement(jj-1,0)!=0) { nc=false; }
	}

	Matrix B;
	B.ComputePluckMatrix(this,1,0);		// Remove first row

	if ( nc ) { 
		Matrix C;
		C.ComputePluckMatrix(this,1,j);	// Remove first row and $j$'th column
		Polynomial* p = C.ComputeTuttePol();
		p->MultiplyByX();
		return ps->SetStash(key, new Polynomial(POLY_SUM, B.ComputeTuttePol(), p));
	}


	// Row reduction
	for (COLROWTYPE ii=2; ii<=r; ii++) {
		if (this->GetElement(j-1,ii-1)) {
			this->RowReduce(ii-1,0,j-1);
		}
	}

	Matrix C;
	C.ComputePluckMatrix(this,0,j);	// Remove $j$'th column
	C.RowRoll(1);					// and rotate rows one down

	return ps->SetStash(key, new Polynomial(POLY_SUM, B.ComputeTuttePol(), C.ComputeTuttePol()));
}

DATATYPE Matrix::GetElement(COLROWTYPE c, COLROWTYPE r)
{
	// Lean-and-mean (ie no bounds-checks)
	return ((m_data[ComputeDataIndex(c,r)] & BYTEINDEX[c%8]) == BYTEINDEX[c%8]);
}

void Matrix::SetElement(COLROWTYPE c, COLROWTYPE r, DATATYPE val)
{
	// Lean-and-mean (ie no bounds-checks)
	if (val) {
		m_data[ComputeDataIndex(c,r)] |= BYTEINDEX[c%8];
	}
	else {
		m_data[ComputeDataIndex(c,r)] &= ~BYTEINDEX[c%8];
	}
}

unsigned int Matrix::ComputeDataIndex(COLROWTYPE c, COLROWTYPE r)
{
	return r*(m_columns/8+1)+(c/8);
}


COLROWTYPE Matrix::GetDimensionRows() 
{
	return m_rows;
}

COLROWTYPE Matrix::GetDimensionColumns()
{
	return m_columns;
}

bool Matrix::Print(FILE* fp)
{
	for (COLROWTYPE r=0; r<this->GetDimensionRows(); r++) {
		for (COLROWTYPE c=0; c<this->GetDimensionColumns(); c++) {
			fprintf(fp, "%d ", this->GetElement(c,r));
		}
		fprintf(fp, "\n");
	}
	return true;
}

bool Matrix::RowReduce(COLROWTYPE r1, COLROWTYPE r2, COLROWTYPE j) //* \label{Matrix:RowReduce}
{
	// Reduce row r1 by row r2
	// r1, r2: zero-based indices of rows to reduce
	// j: zero-based index of first column in reduction (default: 0)
	unsigned int datapos1 = this->ComputeDataIndex(j,r1);
	unsigned int datapos1a = this->ComputeDataIndex(this->GetDimensionColumns(),r1);
	unsigned int datapos2 = this->ComputeDataIndex(j,r2);
	
	// Special first byte
	unsigned char mask = BYTEINDEXACC[j%8];
	m_data[datapos1] = (m_data[datapos1] & mask) | ((m_data[datapos1]^m_data[datapos2]) & ~mask);
	
	// Regular bytes
	for (datapos1++;datapos1<=datapos1a; datapos1++) {
		datapos2++;
		m_data[datapos1] ^= m_data[datapos2];
	}
	return true;
}

bool Matrix::RowSwap(COLROWTYPE r1, COLROWTYPE r2)
{
	for (COLROWTYPE c=0; c<this->GetDimensionColumns(); c++) {
		DATATYPE t = this->GetElement(c,r1);
		this->SetElement(c,r1,this->GetElement(c,r2));
		this->SetElement(c,r2,t);
	}
	return true;
}

bool Matrix::RowRoll(COLROWTYPE n)
{
	if (n<1) { return false; }
	if (n>=m_rows) { return false; }

	DATATYPE* temp = (DATATYPE*)malloc(n*sizeof(DATATYPE));
	COLROWTYPE r,c;
	for (c=0; c<m_columns; c++) {
		for (r=0; r<m_rows; r++) {
			if (r<n) {
				temp[r] = GetElement(c,r);
			} else {
				SetElement(c,r-n,GetElement(c,r));
			}
		}
		for (r=0; r<n; r++) {
			if (m_rows-n+r<0) {
				fprintf (stderr, "Overflow (m_rows,n,r)=(%d,%d,%d)\n", m_rows, n, r);
			}
			SetElement(c,m_rows-n+r,temp[r]);
		}
	}
	free(temp);
	return true;
}

bool Matrix::ReadFromFile(char* filename)
{
	fprintf(stdout, "Reading matrix from file %s...\n", filename);
	FILE* mf = fopen(filename, "r");
	if ( mf == NULL ) {
		fprintf(stderr, "Error opening file %s\n", filename);
		return false;
	}

	int rows = 0;
	int columns = -1;

	char* line = (char*)malloc(1024*sizeof(char));
	// read specification line
	fgets(line,1024,mf);
	// parse specification line
	if (sscanf (line, "%d %d", &rows, &columns) != 2) {
		return false;
	}

	if (columns<0) {
		fprintf(stderr, "Malformed line (%d) in matrix description\n", -columns);
		free(line);
		return false;
	}

	fprintf(stdout, "Reading %d rows of %d columns from file %s\n", rows, columns, filename);

	if (m_data != NULL) { free(m_data); }

	m_rows = rows;
	m_columns = columns;
	this->AllocateData(m_columns, m_rows);

	int row = 0;
	while (fgets(line, 1024, mf)!=NULL) {
		int column=0;
		for (unsigned int c=0; c<strlen(line); c++) {
			unsigned int c1;
			for (c1=c+1; column<columns && c1<strlen(line) && line[c1]!=',' && line[c1]!=' '; c1++) {};
			// Column data is in line[c..c1[
			SetElement(column,row,atoi(line+c)==1);
			column++;
			c=c1;
		}
		row++;
	}

	free(line);
	fclose(mf);
	
	// NB: Powers of x are bounded by number of rows; powers of y by number of columns
	global_maxx = this->GetDimensionRows();
	global_maxy = this->GetDimensionColumns();
	
	return true;
}

bool Matrix::AllocateData(COLROWTYPE columns, COLROWTYPE rows)
{
	unsigned int d = ((columns/8)+1)*rows;
	if (m_data != NULL) {
		free(m_data);
		fprintf(stderr,"Error in data allocation\n");
		exit(1);
	}
	m_data = (unsigned char*)calloc(d,sizeof(unsigned char));
	return true;
}

void Matrix::ComputePluckMatrix(Matrix *m, COLROWTYPE r, COLROWTYPE c) //* \label{Matrix:ComputePluckMatrix}
{
// if r=0: no action on rows; r>0: pluck row r-1
// if c=0: no action on columns; c>0: pluck column c-1
	COLROWTYPE br = m->GetDimensionRows();
	COLROWTYPE bn = m->GetDimensionColumns();
	m_rows = br;
	if (r>0) {
		m_rows--;
	} 
	m_columns = bn;
	if (c>0) {
		m_columns--;
	} 
	this->AllocateData(m_columns, m_rows);

	if ( (c==0) && (r==1) ) { // No change on columns, remove first row (typical use)
		unsigned int datapos1a = m->ComputeDataIndex(0,1);
		unsigned int datapos1b = m->ComputeDataIndex(bn-1,br-1);
		unsigned int datapos2a = this->ComputeDataIndex(0,0);
		memcpy((void*)(this->m_data+datapos2a*sizeof(unsigned char)), 
			(void*)(m->m_data+datapos1a*sizeof(unsigned char)), 
			(datapos1b-datapos1a+1)*sizeof(unsigned char));
		return;
	}

	int bi = 0;
	for (COLROWTYPE i=0; i<br; i++) {
		if (i+1!=r) {
			int bj = 0;
			for (COLROWTYPE j=0; j<bn; j++) {
				if (j+1!=c) {
					this->SetElement(bj,bi, m->GetElement(j,i));
					bj++;
				}
			}
			bi++;
		}
	}
}

unsigned long Matrix::ComputeStashIndex(int type) //* \label{Matrix:ComputeStashIndex}
{
	if (type==1) {
		if ( m_columns+m_rows <= 7 ) {
			unsigned long res;
			res = (m_rows-2) + (m_columns-2)*4;

			unsigned long factor = 16;
			for (COLROWTYPE i=0; i<m_rows; i++) {
				res += (m_data[i] & BYTEINDEXACC[m_columns]) * factor;
				factor *= BYTEINDEX[m_columns];
			}
			return res;
		}
	}
	return 0;
}
