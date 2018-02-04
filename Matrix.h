#pragma once

#include "Polynomial.h"

#define COLROWTYPE unsigned int
#define DATATYPE bool


class Matrix
{
public:
	Matrix(void);
public:
	~Matrix(void);

public:
	unsigned long ComputeStashIndex(int type=0);
	void ComputePluckMatrix(Matrix* m, COLROWTYPE r, COLROWTYPE c);
	DATATYPE GetElement(COLROWTYPE c, COLROWTYPE r);
	void SetElement(COLROWTYPE c, COLROWTYPE r, DATATYPE val);
	COLROWTYPE GetDimensionRows();
	COLROWTYPE GetDimensionColumns();

	Polynomial* ComputeTuttePol();

	bool ReadFromFile(char* filename);
	bool Print(FILE* fp);

	bool RowReduce(COLROWTYPE r1, COLROWTYPE r2, COLROWTYPE j=0);
	bool RowSwap(COLROWTYPE r1, COLROWTYPE r2);
	bool RowRoll(COLROWTYPE n);

private:
	COLROWTYPE m_rows;
	COLROWTYPE m_columns;

	unsigned char* m_data;
	unsigned int ComputeDataIndex(COLROWTYPE c, COLROWTYPE r);
	bool AllocateData(COLROWTYPE columns, COLROWTYPE rows);
};
