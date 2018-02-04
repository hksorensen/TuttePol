// PolyStash.h: interface for the PolyStash class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POLYSTASH_H__47E44257_5E33_4F6C_8A1F_DDA3F2B07D12__INCLUDED_)
#define AFX_POLYSTASH_H__47E44257_5E33_4F6C_8A1F_DDA3F2B07D12__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Matrix.h"
#include "Polynomial.h"

#define STASHINDEX unsigned int
#define STASHSIZE 65536

class PolyStash  
{
public:
	Polynomial* LookupPoly(STASHINDEX key);
	Polynomial* SetStash(STASHINDEX key, Polynomial* p);
	Polynomial* GetPoly(STASHINDEX key);
	bool CheckStash(STASHINDEX key);
	PolyStash();
	virtual ~PolyStash();

	STASHINDEX m_actives;

private:
	Polynomial** m_stash;
};

#endif // !defined(AFX_POLYSTASH_H__47E44257_5E33_4F6C_8A1F_DDA3F2B07D12__INCLUDED_)
