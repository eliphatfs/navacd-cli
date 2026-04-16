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

// TFixedAllocator stub - identical to TInlineAllocator for our purposes
template<int32 N>
using TFixedAllocator = std::allocator<int>; // Never actually used, just a placeholder

// TMemStackAllocator / FDefaultSetAllocator stubs
class FDefaultSetAllocator {};

// ========================================================================
// TArray - backed by std::vector with UE-style API
// ========================================================================
template<typename T, typename AllocatorOrPolicy = std::allocator<T>>
class TArray : public std::vector<T, std::allocator<T>>
{
	using Base = std::vector<T, std::allocator<T>>;
public:
	// UE range-for iterator aliases
	using ElementType = T;
	using RangedForIteratorType = typename Base::iterator;
	using RangedForConstIteratorType = typename Base::const_iterator;

	TArray() = default;
	TArray(std::initializer_list<T> Init) : Base(Init) {}
	explicit TArray(int32 InNum) : Base(InNum) {}
	TArray(int32 InNum, const T& Val) : Base(InNum, Val) {}
	TArray(const T* Ptr, int32 Count) : Base(Ptr, Ptr + Count) {}

	// UE-style API
	int32 Num() const { return static_cast<int32>(Base::size()); }
	size_t NumBytes() const { return Base::size() * sizeof(T); }
	void SetNum(int32 NewNum) { Base::resize(NewNum); }
	void SetNum(int32 NewNum, EAllowShrinking) { Base::resize(NewNum); }
	void SetNumUninitialized(int32 NewNum) { Base::resize(NewNum); }
	void SetNumUninitialized(int32 NewNum, EAllowShrinking) { Base::resize(NewNum); }
	// Use reserve + in-place default-construct so the element type only needs
	// to be default-constructible (not copy- or move-constructible).  This
	// matters for types like std::atomic<T> which libstdc++'s vector::resize
	// can't instantiate due to its move branch, even when nothing actually
	// needs to be moved.
	void SetNumZeroed(int32 NewNum)
	{
		if (NewNum < (int32)Base::size())
		{
			// pop_back only needs the destructor; avoids erase()'s move ops
			while ((int32)Base::size() > NewNum) { Base::pop_back(); }
			return;
		}
		if ((int32)Base::capacity() < NewNum) { Base::reserve(NewNum); }
		while ((int32)Base::size() < NewNum) { Base::emplace_back(); }
	}
	void SetNumZeroed(int32 NewNum, EAllowShrinking) { SetNumZeroed(NewNum); }
	void Init(const T& Element, int32 Number) { Base::assign(Number, Element); }
	void Reserve(int32 Number) { Base::reserve(Number); }
	int32 Add(const T& Item) { Base::push_back(Item); return Num() - 1; }
	int32 Add(T&& Item) { Base::push_back(std::move(Item)); return Num() - 1; }
	template<typename... ArgsType>
	int32 Emplace(ArgsType&&... Args) { Base::emplace_back(std::forward<ArgsType>(Args)...); return Num() - 1; }
	template<typename... ArgsType>
	T& Emplace_GetRef(ArgsType&&... Args) { Base::emplace_back(std::forward<ArgsType>(Args)...); return Base::back(); }
	int32 AddUninitialized(int32 Count = 1) { int32 Start = Num(); Base::resize(Num() + Count); return Start; }
	int32 Add_GetRef_Index() { Base::emplace_back(); return Num() - 1; }
	T& AddZeroed_GetRef() { Base::emplace_back(); return Base::back(); }
	T& Add_GetRef(const T& Item) { Base::push_back(Item); return Base::back(); }
	T& Add_GetRef(T&& Item) { Base::push_back(std::move(Item)); return Base::back(); }
	void Append(const TArray& Source) { Base::insert(Base::end(), Source.begin(), Source.end()); }
	// Append from a TArrayView — restricted to types that aren't TArray (SFINAE)
	template<typename ViewType,
		typename = std::enable_if_t<
			!std::is_base_of_v<TArray, std::decay_t<ViewType>> &&
			!std::is_same_v<TArray, std::decay_t<ViewType>>>>
	auto Append(const ViewType& Source) -> decltype(Source.GetData(), Source.Num(), void())
	{
		Base::insert(Base::end(), Source.GetData(), Source.GetData() + Source.Num());
	}
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
	void RemoveAt(int32 Index, EAllowShrinking) { Base::erase(Base::begin() + Index); }
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
	int32 RemoveSingle(const T& Item)
	{
		auto It = std::find(Base::begin(), Base::end(), Item);
		if (It == Base::end()) return 0;
		Base::erase(It);
		return 1;
	}
	int32 RemoveSingleSwap(const T& Item, EAllowShrinking = EAllowShrinking::Yes)
	{
		auto It = std::find(Base::begin(), Base::end(), Item);
		if (It == Base::end()) return 0;
		if (It != Base::end() - 1) std::iter_swap(It, Base::end() - 1);
		Base::pop_back();
		return 1;
	}
	template<typename Predicate>
	int32 RemoveAll(Predicate Pred)
	{
		auto It = std::remove_if(Base::begin(), Base::end(), Pred);
		int32 Removed = static_cast<int32>(Base::end() - It);
		Base::erase(It, Base::end());
		return Removed;
	}
	template<typename Predicate>
	int32 RemoveAllSwap(Predicate Pred)
	{
		int32 Removed = 0;
		for (int32 i = Num() - 1; i >= 0; --i)
		{
			if (Pred((*this)[i])) { RemoveAtSwap(i); ++Removed; }
		}
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
	T Pop(EAllowShrinking) { return Pop(); }

	T& Top() { return Base::back(); }
	const T& Top() const { return Base::back(); }
	void Push(const T& Item) { Base::push_back(Item); }
	void Push(T&& Item) { Base::push_back(std::move(Item)); }

	// Allow implicit conversion to span-like views
	operator std::span<const T>() const { return std::span<const T>(Base::data(), Base::size()); }
	operator std::span<T>() { return std::span<T>(Base::data(), Base::size()); }
};

// ========================================================================
// TArray specialization for std::atomic<T>
//
// std::vector<std::atomic<T>> is unusable because std::atomic is neither
// copy- nor move-constructible, and libstdc++ vector instantiates those
// branches in reserve/resize even when they'd never run dynamically.
// We back atomic TArrays with a heap-allocated raw array instead; only
// SetNumZeroed / operator[] are needed by the ported sources.
// ========================================================================
template<typename T, typename AllocatorOrPolicy>
class TArray<std::atomic<T>, AllocatorOrPolicy>
{
	std::unique_ptr<std::atomic<T>[]> Data;
	int32 Count = 0;
public:
	using ElementType = std::atomic<T>;

	TArray() = default;
	TArray(const TArray&) = delete;
	TArray& operator=(const TArray&) = delete;
	TArray(TArray&&) = default;
	TArray& operator=(TArray&&) = default;
	~TArray() = default;

	int32 Num() const { return Count; }
	bool IsEmpty() const { return Count == 0; }
	std::atomic<T>* GetData() { return Data.get(); }
	const std::atomic<T>* GetData() const { return Data.get(); }
	std::atomic<T>* data() { return Data.get(); }
	const std::atomic<T>* data() const { return Data.get(); }

	std::atomic<T>& operator[](int32 Idx) { return Data[Idx]; }
	const std::atomic<T>& operator[](int32 Idx) const { return Data[Idx]; }

	void SetNumZeroed(int32 NewNum)
	{
		Data.reset(new std::atomic<T>[NewNum]());
		Count = NewNum;
		for (int32 i = 0; i < NewNum; ++i) { Data[i].store(T{}, std::memory_order_relaxed); }
	}
	void SetNumZeroed(int32 NewNum, EAllowShrinking) { SetNumZeroed(NewNum); }

	std::atomic<T>* begin() { return Data.get(); }
	std::atomic<T>* end()   { return Data.get() + Count; }
	const std::atomic<T>* begin() const { return Data.get(); }
	const std::atomic<T>* end()   const { return Data.get() + Count; }
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

	// Allow TArrayView<const U> to bind to a TArray<U>& (UE's TConstArrayView<U> == TArrayView<const U>).
	template<typename U, typename = std::enable_if_t<std::is_same_v<T, const U>>>
	TArrayView(const TArray<U>& V) : Data_(V.data()), Num_(V.Num()) {}
	template<typename U, typename A, typename = std::enable_if_t<std::is_same_v<T, const U>>>
	TArrayView(const TArray<U, A>& V) : Data_(V.data()), Num_(V.Num()) {}
	template<typename U, typename = std::enable_if_t<std::is_same_v<T, const U>>>
	TArrayView(const std::vector<U>& V) : Data_(V.data()), Num_(static_cast<int32>(V.size())) {}
	// Allow TArrayView<const U> to bind to a TArrayView<U>& (const-qualification conversion)
	template<typename U, typename = std::enable_if_t<std::is_same_v<T, const U>>>
	TArrayView(const TArrayView<U>& V) : Data_(V.GetData()), Num_(V.Num()) {}

	// Allow TArrayView<const Derived> to bind to a TArrayView<const Base>&
	// (layout-compatible base-to-derived reinterpret). Used for FIntVector3
	// which inherits from FIntVector with identical layout.
	template<typename U, typename = std::enable_if_t<
		!std::is_same_v<T, const U> &&
		std::is_const_v<T> &&
		std::is_base_of_v<U, std::remove_const_t<T>> &&
		sizeof(U) == sizeof(std::remove_const_t<T>)>>
	TArrayView(const TArrayView<const U>& V)
		: Data_(reinterpret_cast<T*>(const_cast<U*>(V.GetData()))), Num_(V.Num()) {}


	T* data() const { return Data_; }
	T* GetData() const { return Data_; }
	int32 Num() const { return Num_; }
	bool IsEmpty() const { return Num_ == 0; }
	T& operator[](int32 Idx) { return Data_[Idx]; }
	const T& operator[](int32 Idx) const { return Data_[Idx]; }
	T* begin() const { return Data_; }
	T* end() const { return Data_ + Num_; }
	bool Contains(const T& Item) const {
		for (int32 i = 0; i < Num_; ++i) if (Data_[i] == Item) return true;
		return false;
	}
	int32 Find(const T& Item) const {
		for (int32 i = 0; i < Num_; ++i) if (Data_[i] == Item) return i;
		return INDEX_NONE;
	}
};

// MakeArrayView helpers
template<typename T>
inline TArrayView<T> MakeArrayView(T* Data, int32 Count) { return TArrayView<T>(Data, Count); }
template<typename T>
inline TArrayView<T> MakeArrayView(TArray<T>& Arr) { return TArrayView<T>(Arr.GetData(), Arr.Num()); }
template<typename T>
inline TArrayView<const T> MakeArrayView(const TArray<T>& Arr) { return TArrayView<const T>(Arr.GetData(), Arr.Num()); }

// ========================================================================
// TConstArrayView - read-only array view (UE typedef)
// ========================================================================
template<typename T>
using TConstArrayView = TArrayView<const T>;

// ========================================================================
// TPair - forward declaration (see below for definition)
// ========================================================================
template<typename K, typename V>
struct TPair;

// ========================================================================
// TMap - backed by std::unordered_map
// ========================================================================
template<typename KeyType, typename ValueType, typename Hasher = std::hash<KeyType>>
class TMap : public std::unordered_map<KeyType, ValueType, Hasher>
{
	using Base = std::unordered_map<KeyType, ValueType, Hasher>;
public:
	using PairType = TPair<KeyType, ValueType>;

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
	ValueType& FindOrAdd(KeyType&& Key) { return Base::operator[](std::move(Key)); }
	// FindOrAdd(Key, Default): insert Default if Key is absent, return reference either way.
	template<typename DefaultType>
	ValueType& FindOrAdd(const KeyType& Key, DefaultType&& Default)
	{
		auto It = Base::find(Key);
		if (It != Base::end()) return It->second;
		return Base::emplace(Key, std::forward<DefaultType>(Default)).first->second;
	}

	// UE-style Add returns reference to inserted value
	ValueType& Add(const KeyType& Key, const ValueType& Value) { return (Base::operator[](Key) = Value); }
	ValueType& Add(const KeyType& Key, ValueType&& Value) { return (Base::operator[](Key) = std::move(Value)); }
	ValueType& Add(KeyType&& Key, ValueType&& Value) { auto& Slot = Base::operator[](std::move(Key)); Slot = std::move(Value); return Slot; }
	ValueType& Add(const KeyType& Key) { return Base::operator[](Key); }

	// UE Emplace on TMap: same semantics as Add
	template<typename... ArgsType>
	ValueType& Emplace(const KeyType& Key, ArgsType&&... Args)
	{
		ValueType& Slot = Base::operator[](Key);
		Slot = ValueType(std::forward<ArgsType>(Args)...);
		return Slot;
	}
	template<typename... ArgsType>
	ValueType& Emplace(KeyType&& Key, ArgsType&&... Args)
	{
		ValueType& Slot = Base::operator[](std::move(Key));
		Slot = ValueType(std::forward<ArgsType>(Args)...);
		return Slot;
	}

	// FindAndRemoveChecked: returns the removed value (asserts if missing)
	ValueType FindAndRemoveChecked(const KeyType& Key)
	{
		auto It = Base::find(Key);
		assert(It != Base::end());
		ValueType Val = std::move(It->second);
		Base::erase(It);
		return Val;
	}

	void Reserve(int32 N) { Base::reserve(static_cast<size_t>(N)); }

	int32 Num() const { return static_cast<int32>(Base::size()); }
	bool IsEmpty() const { return Base::empty(); }
	void Reset() { Base::clear(); }
	void Empty() { Base::clear(); }
	int32 Remove(const KeyType& Key) { return static_cast<int32>(Base::erase(Key)); }
	bool Contains(const KeyType& Key) const { return Base::find(Key) != Base::end(); }

	ValueType& operator[](const KeyType& Key) { return *Find(Key); }
	const ValueType& operator[](const KeyType& Key) const { return *Find(Key); }

	// Iterator wrappers that expose TPair with .Key/.Value
	class Iterator
	{
		typename Base::iterator It;
	public:
		Iterator(typename Base::iterator InIt) : It(InIt) {}
		PairType& operator*() const { return reinterpret_cast<PairType&>(*It); }
		PairType* operator->() const { return reinterpret_cast<PairType*>(&*It); }
		Iterator& operator++() { ++It; return *this; }
		bool operator!=(const Iterator& Other) const { return It != Other.It; }
		bool operator==(const Iterator& Other) const { return It == Other.It; }
	};
	class ConstIterator
	{
		typename Base::const_iterator It;
	public:
		ConstIterator(typename Base::const_iterator InIt) : It(InIt) {}
		const PairType& operator*() const { return reinterpret_cast<const PairType&>(*It); }
		const PairType* operator->() const { return reinterpret_cast<const PairType*>(&*It); }
		ConstIterator& operator++() { ++It; return *this; }
		bool operator!=(const ConstIterator& Other) const { return It != Other.It; }
		bool operator==(const ConstIterator& Other) const { return It == Other.It; }
	};

	Iterator begin() { return Iterator(Base::begin()); }
	Iterator end() { return Iterator(Base::end()); }
	ConstIterator begin() const { return ConstIterator(Base::begin()); }
	ConstIterator end() const { return ConstIterator(Base::end()); }

	// UE-style FConstIterator: has explicit bool, Key(), Value(), operator++
	class FConstIterator
	{
		typename Base::const_iterator It;
		typename Base::const_iterator End;
	public:
		FConstIterator(typename Base::const_iterator InIt, typename Base::const_iterator InEnd) : It(InIt), End(InEnd) {}
		explicit operator bool() const { return It != End; }
		const KeyType& Key() const { return It->first; }
		const ValueType& Value() const { return It->second; }
		FConstIterator& operator++() { ++It; return *this; }
	};
	FConstIterator CreateConstIterator() const { return FConstIterator(Base::begin(), Base::end()); }

	void GenerateKeyArray(TArray<KeyType>& OutKeys) const
	{
		OutKeys.Reset();
		OutKeys.Reserve(Num());
		for (const auto& Pair : static_cast<const Base&>(*this)) OutKeys.Add(Pair.first);
	}
	void GenerateValueArray(TArray<ValueType>& OutValues) const
	{
		OutValues.Reset();
		OutValues.Reserve(Num());
		for (const auto& Pair : static_cast<const Base&>(*this)) OutValues.Add(Pair.second);
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

	// UE's FindKey returns a KeyType* for a given Value (reverse lookup). Returning nullptr if no pair maps a key to Value.
	const KeyType* FindKey(const ValueType& Value) const
	{
		for (const auto& Pair : *this)
		{
			if (Pair.second == Value) return &Pair.first;
		}
		return nullptr;
	}

	// UE MultiFind: append all values matching Key into OutValues. bMaintainOrder ignored in this stub.
	void MultiFind(const KeyType& Key, TArray<ValueType>& OutValues, bool bMaintainOrder = false) const
	{
		auto Range = Base::equal_range(Key);
		for (auto It = Range.first; It != Range.second; ++It) OutValues.Add(It->second);
	}

	// UE RemoveSingle: erase a single Key-Value match; returns count removed (0 or 1).
	int32 RemoveSingle(const KeyType& Key, const ValueType& Value)
	{
		auto Range = Base::equal_range(Key);
		for (auto It = Range.first; It != Range.second; ++It)
		{
			if (It->second == Value) { Base::erase(It); return 1; }
		}
		return 0;
	}

	// UE CreateConstKeyIterator: bool-convertible iterator over values for a specific key.
	class FConstKeyIterator
	{
		typename Base::const_iterator It;
		typename Base::const_iterator End;
	public:
		FConstKeyIterator(typename Base::const_iterator InIt, typename Base::const_iterator InEnd) : It(InIt), End(InEnd) {}
		explicit operator bool() const { return It != End; }
		const KeyType& Key() const { return It->first; }
		const ValueType& Value() const { return It->second; }
		FConstKeyIterator& operator++() { ++It; return *this; }
	};

	FConstKeyIterator CreateConstKeyIterator(const KeyType& Key) const
	{
		auto Range = Base::equal_range(Key);
		return FConstKeyIterator(Range.first, Range.second);
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
	TSet(const TArray<ElementType>& Arr) { for (const auto& E : Arr) Base::insert(E); }
	int32 Num() const { return static_cast<int32>(Base::size()); }
	bool IsEmpty() const { return Base::empty(); }
	void Reset() { Base::clear(); }
	void Empty() { Base::clear(); }
	void Reserve(int32 N) { Base::reserve(static_cast<size_t>(N)); }
	bool Contains(const ElementType& Elem) const { return Base::find(Elem) != Base::end(); }
	int32 Add(const ElementType& Elem) { Base::insert(Elem); return Num(); }
	int32 Add(const ElementType& Elem, bool* bAlreadyInSet) { auto R = Base::insert(Elem); if (bAlreadyInSet) *bAlreadyInSet = !R.second; return Num(); }
	int32 Add(ElementType&& Elem, bool* bAlreadyInSet) { auto R = Base::insert(std::move(Elem)); if (bAlreadyInSet) *bAlreadyInSet = !R.second; return Num(); }
	int32 Remove(const ElementType& Elem) { return static_cast<int32>(Base::erase(Elem)); }

	void Append(const TArray<ElementType>& Arr) { for (const auto& E : Arr) Base::insert(E); }
	void Append(const TSet& Other) { for (const auto& E : Other) Base::insert(E); }

	TArray<ElementType> Array() const
	{
		TArray<ElementType> Result;
		Result.Reserve(Num());
		for (const auto& E : *this) Result.Add(E);
		return Result;
	}
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

	// TIndirectArray owns its pointers; it must not be copied.
	TIndirectArray(const TIndirectArray&) = delete;
	TIndirectArray& operator=(const TIndirectArray&) = delete;
	TIndirectArray(TIndirectArray&&) = default;
	TIndirectArray& operator=(TIndirectArray&&) = default;

	int32 Num() const { return static_cast<int32>(Data.size()); }
	bool IsEmpty() const { return Data.empty(); }

	T& operator[](int32 Idx) { return *Data[Idx]; }
	const T& operator[](int32 Idx) const { return *Data[Idx]; }

	int32 Add(T* Item) { Data.emplace_back(Item); return Num() - 1; }
	int32 Emplace(T* Item) { Data.emplace_back(Item); return Num() - 1; }

	void Reserve(int32 Number) { Data.reserve(Number); }
	void Reset() { Data.clear(); }
	void Empty(int32 NewSize = 0) { Data.clear(); if (NewSize > 0) Data.reserve(NewSize); }
	void RemoveAt(int32 Idx) { Data.erase(Data.begin() + Idx); }
	void RemoveAt(int32 Idx, int32 Count) { Data.erase(Data.begin() + Idx, Data.begin() + Idx + Count); }
	void RemoveAt(int32 Idx, EAllowShrinking) { Data.erase(Data.begin() + Idx); }
	void RemoveAtSwap(int32 Idx, EAllowShrinking = EAllowShrinking::Yes)
	{
		if (Idx != (int32)Data.size() - 1) std::swap(Data[Idx], Data.back());
		Data.pop_back();
	}
	void Swap(int32 A, int32 B) { std::swap(Data[A], Data[B]); }
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
// TPair - layout-compatible wrapper around std::pair<const K, V>.
// Provides UE-style .Key and .Value reference members. Layout matches the
// underlying std::pair, so references to std::pair<const K,V>& can be
// reinterpret_cast to TPair<K,V>& (used by TMap::Iterator).
// ========================================================================
template<typename K, typename V>
struct TPair
{
	const K Key;
	V Value;

	TPair() : Key(), Value() {}
	TPair(const K& InKey, const V& InValue) : Key(InKey), Value(InValue) {}
	TPair(const K& InKey, V&& InValue) : Key(InKey), Value(std::move(InValue)) {}
	TPair(K&& InKey, V&& InValue) : Key(std::move(InKey)), Value(std::move(InValue)) {}
};

template<typename K, typename V>
inline TPair<K, V> MakeTPair(const K& Key, const V& Value) { return TPair<K, V>(Key, Value); }

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
