#pragma once

#include "CoreTypes.h"
#include "Types.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <span>
#include <algorithm>
#include <functional>
#include <utility>
#include <cassert>
#include <type_traits>

#define INDEX_NONE (-1)

// TInlineAllocator stub - just an alias for std::allocator
// UE uses this as the second template arg to TArray
template<int32 N>
using TInlineAllocator = std::allocator<int>; // Never actually used, just a placeholder

// ========================================================================
// TArray - backed by std::vector with UE-style API
// ========================================================================
template<typename T, typename AllocatorOrPolicy = std::allocator<T>>
class TArray : public std::vector<T, std::allocator<T>>
{
	using Base = std::vector<T, std::allocator<T>>;
public:
	TArray() = default;
	TArray(std::initializer_list<T> Init) : Base(Init) {}
	explicit TArray(int32 InNum) : Base(InNum) {}
	TArray(int32 InNum, const T& Val) : Base(InNum, Val) {}
	TArray(const T* Ptr, int32 Count) : Base(Ptr, Ptr + Count) {}

	// UE-style API
	int32 Num() const { return static_cast<int32>(Base::size()); }
	void SetNum(int32 NewNum) { Base::resize(NewNum); }
	void SetNumUninitialized(int32 NewNum) { Base::resize(NewNum); }
	void SetNumZeroed(int32 NewNum) { Base::resize(NewNum, T{}); }
	void Reserve(int32 Number) { Base::reserve(Number); }
	int32 Add(const T& Item) { Base::push_back(Item); return Num() - 1; }
	int32 Add(T&& Item) { Base::push_back(std::move(Item)); return Num() - 1; }
	template<typename... ArgsType>
	int32 Emplace(ArgsType&&... Args) { Base::emplace_back(std::forward<ArgsType>(Args)...); return Num() - 1; }
	void Append(const TArray& Source) { Base::insert(Base::end(), Source.begin(), Source.end()); }
	void Append(const T* Ptr, int32 Count) { Base::insert(Base::end(), Ptr, Ptr + Count); }
	int32 AddUnique(const T& Item)
	{
		int32 Idx = Find(Item);
		if (Idx == INDEX_NONE) return Add(Item);
		return Idx;
	}
	int32 AddZeroed(int32 Count = 1) { int32 StartIdx = Num(); Base::resize(Num() + Count); return StartIdx; }
	void Reset(int32 NewSize = 0) { Base::clear(); if (NewSize > 0) Base::reserve(NewSize); }
	void Empty(int32 NewSize = 0) { Base::clear(); Base::shrink_to_fit(); if (NewSize > 0) Base::reserve(NewSize); }
	bool IsEmpty() const { return Base::empty(); }
	T* GetData() { return Base::data(); }
	const T* GetData() const { return Base::data(); }
	void RemoveAt(int32 Index) { Base::erase(Base::begin() + Index); }
	void RemoveAt(int32 Index, int32 Count) { Base::erase(Base::begin() + Index, Base::begin() + Index + Count); }
	void RemoveAt(int32 Index, int32 Count, EAllowShrinking) { RemoveAt(Index, Count); }
	void RemoveAtSwap(int32 Index)
	{
		if (Index != Num() - 1) std::swap((*this)[Index], Base::back());
		Base::pop_back();
	}
	void RemoveAtSwap(int32 Index, int32 Count)
	{
		int32 EndSwap = Num() - 1;
		for (int32 i = 0; i < Count && EndSwap >= Index + i; i++, EndSwap--)
			std::swap((*this)[Index + i], (*this)[EndSwap]);
		Base::resize(Num() - Count);
	}
	void RemoveAtSwap(int32 Index, EAllowShrinking) { RemoveAtSwap(Index); }
	void RemoveAtSwap(int32 Index, int32 Count, EAllowShrinking) { RemoveAtSwap(Index, Count); }
	int32 Remove(const T& Item)
	{
		auto It = std::remove(Base::begin(), Base::end(), Item);
		int32 Removed = static_cast<int32>(Base::end() - It);
		Base::erase(It, Base::end());
		return Removed;
	}
	void Swap(int32 A, int32 B) { std::swap((*this)[A], (*this)[B]); }
	void Sort() { std::sort(Base::begin(), Base::end()); }
	template<typename Predicate>
	void Sort(Predicate Pred) { std::sort(Base::begin(), Base::end(), Pred); }
	void HeapSort() { std::sort(Base::begin(), Base::end()); }
	template<typename Predicate>
	void HeapSort(Predicate Pred) { std::sort(Base::begin(), Base::end(), Pred); }

	int32 Find(const T& Item) const
	{
		auto It = std::find(Base::begin(), Base::end(), Item);
		return (It != Base::end()) ? static_cast<int32>(It - Base::begin()) : INDEX_NONE;
	}
	bool Find(const T& Item, int32& Index) const
	{
		Index = Find(Item);
		return Index != INDEX_NONE;
	}
	bool Contains(const T& Item) const { return std::find(Base::begin(), Base::end(), Item) != Base::end(); }

	int32 FindLast(const T& Item) const
	{
		auto It = std::find(Base::rbegin(), Base::rend(), Item);
		return (It != Base::rend()) ? static_cast<int32>(Base::rend() - 1 - It) : INDEX_NONE;
	}

	const T& Last(int32 IndexFromEnd = 0) const { return Base::back(); }
	T& Last(int32 IndexFromEnd = 0) { return Base::back(); }
	T Pop() { T Val = std::move(Base::back()); Base::pop_back(); return Val; }

	T& Top() { return Base::back(); }
	const T& Top() const { return Base::back(); }
	void Push(const T& Item) { Base::push_back(Item); }
	void Push(T&& Item) { Base::push_back(std::move(Item)); }

	// Allow implicit conversion to span-like views
	operator std::span<const T>() const { return std::span<const T>(Base::data(), Base::size()); }
	operator std::span<T>() { return std::span<T>(Base::data(), Base::size()); }
};

// ========================================================================
// TArrayView - backed by std::span
// ========================================================================
template<typename T>
class TArrayView
{
	T* Data_;
	int32 Num_;
public:
	TArrayView() : Data_(nullptr), Num_(0) {}
	TArrayView(T* InData, int32 InNum) : Data_(InData), Num_(InNum) {}
	TArrayView(T* InData, size_t InNum) : Data_(InData), Num_(static_cast<int32>(InNum)) {}

	// Constructor from std::vector of same type
	TArrayView(std::vector<T>& V) : Data_(V.data()), Num_(static_cast<int32>(V.size())) {}

	// Constructor from TArray of same type
	TArrayView(TArray<T>& V) : Data_(V.data()), Num_(V.Num()) {}

	T* data() const { return Data_; }
	int32 Num() const { return Num_; }
	bool IsEmpty() const { return Num_ == 0; }
	T& operator[](int32 Idx) { return Data_[Idx]; }
	const T& operator[](int32 Idx) const { return Data_[Idx]; }
	T* begin() const { return Data_; }
	T* end() const { return Data_ + Num_; }
};

// ========================================================================
// TConstArrayView - read-only array view (UE typedef)
// ========================================================================
template<typename T>
using TConstArrayView = TArrayView<const T>;

// ========================================================================
// TMap - backed by std::unordered_map
// ========================================================================
template<typename KeyType, typename ValueType, typename Hasher = std::hash<KeyType>>
class TMap : public std::unordered_map<KeyType, ValueType, Hasher>
{
	using Base = std::unordered_map<KeyType, ValueType, Hasher>;
public:
	TMap() = default;

	// UE-style Find returns pointer (nullptr if not found)
	ValueType* Find(const KeyType& Key)
	{
		auto It = Base::find(Key);
		return (It != Base::end()) ? &It->second : nullptr;
	}
	const ValueType* Find(const KeyType& Key) const
	{
		auto It = Base::find(Key);
		return (It != Base::end()) ? &It->second : nullptr;
	}

	ValueType& FindOrAdd(const KeyType& Key) { return Base::operator[](Key); }

	int32 Num() const { return static_cast<int32>(Base::size()); }
	bool IsEmpty() const { return Base::empty(); }
	void Reset() { Base::clear(); }
	void Empty() { Base::clear(); }
	int32 Remove(const KeyType& Key) { return static_cast<int32>(Base::erase(Key)); }
	bool Contains(const KeyType& Key) const { return Base::find(Key) != Base::end(); }

	void GenerateKeyArray(TArray<KeyType>& OutKeys) const
	{
		OutKeys.Reset();
		OutKeys.Reserve(Num());
		for (const auto& Pair : *this) OutKeys.Add(Pair.first);
	}
	void GenerateValueArray(TArray<ValueType>& OutValues) const
	{
		OutValues.Reset();
		OutValues.Reserve(Num());
		for (const auto& Pair : *this) OutValues.Add(Pair.second);
	}
};

// ========================================================================
// TMultiMap - backed by std::unordered_multimap
// ========================================================================
template<typename KeyType, typename ValueType, typename Hasher = std::hash<KeyType>>
class TMultiMap : public std::unordered_multimap<KeyType, ValueType, Hasher>
{
	using Base = std::unordered_multimap<KeyType, ValueType, Hasher>;
public:
	TMultiMap() = default;

	int32 Num() const { return static_cast<int32>(Base::size()); }
	void Reset() { Base::clear(); }
	void Empty() { Base::clear(); }
	int32 Remove(const KeyType& Key) { return static_cast<int32>(Base::erase(Key)); }

	void Add(const KeyType& Key, const ValueType& Value) { Base::emplace(Key, Value); }

	TArray<ValueType> FindKey(const KeyType& Key) const
	{
		TArray<ValueType> Result;
		auto Range = Base::equal_range(Key);
		for (auto It = Range.first; It != Range.second; ++It) Result.Add(It->second);
		return Result;
	}
};

// ========================================================================
// TSet - backed by std::unordered_set
// ========================================================================
template<typename ElementType, typename Hasher = std::hash<ElementType>>
class TSet : public std::unordered_set<ElementType, Hasher>
{
	using Base = std::unordered_set<ElementType, Hasher>;
public:
	TSet() = default;
	int32 Num() const { return static_cast<int32>(Base::size()); }
	bool IsEmpty() const { return Base::empty(); }
	void Reset() { Base::clear(); }
	void Empty() { Base::clear(); }
	bool Contains(const ElementType& Elem) const { return Base::find(Elem) != Base::end(); }
	int32 Add(const ElementType& Elem) { Base::insert(Elem); return Num(); }
	int32 Remove(const ElementType& Elem) { return static_cast<int32>(Base::erase(Elem)); }
};

// ========================================================================
// TIndirectArray - backed by vector<unique_ptr>
// ========================================================================
template<typename T>
class TIndirectArray
{
	std::vector<std::unique_ptr<T>> Data;
public:
	TIndirectArray() = default;
	~TIndirectArray() = default;

	int32 Num() const { return static_cast<int32>(Data.size()); }
	bool IsEmpty() const { return Data.empty(); }

	T& operator[](int32 Idx) { return *Data[Idx]; }
	const T& operator[](int32 Idx) const { return *Data[Idx]; }

	int32 Add(T* Item) { Data.emplace_back(Item); return Num() - 1; }
	int32 Emplace(T* Item) { Data.emplace_back(Item); return Num() - 1; }

	void Reserve(int32 Number) { Data.reserve(Number); }
	void Reset() { Data.clear(); }
	void Empty() { Data.clear(); }
	void RemoveAt(int32 Idx) { Data.erase(Data.begin() + Idx); }
bool IsValidIndex(int32 Idx) const { return Idx >= 0 && Idx < Num(); }

	void Insert(T* Item, int32 Idx) { Data.insert(Data.begin() + Idx, std::unique_ptr<T>(Item)); }

	// Iterator support - dereference unique_ptr to get T&
	class Iterator
	{
		typename std::vector<std::unique_ptr<T>>::iterator It;
	public:
		Iterator(typename std::vector<std::unique_ptr<T>>::iterator InIt) : It(InIt) {}
		T& operator*() { return **It; }
		T* operator->() { return It->get(); }
		Iterator& operator++() { ++It; return *this; }
		bool operator!=(const Iterator& Other) const { return It != Other.It; }
	};
	class ConstIterator
	{
		typename std::vector<std::unique_ptr<T>>::const_iterator It;
	public:
		ConstIterator(typename std::vector<std::unique_ptr<T>>::const_iterator InIt) : It(InIt) {}
		const T& operator*() const { return **It; }
		const T* operator->() const { return It->get(); }
		ConstIterator& operator++() { ++It; return *this; }
		bool operator!=(const ConstIterator& Other) const { return It != Other.It; }
	};

	Iterator begin() { return Iterator(Data.begin()); }
	Iterator end() { return Iterator(Data.end()); }
	ConstIterator begin() const { return ConstIterator(Data.begin()); }
	ConstIterator end() const { return ConstIterator(Data.end()); }
};

// ========================================================================
// TPair - alias for std::pair
// ========================================================================
template<typename K, typename V>
using TPair = std::pair<K, V>;

// ========================================================================
// Hash helpers for UE types
// ========================================================================
struct FIndex2iHash
{
	size_t operator()(const FIndex2i& K) const
	{
		size_t H = std::hash<int32>()(K.A);
		H ^= std::hash<int32>()(K.B) + 0x9e3779b9 + (H << 6) + (H >> 2);
		return H;
	}
};

struct FIndex3iHash
{
	size_t operator()(const FIndex3i& K) const
	{
		size_t H = std::hash<int32>()(K.A);
		H ^= std::hash<int32>()(K.B) + 0x9e3779b9 + (H << 6) + (H >> 2);
		H ^= std::hash<int32>()(K.C) + 0x9e3779b9 + (H << 6) + (H >> 2);
		return H;
	}
};

struct FVector3dHash
{
	size_t operator()(const FVector3d& V) const
	{
		size_t H = std::hash<double>()(V.X);
		H ^= std::hash<double>()(V.Y) + 0x9e3779b9 + (H << 6) + (H >> 2);
		H ^= std::hash<double>()(V.Z) + 0x9e3779b9 + (H << 6) + (H >> 2);
		return H;
	}
};

// TMap specializations for common key types
template<typename V>
using TMapIndex2i = TMap<FIndex2i, V, FIndex2iHash>;

template<typename V>
using TMapIndex3i = TMap<FIndex3i, V, FIndex3iHash>;
