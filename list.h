// Legacy Engine
// -------------
// list.h - Defines template list and stack classes
// -----------
// "©" David Scherfgen 2004, Terriermon 2005, Nmlgc 2007-2010

#ifndef LEGACY_LIST_H
#define LEGACY_LIST_H

#pragma once

// Entry Structures
// ----------------
template <typename Type> struct BaseListEntry
{
	BaseListEntry<Type>*	pPrevEntry;	// Pointer to the previous entry
	BaseListEntry<Type>*	pNextEntry;	// Pointer to the next entry

	BaseListEntry()		{pPrevEntry = pNextEntry = NULL;}
	virtual ulong MemSize() = 0;

	virtual	void	SetData(const Type* Data, ulong Reserved = 0) = 0;

	virtual ~BaseListEntry()	{}
};

template <typename Type> struct ListEntry : public BaseListEntry<Type>
{
	Type	Data;		// The actual list data

	ListEntry<Type>*	Prev()	{return (ListEntry<Type>*)BaseListEntry<Type>::pPrevEntry;}
	ListEntry<Type>*	Next()	{return (ListEntry<Type>*)BaseListEntry<Type>::pNextEntry;}
	ulong	MemSize()			{return sizeof(BaseListEntry<Type>*) * 2 + sizeof(Type);}

	void	SetData(const Type* Data, ulong Reserved = 0);
};

template <typename Type> struct PListEntry : public BaseListEntry<Type>
{
	Type* Data;	// The actual list data
	ulong Size;	// Data Array Size

	PListEntry<Type>*	Prev()	{return (PListEntry<Type>*)BaseListEntry<Type>::pPrevEntry;}
	PListEntry<Type>*	Next()	{return (PListEntry<Type>*)BaseListEntry<Type>::pNextEntry;}
	ulong	MemSize()			{return sizeof(BaseListEntry<Type>*) + sizeof(Type*) + sizeof(ulong) + Size;}

	void	SetData(const Type* Data, ulong Size);

	PListEntry()	{Data = NULL; Size = 0;}
	~PListEntry()	{SAFE_DELETE_ARRAY(Data);}
};
// ----------------

// List Base Class
// ---------------
template <typename Type> class BaseList
{
protected:
	BaseListEntry<Type>*	pFirst;
	BaseListEntry<Type>*	pLast;
	ulong					NumEntries;

			BaseListEntry<Type>*	GetBaseEntry(ulong Index);
			BaseListEntry<Type>*	AppendEntry();
			BaseListEntry<Type>*	InsertEntry(BaseListEntry<Type>* pPrev);
	virtual BaseListEntry<Type>*	NewEntry() = 0;
	
public:
	BaseList();
	virtual ~BaseList()	{};

	BaseListEntry<Type>*	BaseDelete(BaseListEntry<Type>* pEntry);

	ulong	Size()	{return NumEntries;}
	virtual ulong MemSize() = 0;	// Returns size of the list in memory
};
// ---------------

// Normal List
// -----------
template <typename Type> class List : public BaseList<Type>
{
protected:
	BaseListEntry<Type>*	NewEntry();

public:
	List()	{}
	List(List<Type>& Source)	{Copy(&Source);}
	List(List<Type>* Source)	{Copy(Source);}
	~List();
	
	ListEntry<Type>*	Add(const Type* pData = NULL);					// Append entry
	ListEntry<Type>*	Insert(ListEntry<Type>* pPrev, const Type* pData = NULL);	// Insert an entry after the given one
	ListEntry<Type>*	Find(Type* pData);								// Search for entry with given data
	ListEntry<Type>*	Get(ulong Index);
	ListEntry<Type>*	Delete(ListEntry<Type>* pEntry);
	ListEntry<Type>*	PopFirst();	// Delete first entry
	ListEntry<Type>*	PopLast();	// Delete last entry
	void				Clear();
	void				Resize(ulong Size);
	bool				Copy(List<Type>* Source, bool Append = false);
	ulong				MemSize();										// Returns size of the list in memory
	
	// Inline Methods
	ListEntry<Type>*   First()					{return (ListEntry<Type>*)BaseList<Type>::pFirst;}
	ListEntry<Type>*   Last()					{return (ListEntry<Type>*)BaseList<Type>::pLast;}

	ListEntry<Type>* operator []	(ulong EntryID)	{return Get(EntryID);}
};

#define ADD_THIS(List, x)	x* KeywordCloak = this; List.Add(&KeywordCloak);	// Adds a class pointer
// -----------

// Pointer List - saves multiple elements of one type per entry
// ------------
template <typename Type> class PList : public BaseList<Type>
{
protected:
	BaseListEntry<Type>*	NewEntry();

public:
	PList()	{}
	PList(PList<Type>& Source)	{Copy(&Source);}
	PList(PList<Type>* Source)	{Copy(Source);}
	~PList();

	PListEntry<Type>*   Add(const Type* pData, ulong Size);		// Append entry
	PListEntry<Type>*   Insert(PListEntry<Type>* pPrev, const Type* pData, ulong Size);	// Insert an entry after the given one
	PListEntry<Type>*   Find(Type* pData);								// Search for entry with given data
	PListEntry<Type>*   Get(ulong Index);
	PListEntry<Type>*   Delete(PListEntry<Type>* pEntry);
	PListEntry<Type>*	Pop();	// Deletes the first entry
	void				Clear();
	bool               Copy(PList<Type>* Source, bool Append = false);		
	ulong              MemSize();										// Returns size of the list in memory
	
	// Inline methods
	PListEntry<Type>*   First()	{return (PListEntry<Type>*)BaseList<Type>::pFirst;}
	PListEntry<Type>*   Last()	{return (PListEntry<Type>*)BaseList<Type>::pLast;}

	PListEntry<Type>* operator []	(ulong EntryID)	{return Get(EntryID);}
};

template class PList<char>;	// It's the only use of this class anyway
// ------------

// Stack List - Fixed element size
// ----------
template <typename Type> class Stack : public List<Type>
{
private:
	ulong StackSize;

public:
	Stack(ulong lSize);

	ListEntry<Type>*	Add(Type* pData);

	bool	IsFull()			{return StackSize == List<Type>::NumEntries;}
	ulong	GetMaxElements()	{return StackSize;}
};
// ----------

#include "list.cpp.h"

#endif /* LEGACY_LIST_H */
