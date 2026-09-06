#pragma once
#include "Types.h"

// TMP

#pragma region TypeList
template<typename... T>
struct TypeList;

template<typename T, typename U>
struct TypeList<T, U>
{
	using Head = T;
	using Tail = U;
};

template<typename T, typename... U>
struct TypeList<T, U...>
{
	using Head = T;
	using Tail = TypeList<U...>;
};

#pragma endregion

#pragma region Length
template<typename T>
struct Length;

template<>
struct Length<TypeList<>>
{
	enum { value = 0 }; // 컴파일 타임에 결정
};

template<typename T, typename... U>
struct Length<TypeList<T, U...>>
{
	enum { value = 1 + Length<TypeList<U...>>::value };
};

#pragma endregion

#pragma region TypeAt
template<typename TL, int32 index>
struct TypeAt;

template<typename Head, typename... Tail>
struct TypeAt<TypeList<Head, Tail...>, 0>
{
	using Result = Head;
};

template<typename Head, typename... Tail, int32 index>
struct TypeAt<TypeList<Head, Tail...>, index>
{
	using Result = typename TypeAt<TypeList<Tail...>, index - 1>::Result;
};
#pragma endregion

#pragma region IndexOf

template<typename TL, typename T>
struct IndexOf;

template<typename... Tail, typename T>
struct IndexOf<TypeList<T, Tail...>, T>
{
	enum { value = 0 };
};

template<typename T>
struct IndexOf<TypeList<>, T>
{
	enum { value = -1 };
};

template<typename Head, typename... Tail, typename T>
struct IndexOf<TypeList<Head, Tail...>, T>
{
private:
	enum { temp = IndexOf<TypeList<Tail...>, T>::value };
public:
	enum { value = (temp == -1) ? -1 : temp + 1 };
};

#pragma endregion

#pragma region Conversion
template<typename From, typename To>
class Conversion // From에서 To까지의 Casting이 가능한가?
{
private:
	using Small = __int8;
	using Big = __int32;

	static Small Test(const To&) { return 0; }
	static Big Test(...) { return 0; }
	static From MakeFrom() { return 0; }

public:
	enum
	{
		exists = sizeof(Test(MakeFrom())) == sizeof(Small) // casting이 가능한가
	};
};

#pragma endregion

#pragma region TypeCast
template<int32 i>
struct IntToType
{
	enum { value = i };
};

template<typename TL>
class TypeConversion
{
public:
	enum
	{
		length = Length<TL>::value
	};

	TypeConversion()
	{
		MakeConvertTable(IntToType<0>(), IntToType<0>());
	}

	template<int32 i, int32 j>
	static void MakeConvertTable(IntToType<i>, IntToType<j>)
	{
		using FromType = typename TypeAt<TL, i>::Result;
		using ToType = typename TypeAt<TL, j>::Result;

		if (Conversion<const FromType*, const ToType*>::exists)
			s_convert[i][j] = true;
		else
			s_convert[i][j] = false;

		MakeConvertTable(IntToType<i>(), IntToType<j + 1>());
	}

	template<int32 i>
	static void MakeConvertTable(IntToType<i>, IntToType<length>)
	{
		MakeConvertTable(IntToType<i + 1>(), IntToType<0>());
	}

	template<int32 j>
	static void MakeConvertTable(IntToType<length>, IntToType<j>)
	{
	}

	static inline bool CanConvert(int32 from, int32 to)
	{
		static TypeConversion conversion;
		return s_convert[from][to];
	}

public:
	static bool s_convert[length][length];

};

template<typename TL>
bool TypeConversion<TL>::s_convert[length][length];


template<typename To, typename From>
To TypeCast(From* ptr)
{
	if (ptr == nullptr)
		return nullptr;

	using TL = typename From::TL;

	if (TypeConversion<TL>::CanConvert(ptr->m_typeid, IndexOf<TL, std::remove_pointer_t<To>>::value))
		return static_cast<To>(ptr);

	return nullptr;
}

template<typename To, typename From>
std::shared_ptr<To> TypeCast(std::shared_ptr<From> ref)
{
	if (ref == nullptr)
		return nullptr;

	using TL = typename From::TL;

	if (TypeConversion<TL>::CanConvert(ref->m_typeid, IndexOf<TL, std::remove_pointer_t<To>>::value))
		return std::static_pointer_cast<To>(ref);

	return nullptr;
}

template<typename To, typename From>
bool CanCast(From* ptr)
{
	if (ptr == nullptr)
		return false;

	using TL = typename From::TL;

	return TypeConversion<TL>::CanConvert(ptr->m_typeid, IndexOf<TL, std::remove_pointer_t<To>>::value);
}

template<typename To, typename From>
bool CanCast(std::shared_ptr<From> ref)
{
	if (ref == nullptr)
		return false;

	using TL = typename From::TL;

	return TypeConversion<TL>::CanConvert(ref->m_typeid, IndexOf<TL, std::remove_pointer_t<To>>::value);
}
#pragma endregion

#define DECL_TL		using TL = TL; int32 m_typeid;
#define INIT_TL(T) m_typeid = IndexOf<TL, T>::value;