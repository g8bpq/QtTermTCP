/*
Copyright (C) 2019-2020 Andrei Kopanchuk UZ7HO

This file is part of QtSoundModem

QtSoundModem is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QtSoundModem is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QtSoundModem.  If not, see http://www.gnu.org/licenses

*/

// UZ7HO Soundmodem Port by John Wiseman G8BPQ
#include "ax25.h"

// TStringlist And String emulation Functions

// Dephi seems to mix starting counts at 0 or 1. I'll try making everything
// base zero. 

// Initialise a list

void CreateStringList(TStringList * List)
{
	List->Count = 0;
	List->Items = 0;
}


int Count(TStringList * List)
{
	return List->Count;
}

string * newString()
{
	// Creates and Initialises a string 

	UCHAR * ptr = malloc(sizeof(string));			// Malloc Data separately so it can be ralloc'ed
	string * New = (string *)ptr;
	New->Length = 0;
	New->AllocatedLength = 256;
	New->Data = malloc(256);

	return New;
}

void initString(string * S)
{
	S->Length = 0;
	S->AllocatedLength = 256;
	S->Data = malloc(256);
}

void initTStringList(TStringList* T)
{
	//string * New = newString();

	T->Count = 0;
	T->Items = NULL;

	//Add(T, New);
}



TStringList * newTStringList()
{
	TStringList * T = (TStringList *) malloc(sizeof(TStringList));
	string * New = newString();

	T->Count = 0;
	T->Items = NULL;

	Add(T, New);

	return T;
}


void freeString(string * Msg)
{
	if (Msg->Data)
		free(Msg->Data);

	free(Msg);
}

string * Strings(TStringList * Q, int Index)
{
	if (Index >= Q->Count)
		return NULL;

	return Q->Items[Index];
}

int Add(TStringList * Q, string * Entry)
{
	Q->Items = realloc(Q->Items,(Q->Count + 1) * sizeof(void *));
	Q->Items[Q->Count++] = Entry;

	return (Q->Count);
}


void mydelete(string * Source, int StartChar, int Count)
{
	 //Description
	//The Delete procedure deletes up to Count characters from the passed parameter Source string starting
	//from position StartChar.

	if (StartChar > Source->Length)
		return;

	int left = Source->Length - StartChar;

	if (Count > left)
		Count = left;

	memmove(&Source->Data[StartChar], &Source->Data[StartChar + Count], left - Count);

	Source->Length -= Count;
}
 

void Delete(TStringList * Q, int Index)
{
	// Remove item at Index and move rest up list
	// Index starts at zero
	
	if (Index >= Q->Count)
		return;

	// We should free it, so user must duplicate msg if needed after delete

	freeString(Q->Items[Index]);

	Q->Count--;

	while (Index < Q->Count)
	{
		Q->Items[Index] = Q->Items[Index + 1];
		Index++;
	}
}

void setlength(string * Msg, int Count)
{
	// Set length, allocating more space if needed

	if (Count > Msg->AllocatedLength)
	{
		Msg->AllocatedLength = Count + 256;
		Msg->Data = realloc(Msg->Data, Msg->AllocatedLength);
	}

	Msg->Length = Count;
}

string * stringAdd(string * Msg, UCHAR * Chars, int Count)
{
	// Add Chars to string 

	if (Msg->Length + Count > Msg->AllocatedLength)
	{
		Msg->AllocatedLength += Count + 256;
		Msg->Data = realloc(Msg->Data, Msg->AllocatedLength);
	}

	memcpy(&Msg->Data[Msg->Length], Chars, Count);
	Msg->Length += Count;
	
	return Msg;
}

void Clear(TStringList * Q)
{
	int i = 0;

	if (Q->Items == NULL)
		return;

	while (Q->Count)
	{
		freeString(Q->Items[i++]);
		Q->Count--;
	}

	free(Q->Items);

	Q->Items = NULL;
}

// procedure move ( const SourcePointer; var DestinationPointer; CopyCount : Integer ) ;
// Description
// The move procedure is a badly named method of copying a section of memory from one place to another.
 
// CopyCount bytes are copied from storage referenced by SourcePointer and written to DestinationPointer
 
void move(UCHAR * SourcePointer, UCHAR * DestinationPointer, int CopyCount)
{
	memmove(DestinationPointer, SourcePointer, CopyCount);
}

void fmove(float * SourcePointer, float * DestinationPointer, int CopyCount)
{
	memmove(DestinationPointer, SourcePointer, CopyCount);
}



//Description
//The copy function has 2 forms. In the first, it creates a new string from part of an existing string. In the second, it creates a new array from part of an existing array.
 
//1.String copy
 
//The first character of a string has index = 1.

//Up to Count characters are copied from the StartChar of the Source string to the returned string.
//Less than Count characters if the end of the Source string is encountered before Count characters have been copied.


string * copy(string * Source, int StartChar, int Count)
{
	string * NewString = newString();
	int end = StartChar + Count;

	if (end > Source->Length)
		Count = Source->Length - StartChar;

	memcpy(NewString->Data, &Source->Data[StartChar], Count);

	NewString->Length = Count;

	return NewString;
}

// Duplicate from > to

void Assign(TStringList * to, TStringList * from)
{
	int i;

	Clear(to);

	if (from->Count == 0)
		return;

	// Duplicate each item

	for (i = 0; i < from->Count; i++)
	{
		string * new = newString();

		stringAdd(new, from->Items[i]->Data, from->Items[i]->Length);
		Add(to, new);
	}
}

string * duplicateString(string * in)
{
	string * new = newString();

	stringAdd(new, in->Data, in->Length);

	return new;
}


double pila(double x)
{
	//x : = frac(x); The frac function returns the fractional part of a floating point number.

	double whole;
	double rem;

	rem = modf(x, &whole);  // returns fraction, writes whole to whole

	if (rem != rem)
		rem = 0;

	if (rem > 0.5)
		rem = 1 - rem;

	return 2 * rem;
}

boolean compareStrings(string * a, string * b)
{
	if (a->Length == b->Length && memcmp(a->Data, b->Data, a->Length) == 0)
		return TRUE;

	return FALSE;
}

// This looks for a string in a stringlist. Returns index if found, otherwise -1

int  my_indexof(TStringList * l, string * s)
{
	int i;

	for (i = 0; i < l->Count; i++)
	{
		// Need to compare count and data - C doesn't allow struct compare

		if (l->Items[i]->Length == s->Length && memcmp(l->Items[i]->Data, s->Data, s->Length) == 0)
			return i;
	}
	return -1;
}

 