//*\noindent\verb!// $Header: /home/cvs/cvsroot/MSVC/TuttePol/TuttePol.cpp,v 1.26 2006/12/20 23:14:21 hkragh Exp $!
// TuttePol.cpp : Defines the entry point for the console application.

#include "StdAfx.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "Matrix.h"
#include "Polynomial.h"

uint64 fourbyfour=0;
uint64 dimmaxseven=0;
uint64 dimmaxeight=0;
uint64 dimeight=0;

#include "PolyStash.h"
PolyStash* ps;

#include "VersionInfo.cpp"

char global_maxx;
char global_maxy;

int main(int argc, char* argv[])
{
	fprintf(stdout, "TuttePol (hkragh) version %s%s, built %s\n", MAJORVERSION, MINORVERSION, __DATE__);

	if (argc!=2) {
		fprintf(stderr, "Syntax: %s <matrix name>\n", argv[0]);
		return -1;
	}

	if (getenv("HOSTNAME") != NULL) {
		fprintf(stdout, "Running on computer %s\n", getenv("HOSTNAME"));
	} else if (getenv("COMPUTERNAME") != NULL) {
		fprintf(stdout, "Running on computer %s\n", getenv("COMPUTERNAME"));
	}

	fprintf(stdout, "Operating on matrix %s\n", argv[1]);

	char matrixfilename[255];
	char polynomialfilename[255];
	sprintf(matrixfilename, "%s.txt", argv[1]);
	sprintf(polynomialfilename, "%s.polynomial", argv[1]);

	fprintf(stdout, "Matrix input file %s\n", matrixfilename);
	fprintf(stdout, "Polynomial output file %s\n", polynomialfilename);
	
	Matrix* mr = new Matrix;
	if (mr->ReadFromFile(matrixfilename)) {
		mr->Print(stdout);

		ps = new PolyStash();

		time_t t0 = time(NULL);
		Matrix* m;
		if (0) {
			m = new Matrix();
			m->ComputePluckMatrix(mr,1,0);
			delete mr;
		} else {
			m=mr;
		}

		time_t t1=t0;
		fprintf(stdout, "Main computation started at %s", asctime(localtime(&t1)));
		Polynomial* p = m->ComputeTuttePol();
		time_t t2 = time(NULL);

		fprintf(stdout, "\n");
		fprintf(stdout, "Main computation ended   at %s", asctime(localtime(&t2)));
		fprintf(stdout, "Time to compute %d sec\n", t2-t1);

		if (p == NULL) { 
			fprintf(stderr, "Error computing Tutte polynomial\n");
			return -1;
		}
		Polynomial* q = p->Simplify();
		time_t t3 = time(NULL);
		fprintf(stdout, "Time to reduce  %d sec\n", t3-t2);
		q->Print(polynomialfilename);
		q->Print(stdout);
		fprintf(stdout,"\n");
		time_t t4 = time(NULL);
		fprintf(stdout, "Time to print   %d sec\n", t4-t3);

		delete p;
		delete q;
		time_t t5 = time(NULL);
		fprintf(stdout, "Time to kill    %d sec\n", t5-t4);

		fprintf(stdout, "Total run time  %d sec\n", t5-t0);

		float f = 100*(float)ps->m_actives / STASHSIZE;
		fprintf(stdout, "\nTotal stash usage at %2.1f%% (%ld)\n", f, ps->m_actives);
	}

	delete ps;
	return 0;
}