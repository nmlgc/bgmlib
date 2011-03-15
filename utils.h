// Music Room Interface
// --------------------
// utils.h - Random utility functions
// --------------------
// "©" Nmlgc, 2011

void* memcpy_advance(void* dest, char** src, size_t size);	// Performs memcpy and increases [src] by [size]
void* memcpy_advance(char** dest, const void* src, size_t size);	// Performs memcpy and increases [dest] by [size]

// ulong SplitString(const char* String, const char Delimiter, PList<char>* Result);
// int ReadLineFromFile(char* String, int MaxChars, FXFile& File);

// Write [Byte] [Count] times to [File]
bool WriteByteBlock(FXFile& File, const long& Count, const FXchar Byte = 0);

// Removes all occurrences of a given substring
FXString& remove_sub(FXString& str, const FXchar* org, FXint olen);	
FXString& remove_sub(FXString& str, const FXchar& org);
FXString& remove_sub(FXString& str, const FXString& org);

// Return the string value of "[ValName][Assign]" in [Source], terminated by [End]
FXString NamedValue(const FXString& Source, const FXString& ValName, const FXString& Assign, const FXString& End);

// Random endianess value classes
// ------------------------------

#define LE 0
#define BE 1
const bool SysEnd = LE;

// Base class
template <typename T> struct EndVal
{
protected:
	bool	e;	// Endianess flag
	T		d;	// Data

public:
	virtual T& swap() = 0;	// Swaps the endianess of <d>, implemented in subclass

	void set(const T& data, const bool& endian)
	{
		d = data;
		e = endian;
	}

	operator T& ()	{return d;}

	T& val()	{return d;}
	// Returns value in requested endianness
	T& val(const bool& endian)
	{
		if(e == endian)	return d;
		else			return swap();
	}

	T& operator ()	(const bool endian = SysEnd)	{return val(endian);}

	EndVal<T>()													{e = SysEnd; d = 0;}
	EndVal<T>(const EndVal<T>& s)								{set(s.d, s.e);}
	EndVal<T>(const ushort& data, const bool& endian = SysEnd)	{set(_d, _e);}
};
// ------------------------------

// 16-bit
ushort EndianSwap(const ushort& x);

struct u16 : public EndVal<ushort>
{
public:
	ushort& swap();
};


// 32-bit
ulong EndianSwap(const ulong& x);

struct u32 : public EndVal<ulong>
{
public:
	ulong& swap();
};
