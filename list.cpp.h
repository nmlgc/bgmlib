// Legacy Engine
// -------------
// list.cpp.h - Defines template list and stack classes (Template Implementation)
// -----------
// "©" David Scherfgen 2004, Terriermon 2005, Nmlgc 2007-2010

#ifndef LEGACY_LIST_CPP_H
#define LEGACY_LIST_CPP_H

// ListEntry Classes
// -----------------
template <typename Type> void ListEntry<Type>::SetData(const Type* pData, ulong Reserved)
{
	if(pData)	memcpy(&Data, pData, sizeof(Type));
}

template <typename Type> void PListEntry<Type>::SetData(const Type* pData, ulong NewSize)
{
	if(NewSize != Size)
	{
		SAFE_DELETE_ARRAY(Data);
		Size = NewSize;
		Data = new Type[Size];
	}
	memcpy(Data, pData, sizeof(Type) * Size);
}
// -----------------

// List Base Class
// ---------------
template <typename Type> BaseList<Type>::BaseList()
{
	pFirst = NULL;
	pLast = NULL;
	NumEntries = 0;
}

template <typename Type> BaseListEntry<Type>* BaseList<Type>::GetBaseEntry(ulong Index)
{
	BaseListEntry<Type>* pCurrentEntry = pFirst;
	if(Index > NumEntries)	return NULL;	// Out of Range

	for(ulong Count = 0; Count < Index; Count++)
	{
		pCurrentEntry = pCurrentEntry->pNextEntry;
		if(!pCurrentEntry)	return NULL;
	}
	return pCurrentEntry;
}

template <typename Type> BaseListEntry<Type>* BaseList<Type>::AppendEntry()
{
	BaseListEntry<Type>* pNewEntry = (BaseListEntry<Type>*)NewEntry();
	if(!pNewEntry)	return NULL;

	// The new entry is the last one of the list.
	// Thus there's no next entry and the previous one of this one is the originally last one.
	pNewEntry->pPrevEntry = pLast;
	if(pLast) pLast->pNextEntry = pNewEntry;
	pNewEntry->pNextEntry = NULL;
	pLast = pNewEntry;

	if(!pFirst) pFirst = pNewEntry;	// If the list is empty, this entry is the first one

	NumEntries++;
	return pNewEntry;
}

template <typename Type> BaseListEntry<Type>* BaseList<Type>::InsertEntry(BaseListEntry<Type>* pPrev)
{
	BaseListEntry<Type>* pNewEntry = (BaseListEntry<Type>*)NewEntry();
	if(!pNewEntry)	return NULL;

	if(!pPrev)
	{
		// The new entry is added to the beginning of the list.
		if(pFirst)
		{
			pNewEntry->pPrevEntry = NULL;
			pFirst->pPrevEntry = pNewEntry;
		}
		pNewEntry->pNextEntry = pFirst;
		pFirst = pNewEntry;
	}
	else
	{
		// If there is another entry behind the inserted one, the 
		// previous entry of this one has to be the inserted one.
		if(pPrev->pNextEntry)	pPrev->pNextEntry->pPrevEntry = pNewEntry;
		else					pLast = pNewEntry;

		pNewEntry->pNextEntry = pPrev->pNextEntry;
		pPrev->pNextEntry = pNewEntry;

		pNewEntry->pPrevEntry = pPrev;
	}

	NumEntries++;
	return pNewEntry;
}

// Delete list entry, returns next one
template <typename Type> BaseListEntry<Type>* BaseList<Type>::BaseDelete(BaseListEntry<Type>* pEntry)
{
	if(!pEntry) return NULL;
	
	BaseListEntry<Type>* r = pEntry->pNextEntry;

	// By deleting an entry, we cause a "hole" in the list, and have to relink the previous and next entries
	// according to the position of the entry to be deleted.
	if(pEntry == pFirst && pEntry == pLast)
	{
		// The entry is the first and only one
		pFirst = pLast = NULL;
		r = NULL;
	}
	else if(pEntry == pFirst)
	{
		// This entry is the first one.
		// The new first entry is the next entry of this one.
		pFirst = pEntry->pNextEntry;
		pFirst->pPrevEntry = NULL;
	}
	else if(pEntry == pLast)
	{
		// This entry is the last one.
		// The new last entry is the previous entry of this one.
		pLast = pEntry->pPrevEntry;
		pLast->pNextEntry = NULL;
	}
	else
	{
		// The entry is somewhere inbetween.
		// The previous and next entry of this one get linked accordingly.
		pEntry->pPrevEntry->pNextEntry = pEntry->pNextEntry;
		pEntry->pNextEntry->pPrevEntry = pEntry->pPrevEntry;
	}

	NumEntries--;
	return r;
}
// ---------------

// Normal List
// -----------
template <typename Type> List<Type>::~List()	{Clear();}

template <typename Type> BaseListEntry<Type>* List<Type>::NewEntry()
{
 	return new ListEntry<Type>;
}

// Append entry
template <typename Type> ListEntry<Type>* List<Type>::Add(const Type* pData)
{
	ListEntry<Type>* pNewEntry = (ListEntry<Type>*)BaseList<Type>::AppendEntry();
	if(!pNewEntry)	return NULL;

	pNewEntry->SetData(pData);

	return pNewEntry;
}

// Insert an entry after the given one
template <typename Type> ListEntry<Type>* List<Type>::Insert(ListEntry<Type>* pPrev, const Type* pData)
{
	ListEntry<Type>* pNewEntry =(ListEntry<Type>*)InsertEntry(pPrev);
	if(!pNewEntry)	return NULL;

	pNewEntry->SetData(pData);

	return pNewEntry;
}

// Search for an entry with the given data
template <typename Type> ListEntry<Type>* List<Type>::Find(Type* pData)
{
	ListEntry<Type>* CurEntry = First();

	if(!pData) return 0;

	while(CurEntry)
	{
		if(!memcmp(&CurEntry->Data, pData, sizeof(Type)))
		{
			return CurEntry;
		}
		CurEntry = CurEntry->Next();
	}

	// No entry was found!
	return 0;
}

template <typename Type> ListEntry<Type>* List<Type>::Get(ulong Index)
{
	return (ListEntry<Type>*)BaseList<Type>::GetBaseEntry(Index);
}

template <typename Type> ListEntry<Type>* List<Type>::Delete(ListEntry<Type>* pEntry)
{
	ListEntry<Type>* r = (ListEntry<Type>*)BaseList<Type>::BaseDelete(pEntry);
	
	SAFE_DELETE(pEntry);
	return r;
}

// Resize list
template <typename Type> void List<Type>::Resize(ulong Size)
{
	ulong Count;
	ulong Entries = BaseList<Type>::NumEntries;	// Yeah, the entry count does indeed change... ;-)

	if(Size > Entries)
	{
		for(Count = Entries; Count < Size; Count++)	Add(NULL);
	}
	else if(Size < Entries)
	{
		for(Count = Size; Count < Entries; Count++) Delete(Last());
	}
	return;
}

// Copy list
template <typename Type> bool List<Type>::Copy(List<Type>* Source, bool Append)
{
	if(!Source)	return false;
	if(!Append)	Clear();

	ListEntry<Type>* SrcEntry = Source->First();

	while(SrcEntry)
	{
		Add(&SrcEntry->Data);
		SrcEntry = SrcEntry->Next();
	}
	return true;
}

// Returns size of the list in memory
template <typename Type> ulong List<Type>::MemSize()
{
	return 2 * sizeof(ListEntry<Type>*) + sizeof(ulong) + First()->MemSize() * BaseList<Type>::NumEntries;
}

// Delete first entry
template <typename Type> ListEntry<Type>* List<Type>::PopFirst()
{
	return Delete(First());
}

// Delete first entry
template <typename Type> ListEntry<Type>* List<Type>::PopLast()
{
	return Delete(Last());
}

// Delete entire list
template <typename Type> void List<Type>::Clear()
{
	// Delete the first entry until none is there
	while(First())	PopFirst();
}
// -----------

// Pointer List
// ------------
template <typename Type> PList<Type>::~PList()	{Clear();}

template <typename Type> BaseListEntry<Type>* PList<Type>::NewEntry()
{
	return new PListEntry<Type>;
}

// Add a new list entry
template <typename Type> PListEntry<Type>* PList<Type>::Add(const Type* pData, ulong Size)
{
	PListEntry<Type>* pNewEntry = (PListEntry<Type>*)BaseList<Type>::AppendEntry();
	if(!pNewEntry)	return NULL;

	pNewEntry->SetData(pData, Size);

	return pNewEntry;
}

// Insert a new list entry at a given position
template <typename Type> PListEntry<Type>* PList<Type>::Insert(PListEntry<Type>* pPrev, const Type* pData, ulong Size)
{
	PListEntry<Type>* pNewEntry = (PListEntry<Type>*)InsertEntry(pPrev);
	if(!pNewEntry)	return NULL;

	pNewEntry->SetData(pData, Size);

	return pNewEntry;
}

template <typename Type> PListEntry<Type>* PList<Type>::Find(Type* Data)
{
	if(!Data) return NULL;

	PListEntry<Type>* CurEntry = First();

	while(CurEntry)
	{
		if(!memcmp(&CurEntry->Data, Data, sizeof(Type) * CurEntry->Size))	return CurEntry;
		CurEntry = CurEntry->Next();
	}

	return NULL;
}

template <typename Type> PListEntry<Type>* PList<Type>::Get(ulong Index)
{
	return (PListEntry<Type>*)BaseList<Type>::GetBaseEntry(Index);
}

template <typename Type> PListEntry<Type>* PList<Type>::Delete(PListEntry<Type>* pEntry)
{
	PListEntry<Type>* r = (PListEntry<Type>*)BaseList<Type>::BaseDelete(pEntry);

	SAFE_DELETE(pEntry);
	return r;
}

// Delete first entry
template <typename Type> PListEntry<Type>* PList<Type>::Pop()
{
	return Delete(First());
}

// Copy list
template <typename Type> bool PList<Type>::Copy(PList<Type>* Source, bool Append)
{
	if(!Source)	return false;
	if(!Append)	Clear();

	PListEntry<Type>* SrcEntry = Source->First();

	while(SrcEntry)
	{
		Add(SrcEntry->Data, SrcEntry->Size);
		SrcEntry = SrcEntry->Next();
	}
	return true;
}

// Delete entire list
template <typename Type> void PList<Type>::Clear()
{
	// Delete the first entry until none is there
	while(First())	Pop();
}

// Returns size of the list in memory
template <typename Type> ulong PList<Type>::MemSize()
{
	ulong MS = 2 * sizeof(PListEntry<Type>*) + sizeof(ulong);
	PListEntry<Type>* CurEntry = First();
	while(CurEntry)
	{
		MS += CurEntry->MemSize();
		CurEntry = CurEntry->Next();
	}

	return MS;
}
// ------------

// Stack List
// ----------
template <typename Type> Stack<Type>::Stack(ulong Size)	{StackSize = Size;}

template <typename Type> ListEntry<Type>* Stack<Type>::Add(Type* pData)
{
	ListEntry<Type>* pNewEntry = (ListEntry<Type>*)List<Type>::AppendEntry();
	if(!pNewEntry)	return NULL;

	if(List<Type>::NumEntries == StackSize)	List<Type>::Delete(List<Type>::pFirst);

	pNewEntry->SetEntryData(pData);

	return pNewEntry;
}
// ----------

#endif /* LEGACY_LIST_CPP_H */
