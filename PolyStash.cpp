//*\noindent\verb!// $Header: /home/cvs/cvsroot/MSVC/TuttePol/PolyStash.cpp,v 1.11 2006/12/20 23:14:21 hkragh Exp $!

#include "StdAfx.h"
#include "PolyStash.h"

#include <stdlib.h>


PolyStash::PolyStash()
{
	m_stash = (Polynomial**)malloc(STASHSIZE*sizeof(Polynomial*));
	// All stash is set to NULL (cannot be securely done with calloc)
	for (STASHINDEX i=0; i<STASHSIZE; i++) {
		m_stash[i] = NULL;
	}
	m_actives = 0;
}

PolyStash::~PolyStash()
{
	for (STASHINDEX i=0; i<STASHSIZE; i++) {
		if (m_stash[i] != NULL) {
			delete m_stash[i];
		}
	}
	free (m_stash);
}

bool PolyStash::CheckStash(STASHINDEX key)
{
	if ((key!=0) && (LookupPoly(key)!=NULL)) {
		return true;
	} else {
		return false;
	}
}

Polynomial* PolyStash::GetPoly(STASHINDEX key)
{
	if (key != 0) {
		return LookupPoly(key)->Clone();
	} else {
		fprintf(stderr, "Trying to lookup key 0.\n");
		return NULL;
	}
}

Polynomial* PolyStash::SetStash(STASHINDEX key, Polynomial *p)
{
	if ( p == NULL) {
		return NULL;
	}

	if (key == 0) {
		return p;
	}

	if ( m_stash[key] == NULL ) {
		// The polynomium does not exist in the stash: Add it (in simple form) and return.

		m_stash[key] = p->Simplify();

		m_actives++;
		if (m_actives % 256 == 0) {
			float f = 100*(float)m_actives / STASHSIZE;
			fprintf(stderr, "Stash at %2.1f%% (%ld)\n", f, m_actives);
		}

		return p;	
	}
	else {
		// The polynomium already exists in the stash. This should not happen!
		fprintf(stderr, "Trying to reset stash (%d)\n", key);
		return p;
	}
}

Polynomial* PolyStash::LookupPoly(STASHINDEX key)
{
	return m_stash[key];
}
