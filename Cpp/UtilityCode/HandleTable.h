//
// HandleTable
//

//
// TODO: 
// - Add proper error handling in case of:
//   1. No free handle table entries are left.
//   2. Trying to access a handle that has already been released.
//   3. Trying to access a handle that is out of range.
//		This shouldn't happen, unless the handle is corrupted.
//

#include <atomic>
#include <cassert>
#include <cstdint>
#include <type_traits>

namespace Utility {
// T must be trivial.
template <typename T, uint32_t _HandleBits>
class THandleTable
{
public:
	static_assert(_HandleBits == 32 || _HandleBits == 64,
				  "_HandleBits must be either 32 or 64");

	static_assert(std::is_trivial_v<T>, "THandleTable<T>, T must be trivial");

	using HandleType =
		std::conditional_t<_HandleBits == 64, uint64_t, uint32_t>;

private:
	struct Entry
	{
		uint32_t version;
		uint32_t nextFree;
		T obj;
	};

	// Divide the available handle bits into two equally sized parts.
	const uint32_t kIndexBits = _HandleBits >> 1;
	const uint32_t kVersionBits = _HandleBits >> 1;

	const uint32_t kIndexMask = static_cast<uint32_t>((1ULL << kIndexBits) - 1);
	const uint32_t kVersionMask = static_cast<uint32_t>((1ULL << kVersionBits) - 1);

	constexpr uint32_t GetHandleIndex(HandleType handle)
	{
		return static_cast<uint32_t>(handle & kIndexMask);
	}

	constexpr uint32_t GetHandleVersion(HandleType handle)
	{
		return static_cast<uint32_t>((handle >> kIndexBits) & kVersionMask);
	}

	constexpr HandleType ConstructHandle(HandleType index, HandleType version)
	{
		return (index & kIndexMask) | ((version & kVersionMask) << kIndexBits);
	}

public:
	THandleTable(uint32_t maxEntries)
		: m_MaxEntries(maxEntries)
	{
		m_Entries = new Entry[maxEntries]();
	}

	~THandleTable()
	{
		delete[] m_Entries;
	}

	HandleType Insert(const T& obj)
	{
		uint32_t index = GetFreeIndex();
		Entry& ent = m_Entries[index];
		ent.version++;
		ent.nextFree = 0;
		ent.obj = obj;
		return ConstructHandle(index, ent.version);
	}

	// Unsafe
	T& Get(HandleType handle)
	{
		return TryGetEntryAndValidateVersion(handle)->obj;
	}

	// Unsafe
	const T& Get(HandleType handle) const
	{
		return TryGetEntryAndValidateVersion(handle)->obj;
	}

	void Free(HandleType handle)
	{
		Entry& ent = *TryGetEntryAndValidateVersion(handle);
		uint32_t newHead = GetHandleIndex(handle);
		uint32_t currentHead = m_FreeListHead;
		do
		{
			ent.nextFree = currentHead;
		} while (!m_FreeListHead.compare_exchange_weak(currentHead, newHead));
	}

public:
	Entry* TryGetEntryAndValidateVersion(HandleType handle)
	{
		uint32_t index = GetHandleIndex(handle);
		uint32_t version = GetHandleVersion(handle);
		if (index >= m_MaxEntries)
			return nullptr;
		Entry& ent = m_Entries[index];
		if (ent.version != version)
			return nullptr;
		return &ent;
	}

	uint32_t GetFreeIndex()
	{
		uint32_t selectedEntry;
		if (m_FreeListHead == 0)
		{
			// This is OK because m_NextFreeEntry++ is atomic.
			selectedEntry = m_NextFreeEntry++;
			if (selectedEntry >= m_MaxEntries)
				throw;
			return selectedEntry;
		}

		// Use the free list to grab a free entry.
		do
		{
			selectedEntry = m_FreeListHead;
			if (!selectedEntry)
				throw;
		} while (!m_FreeListHead.compare_exchange_weak(
			selectedEntry, m_Entries[selectedEntry].nextFree));
		return selectedEntry;
	}

protected:
	// The first entry is reserved.
	std::atomic_uint32_t m_NextFreeEntry = 1;
	std::atomic_uint32_t m_FreeListHead = 0;
	uint32_t m_MaxEntries;
	Entry* m_Entries;
};
}
