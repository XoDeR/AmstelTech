#pragma once

#include "Core/Containers/Types.h"
#include "Core/Containers/Vector.h"

// #define RBTREE_VERIFY

namespace Rio
{

namespace MapFn
{
	// Returns the number of items in the map <m>
	template <typename TKey, typename TValue> uint32_t getCount(const Map<TKey, TValue>& m);

	// Returns whether the given <key> exists in the map <m>
	template <typename TKey, typename TValue> bool has(const Map<TKey, TValue>& m, const TKey& key);

	// Returns the value for the given <key> or <deffault> if the key does not exist in the map
	template <typename TKey, typename TValue> const TValue& get(const Map<TKey, TValue>& m, const TKey& key, const TValue& deffault);

	// Sets the <value> for the <key> in the map
	template <typename TKey, typename TValue> void set(Map<TKey, TValue>& m, const TKey& key, const TValue& value);

	// Removes the <key> from the map if it exists
	template <typename TKey, typename TValue> void remove(Map<TKey, TValue>& m, const TKey& key);

	// Removes all the items in the map
	// Calls destructor on the items
	template <typename TKey, typename TValue> void clear(Map<TKey, TValue>& m);

	// Returns a pointer to the first item in the map, can be used to efficiently iterate over the elements (in random order)
	template <typename TKey, typename TValue> const typename Map<TKey, TValue>::Node* begin(const Map<TKey, TValue>& m);
	template <typename TKey, typename TValue> const typename Map<TKey, TValue>::Node* end(const Map<TKey, TValue>& m);

} // namespace MapFn

namespace MapInternalFn
{
	const uint32_t BLACK = 0xB1B1B1B1u;
	const uint32_t RED = 0xEDEDEDEDu;
	const uint32_t NIL = 0xFFFFFFFFu;

	template <typename TKey, typename TValue>
	inline uint32_t getRoot(const Map<TKey, TValue>& m)
	{
		return m.root;
	}

	template <typename TKey, typename TValue>
	inline uint32_t getParent(const Map<TKey, TValue>& m, uint32_t n)
	{
		RIO_ASSERT(n < VectorFn::getCount(m.dataVector), "Index out of bounds (size = %d, n = %d)", VectorFn::getCount(m.dataVector), n);
		return m.dataVector[n].parent;
	}

	template <typename TKey, typename TValue>
	inline uint32_t getLeft(const Map<TKey, TValue>& m, uint32_t n)
	{
		RIO_ASSERT(n < VectorFn::getCount(m.dataVector), "Index out of bounds (size = %d, n = %d)", VectorFn::getCount(m.dataVector), n);
		return m.dataVector[n].left;
	}

	template <typename TKey, typename TValue>
	inline uint32_t getRight(const Map<TKey, TValue>& m, uint32_t n)
	{
		RIO_ASSERT(n < VectorFn::getCount(m.dataVector), "Index out of bounds (size = %d, n = %d)", VectorFn::getCount(m.dataVector), n);
		return m.dataVector[n].right;
	}

	template <typename TKey, typename TValue>
	inline uint32_t getColor(const Map<TKey, TValue>& m, uint32_t n)
	{
		RIO_ASSERT(n < VectorFn::getCount(m.dataVector), "Index out of bounds (size = %d, n = %d)", VectorFn::getCount(m.dataVector), n);
		return m.dataVector[n].color;
	}

#ifdef RBTREE_VERIFY
	template<typename TKey, typename TValue>
	inline int32_t debugVerify(Map<TKey, TValue>& m, uint32_t n)
	{
		if (n == m.sentinel)
		{
			return 0;
		}

		if (getLeft(m, n) != m.sentinel)
		{
			RIO_ASSERT(getParent(m, getLeft(m, n)) == n, "Bad RBTree");
			RIO_ASSERT(m.dataVector[getLeft(m, n)].pair.first < m.dataVector[n].pair.first, "Bad RBTree");
		}

		if (getRight(m, n) != m.sentinel)
		{
			RIO_ASSERT(getParent(m, getRight(m, n)) == n, "Bad RBTree");
			RIO_ASSERT(m.dataVector[n].pair.first < m.dataVector[getRight(m, n)].pair.first, "Bad RBTree");
		}

		int32_t bhL = debugVerify(m, getLeft(m, n));
		int32_t bhR = debugVerify(m, getRight(m, n));
		RIO_ASSERT(bhL == bhR, "Bad RBTree");

		if (getColor(m, n) == BLACK)
		{
			bhL += 1;
		}
		else
		{
			if (getParent(m, n) != NIL && getColor(m, getParent(m, n)) == RED)
			{
				RIO_ASSERT(false, "Bad RBTree");
			}
		}

		return bhL;
	}
#endif // RBTREE_VERIFY

	template <typename TKey, typename TValue>
	inline uint32_t min(const Map<TKey, TValue>& m, uint32_t x)
	{
		if (x == m.sentinel)
		{
			return x;
		}

		while (getLeft(m, x) != m.sentinel)
		{
			x = getLeft(m, x);
		}

		return x;
	}

	template <typename TKey, typename TValue>
	inline uint32_t max(const Map<TKey, TValue>& m, uint32_t x)
	{
		if (x == m.sentinel)
		{
			return x;
		}

		while (getRight(m, x) != m.sentinel)
		{
			x = getRight(m, x);
		}

		return x;
	}

	template <typename TKey, typename TValue>
	inline uint32_t successor(const Map<TKey, TValue>& m, uint32_t x)
	{
		if (getRight(m, x) != m.sentinel)
		{
			return min(m, getRight(m, x));
		}

		uint32_t y = getParent(m, x);

		while (y != NIL && x == getRight(m, y))
		{
			x = y;
			y = getParent(m, y);
		}

		return y;
	}

	template <typename TKey, typename TValue>
	inline uint32_t predecessor(const Map<TKey, TValue>& m, uint32_t x)
	{
		if (getLeft(m, x) != m.sentinel)
		{
			return max(m, getLeft(m, x));
		}

		uint32_t y = getParent(m, x);

		while (y != NIL && x == getLeft(m, y))
		{
			x = y;
			y = getParent(m, y);
		}

		return y;
	}

	template <typename TKey, typename TValue>
	inline void rotateLeft(Map<TKey, TValue>& m, uint32_t x)
	{
		RIO_ASSERT(x < VectorFn::getCount(m.dataVector), "Index out of bounds (size = %d, n = %d)", VectorFn::getCount(m.dataVector), x);

		uint32_t y = getRight(m, x);
		m.dataVector[x].right = getLeft(m, y);

		if (getLeft(m, y) != m.sentinel)
		{
			m.dataVector[getLeft(m, y)].parent = x;
		}

		m.dataVector[y].parent = getParent(m, x);

		if (getParent(m, x) == NIL)
		{
			m.root = y;
		}
		else
		{
			if (x == getLeft(m, getParent(m, x)))
			{
				m.dataVector[getParent(m, x)].left = y;
			}
			else
			{
				m.dataVector[getParent(m, x)].right = y;
			}
		}

		m.dataVector[y].left = x;
		m.dataVector[x].parent = y;
	}

	template <typename TKey, typename TValue>
	inline void rotateRight(Map<TKey, TValue>& m, uint32_t x)
	{
		RIO_ASSERT(x < VectorFn::getCount(m.dataVector), "Index out of bounds (size = %d, n = %d)", VectorFn::getCount(m.dataVector), x);

		uint32_t y = getLeft(m, x);
		m.dataVector[x].left = getRight(m, y);

		if (getRight(m, y) != m.sentinel)
		{
			m.dataVector[getRight(m, y)].parent = x;
		}

		m.dataVector[y].parent = getParent(m, x);

		if (getParent(m, x) == NIL)
		{
			m.root = y;
		}
		else
		{
			if (x == getLeft(m, getParent(m, x)))
			{
				m.dataVector[getParent(m, x)].left = y;
			}
			else
			{
				m.dataVector[getParent(m, x)].right = y;
			}
		}

		m.dataVector[y].right = x;
		m.dataVector[x].parent = y;
	}

	template <typename TKey, typename TValue>
	inline void destroy(Map<TKey, TValue>& m, uint32_t n)
	{
		RIO_ASSERT(n < VectorFn::getCount(m.dataVector), "Index out of bounds (size = %d, n = %d)", VectorFn::getCount(m.dataVector), n);

		uint32_t x = VectorFn::getCount(m.dataVector) - 1;

		if (x == m.root)
		{
			m.root = n;

			if (getLeft(m, x) != NIL)
			{
				m.dataVector[getLeft(m, x)].parent = n;
			}
			if (getRight(m, x) != NIL)
			{
				m.dataVector[getRight(m, x)].parent = n;
			}

			m.dataVector[n] = m.dataVector[x];
		}
		else
		{
			if (x != n)
			{
				if (x == getLeft(m, getParent(m, x)))
				{
					m.dataVector[getParent(m, x)].left = n;
				}
				else if (x == getRight(m, getParent(m, x)))
				{
					m.dataVector[getParent(m, x)].right = n;
				}

				if (getLeft(m, x) != NIL)
				{
					m.dataVector[getLeft(m, x)].parent = n;
				}
				if (getRight(m, x) != NIL)
				{
					m.dataVector[getRight(m, x)].parent = n;
				}

				m.dataVector[n] = m.dataVector[x];
			}
		}

		#ifdef RBTREE_VERIFY
			debugVerify(m, m.root);
		#endif // RBTREE_VERIFY

		VectorFn::popBack(m.dataVector);
	}

	template <typename TKey, typename TValue>
	inline void insertFixup(Map<TKey, TValue>& m, uint32_t n)
	{
		RIO_ASSERT(n < VectorFn::getCount(m.dataVector), "Index out of bounds (size = %d, n = %d)", VectorFn::getCount(m.dataVector), n);

		uint32_t x = 0;
		uint32_t y = 0;

		while (n != getRoot(m) && getColor(m, getParent(m, n)) == RED)
		{
			x = getParent(m, n);

			if (x == getLeft(m, getParent(m, x)))
			{
				y = getRight(m, getParent(m, x));

				if (getColor(m, y) == RED)
				{
					m.dataVector[x].color = BLACK;
					m.dataVector[y].color = BLACK;
					m.dataVector[getParent(m, x)].color = RED;
					n = getParent(m, x);
					continue;
				}
				else
				{
					if (n == getRight(m, x))
					{
						n = x;
						rotateLeft(m, n);
						x = getParent(m, n);
					}

					m.dataVector[x].color = BLACK;
					m.dataVector[getParent(m, x)].color = RED;
					rotateRight(m, getParent(m, x));
				}
			}
			else
			{
				y = getLeft(m, getParent(m, x));

				if (getColor(m, y) == RED)
				{
					m.dataVector[x].color = BLACK;
					m.dataVector[y].color = BLACK;
					m.dataVector[getParent(m, x)].color = RED;
					n = getParent(m, x);
					continue;
				}
				else
				{
					if (n == getLeft(m, x))
					{
						n = x;
						rotateRight(m, n);
						x = getParent(m, n);
					}

					m.dataVector[x].color = BLACK;
					m.dataVector[getParent(m, x)].color = RED;
					rotateLeft(m, getParent(m, x));
				}
			}
		}
	}

	template <typename TKey, typename TValue>
	inline uint32_t innerFind(const Map<TKey, TValue>& m, const TKey& key)
	{
		uint32_t x = m.root;

		while (x != m.sentinel)
		{
			if (m.dataVector[x].pair.first < key)
			{
				if (getRight(m, x) == m.sentinel)
				{
					return x;
				}

				x = getRight(m, x);
			}
			else if (key < m.dataVector[x].pair.first)
			{
				if (getLeft(m, x) == m.sentinel)
				{
					return x;
				}

				x = getLeft(m, x);
			}
			else
			{
				break;
			}
		}

		return x;
	}

	template <typename TKey, typename TValue>
	inline uint32_t findOrFail(const Map<TKey, TValue>& m, const TKey& key)
	{
		uint32_t p = innerFind(m, key);

		if (p != m.sentinel && m.dataVector[p].pair.first == key)
		{
			return p;
		}

		return NIL;
	}

} // namespace MapInternalFn

namespace MapFn
{
	template <typename TKey, typename TValue>
	uint32_t getCount(const Map<TKey, TValue>& m)
	{
		RIO_ASSERT(VectorFn::getCount(m.dataVector) > 0, "Bad Map"); // There should be at least sentinel
		return VectorFn::getCount(m.dataVector) - 1;
	}

	template <typename TKey, typename TValue>
	inline bool has(const Map<TKey, TValue>& m, const TKey& key)
	{
		return MapInternalFn::findOrFail(m, key) != MapInternalFn::NIL;
	}

	template <typename TKey, typename TValue>
	inline const TValue& get(const Map<TKey, TValue>& m, const TKey& key, const TValue& deffault)
	{
		uint32_t p = MapInternalFn::innerFind(m, key);

		if (p != m.sentinel && m.dataVector[p].pair.first == key)
		{
			return m.dataVector[p].pair.second;
		}

		return deffault;
	}

	template <typename TKey, typename TValue>
	inline void set(Map<TKey, TValue>& m, const TKey& key, const TValue& value)
	{
		typename Map<TKey, TValue>::Node node(*m.dataVector.allocator);
		node.pair.first = key;
		node.pair.second = value;
		node.color = MapInternalFn::RED;
		node.left = m.sentinel;
		node.right = m.sentinel;
		node.parent = MapInternalFn::NIL;
		uint32_t n = VectorFn::pushBack(m.dataVector, node);
		uint32_t x = m.root;
		uint32_t y = MapInternalFn::NIL;

		if (x == m.sentinel)
		{
			m.root = n;
		}
		else
		{
			while (x != m.sentinel)
			{
				y = x;

				if (key < m.dataVector[x].pair.first)
				{
					x = m.dataVector[x].left;
				}
				else
				{
					x = m.dataVector[x].right;
				}
			}

			if (key < m.dataVector[y].pair.first)
			{
				m.dataVector[y].left = n;
			}
			else
			{
				m.dataVector[y].right = n;
			}

			m.dataVector[n].parent = y;
		}

		MapInternalFn::insertFixup(m, n);
		m.dataVector[m.root].color = MapInternalFn::BLACK;
		#ifdef RBTREE_VERIFY
			MapInternalFn::debugVerify(m, m.root);
		#endif // RBTREE_VERIFY
	}

	template <typename TKey, typename TValue>
	inline void remove(Map<TKey, TValue>& m, const TKey& key)
	{
		using namespace MapInternalFn;

		uint32_t n = innerFind(m, key);

		if (!(m.dataVector[n].pair.first == key))
		{
			return;
		}

		uint32_t x = 0;
		uint32_t y = 0;

		if (getLeft(m, n) == m.sentinel || getRight(m, n) == m.sentinel)
		{
			y = n;
		}
		else
		{
			y = successor(m, n);
		}

		if (getLeft(m, y) != m.sentinel)
		{
			x = getLeft(m, y);
		}
		else
		{
			x = getRight(m, y);
		}

		m.dataVector[x].parent = getParent(m, y);

		if (getParent(m, y) != MapInternalFn::NIL)
		{
			if (y == getLeft(m, getParent(m, y)))
			{
				m.dataVector[getParent(m, y)].left = x;
			}
			else
			{
				m.dataVector[getParent(m, y)].right = x;
			}
		}
		else
		{
			m.root = x;
		}

		if (y != n)
		{
			m.dataVector[n].pair.first = m.dataVector[y].pair.first;
			m.dataVector[n].pair.second = m.dataVector[y].pair.second;
		}

		// Do the fixup
		if (getColor(m, y) == MapInternalFn::BLACK)
		{
			uint32_t y = 0;

			while (x != m.root && getColor(m, x) == MapInternalFn::BLACK)
			{
				if (x == getLeft(m, getParent(m, x)))
				{
					y = getRight(m, getParent(m, x));

					if (getColor(m, y) == MapInternalFn::RED)
					{
						m.dataVector[y].color = MapInternalFn::BLACK;
						m.dataVector[getParent(m, x)].color = MapInternalFn::RED;
						rotateLeft(m, getParent(m, x));
						y = getRight(m, getParent(m, x));
					}

					if (getColor(m, getLeft(m, y)) == MapInternalFn::BLACK && getColor(m, getRight(m, y)) == MapInternalFn::BLACK)
					{
						m.dataVector[y].color = MapInternalFn::RED;
						x = getParent(m, x);
					}
					else
					{
						if (getColor(m, getRight(m, y)) == MapInternalFn::BLACK)
						{
							m.dataVector[getLeft(m, y)].color = MapInternalFn::BLACK;
							m.dataVector[y].color = MapInternalFn::RED;
							rotateRight(m, y);
							y = getRight(m, getParent(m, x));
						}

						m.dataVector[y].color = getColor(m, getParent(m, x));
						m.dataVector[getParent(m, x)].color = MapInternalFn::BLACK;
						m.dataVector[getRight(m, y)].color = MapInternalFn::BLACK;
						rotateLeft(m, getParent(m, x));
						x = m.root;
					}
				}
				else
				{
					y = getLeft(m, getParent(m, x));

					if (getColor(m, y) == MapInternalFn::RED)
					{
						m.dataVector[y].color = MapInternalFn::BLACK;
						m.dataVector[getParent(m, x)].color = MapInternalFn::RED;
						rotateRight(m, getParent(m, x));
						y = getLeft(m, getParent(m, x));
					}

					if (getColor(m, getRight(m, y)) == MapInternalFn::BLACK && getColor(m, getLeft(m, y)) == MapInternalFn::BLACK)
					{
						m.dataVector[y].color = MapInternalFn::RED;
						x = getParent(m, x);
					}
					else
					{
						if (getColor(m, getLeft(m, y)) == MapInternalFn::BLACK)
						{
							m.dataVector[getRight(m, y)].color = MapInternalFn::BLACK;
							m.dataVector[y].color = MapInternalFn::RED;
							rotateLeft(m, y);
							y = getLeft(m, getParent(m, x));
						}

						m.dataVector[y].color = getColor(m, getParent(m, x));
						m.dataVector[getParent(m, x)].color = MapInternalFn::BLACK;
						m.dataVector[getLeft(m, y)].color = MapInternalFn::BLACK;
						rotateRight(m, getParent(m, x));
						x = m.root;
					}
				}
			}

			m.dataVector[x].color = MapInternalFn::BLACK;
		}

		destroy(m, y);
	 	#ifdef RBTREE_VERIFY
			MapInternalFn::debugVerify(m, m.root);
	 	#endif // RBTREE_VERIFY
	}

	template <typename TKey, typename TValue>
	void clear(Map<TKey, TValue>& m)
	{
		VectorFn::clear(m.dataVector);

		m.root = 0;
		m.sentinel = 0;

		typename Map<TKey, TValue>::Node r(*m.dataVector.allocator);
		r.left = MapInternalFn::NIL;
		r.right = MapInternalFn::NIL;
		r.parent = MapInternalFn::NIL;
		r.color = MapInternalFn::BLACK;
		VectorFn::pushBack(m.dataVector, r);
	}

	template <typename TKey, typename TValue>
	const typename Map<TKey, TValue>::Node* begin(const Map<TKey, TValue>& m)
	{
		return VectorFn::begin(m.dataVector) + 1; // Skip sentinel at index 0
	}

	template <typename TKey, typename TValue>
	const typename Map<TKey, TValue>::Node* end(const Map<TKey, TValue>& m)
	{
		return VectorFn::end(m.dataVector);
	}

} // namespace MapFn

template <typename TKey, typename TValue>
inline Map<TKey, TValue>::Map(Allocator& a)
	: dataVector(a)
{
	MapFn::clear(*this);
}

template <typename TKey, typename TValue>
inline const TValue& Map<TKey, TValue>::operator[](const TKey& key) const
{
	return MapFn::get(*this, key, TValue());
}

} // namespace Rio
