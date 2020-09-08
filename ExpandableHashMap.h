#ifndef EHM_H_
#define EHM_H_

// ExpandableHashMap.h

// Dean Jones
// 005-299-127

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
	ExpandableHashMap(double maximumLoadFactor = 0.5); // constructor
	~ExpandableHashMap(); // destructor; deletes all items in the hashmap
	void reset(); // resets the hashmap back to 8 buckets; deletes all itemms
	int size() const; // returns the number of associations in the hashmap
	
	// The associate method associates one item (key) with another (value).
	// If no association currently exists with that key, this method inserts
	// a new association into the hashmap with that key/value pair. If there is
	// already an association with that key in the hashmap, then the item
	// associated with that key is replaced by the second parameter (value).
	// Thus, the hashmap must contain no duplicate keys.
	void associate(const KeyType& key, const ValueType& value);

	// If no association exists with the given key, return nullptr; otherwise,
	// return a pointer to the value associated with that key. This pointer can be
	// used to examine that value, and if the hashmap is allowed to be modified, to
	// modify that value directly within the map (the second overload enables
	// this). Using a little C++ magic, we have implemented it in terms of the
	// first overload, which you must implement.

	// for a map that can't be modified, return a pointer to const ValueType
	const ValueType* find(const KeyType& key) const;

	// for a modifiable map, return a pointer to modifiable ValueType
	ValueType* find(const KeyType& key)
	{
		return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
	}

	// C++11 syntax for preventing copying and assignment
	ExpandableHashMap(const ExpandableHashMap&) = delete;
	ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
	// Number of buckets, associations
	unsigned int m_buckets, m_assoc;
	// Load, KVPair (Key-Value) struct, and dynamically allocated array m_map of KVPair pointers
	double m_load;
	struct KVPair {
		KeyType key;
		ValueType value;
		KVPair* next;
	};
	KVPair** m_map;
	// Helper function for adding a KV pair to a bigger map when the load would have been reached
	void addNewKVPair(KVPair**& map, unsigned int buckets, KeyType key, ValueType value);
};

// EHM has 8 buckets and no associations. Every pointer in m_map should initially be nullptr.
template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor): m_buckets(8), m_assoc(0), m_load(maximumLoadFactor), m_map(new KVPair*[m_buckets])
{
	// If load wasn't positive, set it to default of 0.5
	if (maximumLoadFactor <= 0) {
		m_load = 0.5;
	}
	for (unsigned int i = 0; i < m_buckets; ++i) {
		m_map[i] = nullptr;
	}
}

// Delete every entry in m_map
template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
	// For each bucket in the map...
	for (unsigned int i = 0; i < m_buckets; ++i) {
		KVPair* kv = m_map[i];
		// While a pointer at the bucket isn't nullptr...
		while (kv != nullptr)  {
			// Delete the KVPair the pointer refers to
			KVPair* nextkv = kv->next;
			delete kv;
			kv = nextkv;
		}
	}
	// Delete any dynamically allocated memory still left
	delete[] m_map;
}

// Resets the EHM to 8 buckets and no associations
template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
	// Like the destructor, delete all the KVPairs in the map
	for (unsigned int i = 0; i < m_buckets; ++i) {
		KVPair* kv = m_map[i];
		while (kv != nullptr) {
			KVPair* nextkv = kv->next;
			delete kv;
			kv = nextkv;
		}
	}
	m_buckets = 8;
	m_assoc = 0;
	// Reset the buckets to nullptr
	for (unsigned int i = 0; i < m_buckets; ++i) {
		m_map[i] = nullptr;
	}
}

// Returns number of associations made in map
template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
	return m_assoc;
}

// Associates a given key with a given value
template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
	// Try to find the key first
	ValueType* valuePtr = find(key);
	// If it's there, update its value
	if (valuePtr != nullptr) {
		*valuePtr = value;
		return;
	}

	// Otherwise, make a new association
	++m_assoc;

	// If the number of associations exceeds the load...
	if (m_assoc >= double(m_buckets) * m_load) {
		// Double the number of buckets
		m_buckets *= 2;
		// Make a new map with the new number of buckets. Set all its entries to nullptr initially.
		KVPair** newMap = new KVPair*[m_buckets];
		for (unsigned int i = 0; i < m_buckets; ++i) {
			newMap[i] = nullptr;
		}

		// For all the buckets in the old map...
		for (int i = 0; i < m_buckets / 2; ++i) {
			// Get the KVPairs and add them to the new map. Also, delete them from the old one.
			KVPair* kv = m_map[i];
			while (kv != nullptr) {
				KVPair* next = kv->next;
				addNewKVPair(newMap, m_buckets, kv->key, kv->value);
				delete kv;
				kv = next;
			}
		}
		// Delete any remaining memory the pointer refers to and take on the new map
		delete[] m_map;
		m_map = newMap;
	}

	// Add the given key and value into the map
	addNewKVPair(m_map, m_buckets, key, value);
}

// Returns ptr to value if key is in the map or nullptr otherwise
template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
	unsigned int hasher(const KeyType& k); // prototype
	unsigned int index = hasher(key) % m_buckets; // the right index to look has to be from 0 to m_buckets - 1

	// Get KVPair pointer at index
	KVPair* kv = m_map[index];
	// While it's not nullptr...
	while (kv != nullptr) {
		// If its key is the same as in the input key, return the pointer to its value
		if (kv->key == key) {
			return &(kv->value);
		}
		// Otherwise, go to the next KVPair pointer
		kv = kv->next;
	}
	// Return nullptr if it wasn't there
	return nullptr;
}

// Adds key and value into the map
template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::addNewKVPair(KVPair**& map, unsigned int buckets, KeyType key, ValueType value) {
	// Again, use hasher to get the right index
	unsigned int hasher(const KeyType & k); // prototype hash function
	unsigned int index = hasher(key) % buckets;

	// Insert the KVPair at the index by setting its next to the KVPair originally at the index
	KVPair* oldFront = map[index];
	KVPair* newFront = new KVPair{ key, value, oldFront };
	map[index] = newFront;
}
#endif