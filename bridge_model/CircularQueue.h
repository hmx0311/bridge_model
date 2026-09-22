#pragma once
#include <cassert>
#include <memory>
#include <cstring>

template<class T>
class CircularQueue
{
public:
	class ConstIterator
	{
	protected:
		const CircularQueue* m_queue;
		size_t m_idx;
	public:
		using value_type = T;
		using difference_type = ptrdiff_t;

		ConstIterator() = default;
		ConstIterator(const CircularQueue* queue, size_t idx) : m_queue(queue), m_idx(idx) {}

		const T& operator*() const
		{
			return (*m_queue)[m_idx];
		}

		const T* operator->() const
		{
			return &(*m_queue)[m_idx];
		}

		ConstIterator& operator++()
		{
			m_idx++;
			return *this;
		}

		ConstIterator operator++(int)
		{
			return ConstIterator(m_queue, m_idx++);
		}

		ConstIterator& operator--()
		{
			m_idx--;
			return *this;
		}

		ConstIterator operator--(int)
		{
			return ConstIterator(m_queue, m_idx--);
		}

		ConstIterator operator+(difference_type a) const
		{
			return ConstIterator(m_queue, m_idx + a);
		}

		ConstIterator& operator+=(difference_type a)
		{
			m_idx += a;
			return *this;
		}

		ConstIterator operator-(difference_type a) const
		{
			return ConstIterator(m_queue, m_idx - a);
		}

		ConstIterator& operator-=(difference_type a)
		{
			m_idx -= a;
			return *this;
		}

		bool operator<(const ConstIterator& it) const
		{
			assert(m_queue == it.m_queue);
			return m_idx < it.m_idx;
		}

		bool operator>(const ConstIterator& it) const
		{
			return it < *this;
		}

		bool operator<=(const ConstIterator& it) const
		{
			return !(*this > it);
		}

		bool operator>=(const ConstIterator& it) const
		{
			return !(*this < it);
		}

		bool operator==(const ConstIterator& it) const
		{
			assert(m_queue == it.m_queue);
			return m_idx == it.m_idx;
		}

		bool operator!=(const ConstIterator& it) const
		{
			return !(*this == it);
		}

		difference_type operator-(const ConstIterator& it) const
		{
			assert(m_queue == it.m_queue);
			return m_idx - it.m_idx;
		}
	};

	class Iterator : public ConstIterator
	{
	protected:
		using ConstIterator::m_queue;
		using ConstIterator::m_idx;

	public:
		using typename ConstIterator::value_type;
		using typename ConstIterator::difference_type;

		using ConstIterator::ConstIterator;

		T& operator*() const
		{
			return m_queue->m_data[(m_queue->m_begin + m_idx) & m_queue->m_idx_mask];
		}

		T* operator->() const
		{
			return m_queue->m_data + ((m_queue->m_begin + m_idx) & m_queue->m_idx_mask);
		}

		Iterator& operator++()
		{
			m_idx++;
			return *this;
		}

		Iterator operator++(int)
		{
			return Iterator(m_queue, m_idx++);
		}

		Iterator& operator--()
		{
			m_idx--;
			return *this;
		}

		Iterator operator--(int)
		{
			return Iterator(m_queue, m_idx--);
		}

		Iterator operator+(difference_type a) const
		{
			return Iterator(m_queue, m_idx + a);
		}

		Iterator& operator+=(difference_type a)
		{
			m_idx += a;
			return *this;
		}

		Iterator operator-(difference_type a) const
		{
			return Iterator(m_queue, m_idx - a);
		}

		Iterator& operator-=(difference_type a)
		{
			m_idx -= a;
			return *this;
		}

		difference_type operator-(const Iterator& it) const
		{
			assert(m_queue == it.m_queue);
			return m_idx - it.m_idx;
		}
	};

protected:
	T* m_data;
	int64_t m_idx_mask;
	size_t m_begin;
	size_t m_size;

public:
	CircularQueue() : m_data(nullptr), m_idx_mask(-1), m_begin(0), m_size(0) {}

	CircularQueue(uint32_t capacity_exp) : m_idx_mask((1LL << capacity_exp) - 1), m_begin(0), m_size(0)
	{
		assert(capacity_exp < 8 * sizeof(size_t) - 1);
		m_data = static_cast<T*>(malloc(capacity() * sizeof(T)));
	}

	CircularQueue(const CircularQueue& queue) : m_begin(0), m_size(queue.m_size)
	{
		if (m_size == 0)
		{
			m_idx_mask = -1;
			m_data = nullptr;
		}
		else
		{
			for (m_idx_mask = 0; capacity() < queue.m_size; m_idx_mask = (m_idx_mask << 1) + 1);
			m_data = static_cast<T*>(malloc(capacity() * sizeof(T)));
			for (size_t i = 0; i < m_size; i++)
			{
				new (m_data + i) T(queue[i]);
			}
		}
	}

	CircularQueue(CircularQueue&& queue) : m_data(queue.m_data), m_idx_mask(queue.m_idx_mask), m_begin(queue.m_begin), m_size(queue.m_size)
	{
		queue.m_data = nullptr;
		queue.m_idx_mask = -1;
		queue.m_begin = 0;
		queue.m_size = 0;
	}

	~CircularQueue()
	{
		destruct_all_elements();
		free(m_data);
	}

	CircularQueue& operator=(const CircularQueue& queue)
	{
		if (this == &queue)
		{
			return *this;
		}
		destruct_all_elements();
		m_begin = 0;
		m_size = queue.m_size;
		if (capacity() < m_size)
		{
			if (m_idx_mask == -1)
			{
				m_idx_mask = 0;
			}
			else
			{
				free(m_data);
			}
			while (capacity() < m_size)
			{
				m_idx_mask = (m_idx_mask << 1) + 1;
			}
			m_data = static_cast<T*>(malloc(capacity() * sizeof(T)));
		}
		for (size_t i = 0; i < m_size; i++)
		{
			new (m_data + i) T(queue[i]);
		}
		return *this;
	}

	CircularQueue& operator=(CircularQueue&& queue)
	{
		if (this == &queue)
		{
			return *this;
		}
		destruct_all_elements();
		jFree(m_data);
		m_data = queue.m_data;
		m_idx_mask = queue.m_idx_mask;
		m_begin = queue.m_begin;
		m_size = queue.m_size;
		queue.m_data = nullptr;
		queue.m_idx_mask = -1;
		queue.m_begin = 0;
		queue.m_size = 0;
		return *this;
	}

	void reserve(uint32_t capacity_exp)
	{
		assert(capacity_exp < 8 * sizeof(size_t) - 1);
		int64_t idx_mask = (1LL << capacity_exp) - 1;
		if (idx_mask <= m_idx_mask)
		{
			return;
		}
		reallocate(idx_mask + 1, m_size);
	}

	size_t capacity() const
	{
		return static_cast<size_t>(m_idx_mask + 1);
	}

	size_t size() const
	{
		return m_size;
	}

	bool empty() const
	{
		return m_size == 0;
	}

	template<class... Args>
	void emplace_front(Args&&... args)
	{
		if (m_size == capacity())
		{
			inc_capacity(0);
		}
		else
		{
			m_begin = (m_begin - 1) & m_idx_mask;
		}
		new (m_data + m_begin) T(std::forward<Args>(args)...);
		m_size++;
	}

	void push_front(const T& val)
	{
		emplace_front(val);
	}

	void push_front(T&& val)
	{
		emplace_front(std::move(val));
	}

	T& push_front()
	{
		emplace_front();
		return front();
	}

	template<class... Args>
	void emplace_back(Args&&... args)
	{
		if (m_size == capacity())
		{
			inc_capacity(m_size);
		}
		new (m_data + ((m_begin + m_size) & m_idx_mask)) T(std::forward<Args>(args)...);
		m_size++;
	}

	void push_back(const T& val)
	{
		emplace_back(val);
	}

	void push_back(T&& val)
	{
		emplace_back(std::move(val));
	}

	T& push_back()
	{
		emplace_back();
		return back();
	}

	template<class... Args>
	void emplace(size_t idx, Args&&... args)
	{
		assert(idx <= m_size);
		if (idx == 0)
		{
			emplace_front(std::forward<Args>(args)...);
			return;
		}
		if (idx == m_size)
		{
			emplace_back(std::forward<Args>(args)...);
			return;
		}
		size_t addr_idx;
		if (m_size == capacity())
		{
			inc_capacity(idx);
			addr_idx = idx;
		}
		else
		{
			if (2 * idx > m_size)
			{
				addr_idx = (m_begin + idx) & m_idx_mask;
				size_t addr_end = (m_begin + m_size - 1) & m_idx_mask;
				new (m_data + ((m_begin + m_size) & m_idx_mask)) T(std::move(m_data[addr_end]));
				move_backward(addr_idx, addr_end);
			}
			else
			{
				size_t old_begin = m_begin;
				m_begin = (m_begin - 1) & m_idx_mask;
				addr_idx = (m_begin + idx) & m_idx_mask;
				new (m_data + m_begin) T(std::move(m_data[old_begin]));
				move_forward(old_begin, addr_idx);
			}
			m_data[addr_idx].~T();
		}
		new (m_data + addr_idx) T(std::forward<Args>(args)...);
		m_size++;
	}

	void insert(size_t idx, const T& val)
	{
		emplace(idx, val);
	}

	void insert(size_t idx, T&& val)
	{
		emplace(idx, std::move(val));
	}

	T& front()
	{
		assert(m_size > 0);
		return m_data[m_begin];
	}

	const T& front() const
	{
		assert(m_size > 0);
		return m_data[m_begin];
	}

	T& back()
	{
		assert(m_size > 0);
		return m_data[(m_begin + m_size - 1) & m_idx_mask];
	}

	const T& back() const
	{
		assert(m_size > 0);
		return m_data[(m_begin + m_size - 1) & m_idx_mask];
	}

	T& operator[](size_t idx)
	{
		assert(idx < m_size);
		return m_data[(idx + m_begin) & m_idx_mask];
	}

	const T& operator[](size_t idx) const
	{
		assert(idx < m_size);
		return m_data[(idx + m_begin) & m_idx_mask];
	}

	void pop_front()
	{
		assert(m_size > 0);
		m_data[m_begin].~T();
		m_begin = (m_begin + 1) & m_idx_mask;
		m_size--;
	}

	void pop_back()
	{
		assert(m_size > 0);
		m_data[(m_begin + m_size - 1) & m_idx_mask].~T();
		m_size--;
	}

	void remove(size_t idx)
	{
		assert(idx < m_size);
		if (2 * idx > m_size)
		{
			move_forward((m_begin + idx) & m_idx_mask, (m_begin + m_size - 1) & m_idx_mask);
			m_data[(m_begin + m_size - 1) & m_idx_mask].~T();
		}
		else
		{
			move_backward(m_begin, (m_begin + idx) & m_idx_mask);
			m_data[m_begin].~T();
			m_begin = (m_begin + 1) & m_idx_mask;
		}
		m_size--;
	}

	void clear()
	{
		destruct_all_elements();
		m_begin = 0;
		m_size = 0;
	}

	Iterator begin()
	{
		return Iterator(this, 0);
	}

	ConstIterator begin() const
	{
		return ConstIterator(this, 0);
	}

	ConstIterator cbegin() const
	{
		return begin();
	}

	Iterator end()
	{
		return Iterator(this, m_size);
	}

	ConstIterator end() const
	{
		return ConstIterator(this, m_size);
	}

	ConstIterator cend() const
	{
		return end();
	}

	T* get()
	{
		if (m_begin + m_size <= capacity())
		{
			return m_data + m_begin;
		}
		if (2 * m_size > static_cast<size_t>(m_idx_mask))
		{
			inc_capacity(m_size);
			return m_data;
		}
		for (size_t i = (m_begin + m_size) & m_idx_mask; i < m_size; i++)
		{
			new (m_data + i) T(std::move(m_data[(m_begin + i) & m_idx_mask]));
		}
		for (size_t i = (m_begin + m_size) & m_idx_mask; i > 0; )
		{
			i--;
			m_data[i] = std::move(m_data[(m_begin + i) & m_idx_mask]);
		}
		for (size_t i = m_begin; i < capacity(); i++)
		{
			m_data[i].~T();
		}
		m_begin = 0;
		return m_data;
	}

protected:
	void reallocate(size_t count, size_t insert_idx)
	{
		T* temp = static_cast<T*>(malloc(count * sizeof(T)));
		for (size_t i = 0; i < insert_idx; i++)
		{
			T& old = m_data[(m_begin + i) & m_idx_mask];
			new (temp + i) T(std::move(old));
			old.~T();
		}
		for (size_t i = insert_idx; i < m_size;)
		{
			T& old = m_data[(m_begin + i) & m_idx_mask];
			i++;
			new (temp + i) T(std::move(old));
			old.~T();
		}
		free(m_data);
		m_data = temp;
		m_idx_mask = count - 1;
		m_begin = 0;
	}

	void inc_capacity(size_t insert_idx)
	{
		if (m_data == nullptr)
		{
			m_data = static_cast<T*>(malloc(sizeof(T)));
			m_idx_mask = 0;
		}
		else
		{
			reallocate(capacity() << 1, insert_idx);
		}
	}

	void move_forward(size_t addr_begin, size_t addr_end)
	{
		if (addr_begin <= addr_end)
		{
			for (size_t i = addr_begin; i < addr_end; i++)
			{
				m_data[i] = std::move(m_data[i + 1]);
			}
		}
		else
		{
			for (size_t i = addr_begin; i < static_cast<size_t>(m_idx_mask); i++)
			{
				m_data[i] = std::move(m_data[i + 1]);
			}
			m_data[m_idx_mask] = std::move(m_data[0]);
			for (size_t i = 0; i < addr_end; i++)
			{
				m_data[i] = std::move(m_data[i + 1]);
			}
		}

	}

	void move_backward(size_t addr_begin, size_t addr_end)
	{
		if (addr_begin <= addr_end)
		{
			for (size_t i = addr_end; i > addr_begin; i--)
			{
				m_data[i] = std::move(m_data[i - 1]);
			}
		}
		else
		{
			for (size_t i = addr_end; i > 0; i--)
			{
				m_data[i] = std::move(m_data[i - 1]);
			}
			m_data[0] = std::move(m_data[m_idx_mask]);
			for (size_t i = static_cast<size_t>(m_idx_mask); i > addr_begin; i--)
			{
				m_data[i] = std::move(m_data[i - 1]);
			}
		}
	}

	void destruct_all_elements()
	{
		if (m_size == 0)
		{
			return;
		}
		T* p = m_data + m_begin;
		T* end;
		if (m_begin + m_size > capacity())
		{
			end = m_data + capacity();
			while (p != end)
			{
				p->~T();
				p++;
			}
			p = m_data;
			end = m_data + ((m_begin + m_size) & m_idx_mask);
		}
		else
		{
			end = m_data + (m_begin + m_size);
		}
		while (p != end)
		{
			p->~T();
			p++;
		}
	}
};

template<class... Args>
class CircularQueueSOA
{
private:
	template <size_t ElementIndex, class T, class... Rest>
	struct Element
	{
		static_assert(ElementIndex < sizeof...(Args), "提供的元素索引超出范围");
		using Type = typename Element<ElementIndex - 1, Rest...>::Type;
	};

	template<class T, class... Rest>
	struct Element<0, T, Rest...>
	{
		using Type = T;
	};

	template <size_t ElementIndex>
	using ElementType = typename Element<ElementIndex, Args...>::Type;

	template<size_t...>
	struct NumPack
	{
		static constexpr size_t Total = 0;
	};

	template<size_t Cur, size_t... Rest>
	struct NumPack<Cur, Rest...>
	{
		static constexpr size_t Total = Cur + NumPack<Rest...>::Total;
	};

	template<class...>
	struct ArgPack {};

	template<class...>
	class Allocator
	{
	public:
		template<class...>
		class Emplacer
		{
		public:
			static void Emplace(CircularQueueSOA* __restrict queue, size_t addr_idx) {}
		};
		static void CalcOffsets(size_t count, size_t* __restrict offsets) {}
		static void Reallocate(CircularQueueSOA* queue, void** dst, size_t insert_idx) {}
		static void DestructAllElements(CircularQueueSOA* __restrict queue) {}
		static void CopyQueue(CircularQueueSOA* __restrict dst_queue, const CircularQueueSOA* __restrict src_queue) {}
		template<class...>
		static void ConstructElement(CircularQueueSOA* __restrict queue, size_t addr_idx) {}
		template<class...>
		static void CopyElement(CircularQueueSOA* __restrict queue, size_t addr_idx) {}
		static void MoveConstructElement(CircularQueueSOA* __restrict queue, size_t dst_addr_idx, size_t src_addr_idx) {}
		static void MoveCopyElement(CircularQueueSOA* __restrict queue, size_t dst_addr_idx, size_t src_addr_idx) {}
		static void DestructElement(CircularQueueSOA* __restrict queue, size_t addr_idx) {}
		static void MoveForward(CircularQueueSOA* __restrict queue, size_t addr_begin, size_t addr_end) {}
		static void MoveBackward(CircularQueueSOA* __restrict queue, size_t addr_begin, size_t addr_end) {}
	};

	template<class T, class... Rest>
	class Allocator<T, Rest...>
	{
	public:
		static constexpr auto ElementIdx = sizeof...(Args) - sizeof...(Rest) - 1;

		template<class...>
		class Emplacer;

		template<size_t NumRemainingArgs, size_t... NumRestArgs, class... CurArgs, class NextArg, class... RestArgs>
		class Emplacer<typename std::enable_if<(NumRemainingArgs > 0), void>::type, NumPack<NumRemainingArgs, NumRestArgs...>, ArgPack<CurArgs...>, NextArg, RestArgs...>
			: public Emplacer<void, NumPack<NumRemainingArgs - 1, NumRestArgs...>, ArgPack<CurArgs..., NextArg>, RestArgs...>
		{
		};

		template<size_t... NumRestArgs, class... CurArgs, class... RestArgs>
		class Emplacer<void, NumPack<0, NumRestArgs...>, ArgPack<CurArgs...>, RestArgs...>
		{
		public:
			static void Emplace(CircularQueueSOA* __restrict queue, size_t addr_idx, CurArgs&&... cur, RestArgs&&... rest)
			{
				new (static_cast<T*>(queue->m_data[ElementIdx]) + addr_idx) T(std::forward<CurArgs>(cur)...);
				Allocator<Rest...>::template Emplacer<void, NumPack<NumRestArgs...>, ArgPack<>, RestArgs...>::Emplace(queue, addr_idx, std::forward<RestArgs>(rest)...);
			}
		};

		static void CalcOffsets(size_t count, size_t* __restrict offsets)
		{
			static_assert(alignof(T) <= 16, "暂不支持超过16字节的对齐");
			offsets[0] = (offsets[0] + alignof(T) - 1) & ~(alignof(T) - 1);
			offsets[1] = offsets[0] + count * sizeof(T);
			Allocator<Rest...>::CalcOffsets(count, offsets + 1);
		}

		static void Reallocate(CircularQueueSOA* __restrict queue, void** __restrict dst, size_t insert_idx)
		{
			T* temp = static_cast<T*>(dst[ElementIdx]);
			T* data = static_cast<T*>(queue->m_data[ElementIdx]);
			for (size_t i = 0; i < insert_idx; i++)
			{
				T& old = data[(queue->m_begin + i) & queue->m_idx_mask];
				new (temp + i) T(std::move(old));
				old.~T();
			}
			for (size_t i = insert_idx; i < queue->m_size;)
			{
				T& old = data[(queue->m_begin + i) & queue->m_idx_mask];
				i++;
				new (temp + i) T(std::move(old));
				old.~T();
			}
			Allocator<Rest...>::Reallocate(queue, dst, insert_idx);
		}

		static void DestructAllElements(CircularQueueSOA* __restrict queue)
		{
			T* data = static_cast<T*>(queue->m_data[ElementIdx]);
			T* p = data + queue->m_begin;
			T* end;
			if (queue->m_begin + queue->m_size > queue->capacity())
			{
				end = data + queue->capacity();
				while (p != end)
				{
					p->~T();
					p++;
				}
				p = data;
				end = data + ((queue->m_begin + queue->m_size) & queue->m_idx_mask);
			}
			else
			{
				end = data + (queue->m_begin + queue->m_size);
			}
			while (p != end)
			{
				p->~T();
				p++;
			}
			Allocator<Rest...>::DestructAllElements(queue);
		}

		static void CopyQueue(CircularQueueSOA* __restrict dst_queue, const CircularQueueSOA* __restrict src_queue)
		{
			T* dst = static_cast<T*>(dst_queue->m_data[ElementIdx]);
			T* src = static_cast<T*>(src_queue->m_data[ElementIdx]);
			for (size_t i = 0; i < src_queue->m_size; i++)
			{
				new (dst + i) T(src[(src_queue->m_begin + i) & src_queue->m_idx_mask]);
			}
			Allocator<Rest...>::CopyQueue(dst_queue, src_queue);
		}

		template<class RefT, class... RefRest>
		static void ConstructElement(CircularQueueSOA* __restrict queue, size_t addr_idx, RefT&& value, RefRest&&... rest_values)
		{
			new (static_cast<T*>(queue->m_data[ElementIdx]) + addr_idx) T(std::forward<RefT>(value));
			Allocator<Rest...>::ConstructElement(queue, addr_idx, std::forward<RefRest>(rest_values)...);
		}

		template<class RefT, class... RefRest>
		static void CopyElement(CircularQueueSOA* __restrict queue, size_t addr_idx, RefT&& value, RefRest&&... rest_values)
		{
			static_cast<T*>(queue->m_data[ElementIdx])[addr_idx] = std::forward<RefT>(value);
			Allocator<Rest...>::CopyElement(queue, addr_idx, std::forward<RefRest>(rest_values)...);
		}

		static void MoveConstructElement(CircularQueueSOA* __restrict queue, size_t dst_addr_idx, size_t src_addr_idx)
		{
			new (static_cast<T*>(queue->m_data[ElementIdx]) + dst_addr_idx) T(std::move(static_cast<T*>(queue->m_data[ElementIdx])[src_addr_idx]));
			Allocator<Rest...>::MoveConstructElement(queue, dst_addr_idx, src_addr_idx);
		}

		static void MoveCopyElement(CircularQueueSOA* __restrict queue, size_t dst_addr_idx, size_t src_addr_idx)
		{
			static_cast<T*>(queue->m_data[ElementIdx])[dst_addr_idx] = std::move(static_cast<T*>(queue->m_data[ElementIdx])[src_addr_idx]);
			Allocator<Rest...>::MoveCopyElement(queue, dst_addr_idx, src_addr_idx);
		}

		static void DestructElement(CircularQueueSOA* __restrict queue, size_t addr_idx)
		{
			static_cast<T*>(queue->m_data[ElementIdx])[addr_idx].~T();
			Allocator<Rest...>::DestructElement(queue, addr_idx);
		}

		static void MoveForward(CircularQueueSOA* __restrict queue, size_t addr_begin, size_t addr_end)
		{
			T* data = static_cast<T*>(queue->m_data[ElementIdx]);
			if (addr_begin <= addr_end)
			{
				for (size_t i = addr_begin; i < addr_end; i++)
				{
					data[i] = std::move(data[i + 1]);
				}
			}
			else
			{
				for (size_t i = addr_begin; i < static_cast<size_t>(queue->m_idx_mask); i++)
				{
					data[i] = std::move(data[i + 1]);
				}
				data[queue->m_idx_mask] = std::move(data[0]);
				for (size_t i = 0; i < addr_end; i++)
				{
					data[i] = std::move(data[i + 1]);
				}
			}
			Allocator<Rest...>::MoveForward(queue, addr_begin, addr_end);
		}

		static void MoveBackward(CircularQueueSOA* __restrict queue, size_t addr_begin, size_t addr_end)
		{
			T* data = static_cast<T*>(queue->m_data[ElementIdx]);
			if (addr_begin <= addr_end)
			{
				for (size_t i = addr_end; i > addr_begin; i--)
				{
					data[i] = std::move(data[i - 1]);
				}
			}
			else
			{
				for (size_t i = addr_end; i > 0; i--)
				{
					data[i] = std::move(data[i - 1]);
				}
				data[0] = std::move(data[queue->m_idx_mask]);
				for (size_t i = static_cast<size_t>(queue->m_idx_mask); i > addr_begin; i--)
				{
					data[i] = std::move(data[i - 1]);
				}
			}
			Allocator<Rest...>::MoveBackward(queue, addr_begin, addr_end);
		}
	};

	class IteratorBase
	{
	protected:
		const CircularQueueSOA* m_queue;
		size_t m_idx;

	public:
		using difference_type = ptrdiff_t;

		IteratorBase() = default;
		IteratorBase(const CircularQueueSOA* queue, size_t idx) : m_queue(queue), m_idx(idx) {}

		bool operator<(const IteratorBase& it) const
		{
			assert(m_queue == it.m_queue);
			return m_idx < it.m_idx;
		}

		bool operator>(const IteratorBase& it) const
		{
			return it < *this;
		}

		bool operator<=(const IteratorBase& it) const
		{
			return !(*this > it);
		}

		bool operator>=(const IteratorBase& it) const
		{
			return !(*this < it);
		}

		bool operator==(const IteratorBase& it) const
		{
			assert(m_queue == it.m_queue);
			return m_idx == it.m_idx;
		}

		bool operator!=(const IteratorBase& it) const
		{
			return !(*this == it);
		}

		difference_type operator-(const IteratorBase& it) const
		{
			assert(m_queue == it.m_queue);
			return m_idx - it.m_idx;
		}
	};

public:
	template<size_t ElementIndex>
	class ConstElementIterator : public IteratorBase
	{
	private:
		using IteratorBase::m_queue;
		using IteratorBase::m_idx;
	public:
		using value_type = ElementType<ElementIndex>;
		using typename IteratorBase::difference_type;

		using IteratorBase::IteratorBase;

		const value_type& operator*() const
		{
			return static_cast<value_type*>(m_queue->m_data[ElementIndex])[(m_idx + m_queue->m_begin) & m_queue->m_idx_mask];
		}

		const value_type* operator->() const
		{
			return static_cast<value_type*>(m_queue->m_data[ElementIndex]) + ((m_idx + m_queue->m_begin) & m_queue->m_idx_mask);
		}

		ConstElementIterator& operator++()
		{
			m_idx++;
			return *this;
		}

		ConstElementIterator operator++(int)
		{
			return ConstElementIterator(m_queue, m_idx++);
		}

		ConstElementIterator& operator--()
		{
			m_idx--;
			return *this;
		}

		ConstElementIterator operator--(int)
		{
			return ConstElementIterator(m_queue, m_idx--);
		}

		ConstElementIterator& operator+=(difference_type a)
		{
			m_idx += a;
			return *this;
		}

		ConstElementIterator& operator-=(difference_type a)
		{
			m_idx -= a;
			return *this;
		}
	};

	template<size_t ElementIndex>
	class ElementIterator : public IteratorBase
	{
	private:
		using IteratorBase::m_queue;
		using IteratorBase::m_idx;
	public:
		using value_type = ElementType<ElementIndex>;
		using typename IteratorBase::difference_type;

		using IteratorBase::IteratorBase;

		value_type& operator*() const
		{
			return static_cast<value_type*>(m_queue->m_data[ElementIndex])[(m_idx + m_queue->m_begin) & m_queue->m_idx_mask];
		}

		value_type* operator->() const
		{
			return static_cast<value_type*>(m_queue->m_data[ElementIndex]) + ((m_idx + m_queue->m_begin) & m_queue->m_idx_mask);
		}

		ElementIterator& operator++()
		{
			m_idx++;
			return *this;
		}

		ElementIterator operator++(int)
		{
			return ElementIterator(m_queue, m_idx++);
		}

		ElementIterator& operator--()
		{
			m_idx--;
			return *this;
		}

		ElementIterator operator--(int)
		{
			return ElementIterator(m_queue, m_idx--);
		}

		ElementIterator& operator+=(difference_type a)
		{
			m_idx += a;
			return *this;
		}

		ElementIterator& operator-=(difference_type a)
		{
			m_idx -= a;
			return *this;
		}
	};

	class ConstIterator : public IteratorBase
	{
	private:
		using IteratorBase::m_queue;
		using IteratorBase::m_idx;
	public:
		using value_type = ConstIterator;
		using typename IteratorBase::difference_type;

		using IteratorBase::IteratorBase;

		const ConstIterator operator*() const
		{
			return *this;
		}

		template<size_t ElementIndex>
		const ElementType<ElementIndex>& get() const
		{
			return static_cast<ElementType<ElementIndex>*>(m_queue->m_data[ElementIndex])[(m_idx + m_queue->m_begin) & m_queue->m_idx_mask];
		}

		ConstIterator& operator++()
		{
			m_idx++;
			return *this;
		}

		ConstIterator operator++(int)
		{
			return ConstIterator(m_queue, m_idx++);
		}

		ConstIterator& operator--() {
			m_idx--;
			return *this;
		}

		ConstIterator operator--(int)
		{
			return ConstIterator(m_queue, m_idx--);
		}

		ConstIterator& operator+=(difference_type a)
		{
			m_idx += a;
			return *this;
		}

		ConstIterator& operator-=(difference_type a)
		{
			m_idx -= a;
			return *this;
		}
	};

	class Iterator : public IteratorBase
	{
	private:
		using IteratorBase::m_queue;
		using IteratorBase::m_idx;
	public:
		using value_type = Iterator;
		using typename IteratorBase::difference_type;

		using IteratorBase::IteratorBase;

		const Iterator operator*() const
		{
			return *this;
		}

		template<size_t ElementIndex>
		ElementType<ElementIndex>& get() const
		{
			return static_cast<ElementType<ElementIndex>*>(m_queue->m_data[ElementIndex])[(m_idx + m_queue->m_begin) & m_queue->m_idx_mask];
		}

		Iterator& operator++()
		{
			m_idx++;
			return *this;
		}

		Iterator operator++(int)
		{
			return Iterator(m_queue, m_idx++);
		}

		Iterator& operator--()
		{
			m_idx--;
			return *this;
		}

		Iterator operator--(int) { return Iterator(m_queue, m_idx--); }

		Iterator& operator+=(difference_type a)
		{
			m_idx += a;
			return *this;
		}

		Iterator& operator-=(difference_type a)
		{
			m_idx -= a;
			return *this;
		}
	};



	template<size_t ElementIndex>
	class ConstSubQueue
	{
	protected:
		const CircularQueueSOA* m_queue;
	public:
		using value_type = ElementType<ElementIndex>;

		ConstSubQueue(const CircularQueueSOA* queue) : m_queue(queue) {}

		size_t size() const
		{
			return m_queue->m_size;
		}

		const value_type& operator[](size_t idx) const
		{
			return m_queue->get<ElementIndex>(idx);
		}

		const value_type& front() const
		{
			return m_queue->front<ElementIndex>();
		}

		const value_type& back() const
		{
			return m_queue->back<ElementIndex>();
		}

		ConstElementIterator<ElementIndex> begin() const
		{
			return m_queue->begin<ElementIndex>();
		}

		ConstElementIterator<ElementIndex> cbegin() const
		{
			return m_queue->cbegin<ElementIndex>();
		}

		ConstElementIterator<ElementIndex> end() const
		{
			return m_queue->end<ElementIndex>();
		}

		ConstElementIterator<ElementIndex> cend() const
		{
			return m_queue->cend<ElementIndex>();
		}
	};

	template<size_t ElementIndex>
	class SubQueue : public ConstSubQueue<ElementIndex>
	{
	private:
		using ConstSubQueue<ElementIndex>::m_queue;
	public:
		using typename ConstSubQueue<ElementIndex>::value_type;

		SubQueue(CircularQueueSOA* queue) : ConstSubQueue<ElementIndex>(queue) {}

		value_type& operator[](size_t idx)
		{
			return const_cast<CircularQueueSOA*>(m_queue)->get<ElementIndex>(idx);
		}

		value_type& front()
		{
			return const_cast<CircularQueueSOA*>(m_queue)->front<ElementIndex>();
		}

		value_type& back()
		{
			return const_cast<CircularQueueSOA*>(m_queue)->back<ElementIndex>();
		}

		ElementIterator<ElementIndex> begin()
		{
			return const_cast<CircularQueueSOA*>(m_queue)->begin<ElementIndex>();
		}

		ElementIterator<ElementIndex> end()
		{
			return const_cast<CircularQueueSOA*>(m_queue)->end<ElementIndex>();
		}
	};

protected:
	void* m_data[sizeof...(Args)];
	int64_t m_idx_mask;
	size_t m_begin;
	size_t m_size;

public:
	CircularQueueSOA() : m_data{}, m_idx_mask(-1), m_begin(0), m_size(0) {}

	CircularQueueSOA(uint32_t capacity_exp) : m_idx_mask((1LL << capacity_exp) - 1), m_begin(0), m_size(0)
	{
		assert(capacity_exp < 8 * sizeof(size_t) - 1);
		allocate();
	}

	CircularQueueSOA(const CircularQueueSOA& queue) : m_begin(0), m_size(queue.m_size)
	{
		if (m_size == 0)
		{
			m_idx_mask = -1;
			for (void*& data : m_data)
			{
				data = nullptr;
			}
		}
		else
		{
			for (m_idx_mask = 0; capacity() < queue.m_size; m_idx_mask = (m_idx_mask << 1) + 1);
			allocate();
			Allocator<Args...>::CopyQueue(this, &queue);
		}
	}

	CircularQueueSOA(CircularQueueSOA&& queue) : m_idx_mask(queue.m_idx_mask), m_begin(queue.m_begin), m_size(queue.m_size)
	{
		memcpy(m_data, queue.m_data, sizeof(m_data));
		queue.m_idx_mask = -1;
		queue.m_begin = 0;
		queue.m_size = 0;
		for (void*& data : queue.m_data)
		{
			data = nullptr;
		}
	}

	~CircularQueueSOA()
	{
		if (m_data[0] != nullptr)
		{
			Allocator<Args...>::DestructAllElements(this);
			free(m_data[0]);
		}
	}

	CircularQueueSOA& operator=(const CircularQueueSOA& queue)
	{
		if (this == &queue)
		{
			return *this;
		}
		Allocator<Args...>::DestructAllElements(this);
		m_begin = 0;
		m_size = queue.m_size;
		if (capacity() < queue.m_size)
		{
			if (m_idx_mask == -1)
			{
				m_idx_mask = 0;
			}
			else
			{
				free(m_data[0]);
			}
			while (capacity() < queue.m_size)
			{
				m_idx_mask = (m_idx_mask << 1) + 1;
			}
			allocate();
		}
		Allocator<Args...>::CopyQueue(this, &queue);
		return *this;
	}

	CircularQueueSOA& operator=(CircularQueueSOA&& queue)
	{
		if (this == &queue)
		{
			return *this;
		}
		if (m_data[0] != nullptr)
		{
			Allocator<Args...>::DestructAllElements(this);
			free(m_data[0]);
		}
		memcpy(this, &queue, sizeof(CircularQueueSOA));
		queue.m_idx_mask = -1;
		queue.m_begin = 0;
		queue.m_size = 0;
		for (void*& data : queue.m_data)
		{
			data = nullptr;
		}
		return *this;
	}

	void reserve(uint32_t capacity_exp)
	{
		assert(capacity_exp < 8 * sizeof(size_t) - 1);
		int64_t idx_mask = (1LL << capacity_exp) - 1;
		if (idx_mask <= m_idx_mask)
		{
			return;
		}
		reallocate(idx_mask + 1, m_size);
	}

	size_t capacity() const
	{
		return static_cast<size_t>(m_idx_mask + 1);
	}

	size_t size() const
	{
		return m_size;
	}

	bool empty() const
	{
		return m_size == 0;
	}

	template<size_t... NumArgs, class... EmplaceArgs>
	void emplace_front(EmplaceArgs&&... emplace_args)
	{
		static_assert(sizeof...(NumArgs) == sizeof...(Args), "提供的构造参数个数的数量与要构造的元素数量不符");
		static_assert(NumPack<NumArgs...>::Total == sizeof...(EmplaceArgs), "指定的构造参数个数和与提供的构造参数数量不符");
		if (m_size == capacity())
		{
			inc_capacity(0);
		}
		else
		{
			m_begin = (m_begin - 1) & m_idx_mask;
		}
		Allocator<Args...>::template Emplacer<void, NumPack<NumArgs...>, ArgPack<>, EmplaceArgs...>::Emplace(this, m_begin, std::forward<EmplaceArgs>(emplace_args)...);
		m_size++;
	}

	template<class... RefArgs>
	void push_front(RefArgs&&... args)
	{
		emplace_front<(sizeof(Args), 1)...>(std::forward<RefArgs>(args)...);
	}

	void push_front()
	{
		emplace_front<(sizeof(Args), 0)...>();
	}

	template<size_t... NumArgs, class... EmplaceArgs>
	void emplace_back(EmplaceArgs&&... emplace_args)
	{
		static_assert(sizeof...(NumArgs) == sizeof...(Args), "提供的构造参数个数的数量与要构造的元素数量不符");
		static_assert(NumPack<NumArgs...>::Total == sizeof...(EmplaceArgs), "指定的构造参数个数和与提供的构造参数数量不符");
		if (m_size == capacity())
		{
			inc_capacity(m_size);
		}
		Allocator<Args...>::template Emplacer<void, NumPack<NumArgs...>, ArgPack<>, EmplaceArgs...>::Emplace(this, (m_begin + m_size) & m_idx_mask, std::forward<EmplaceArgs>(emplace_args)...);
		m_size++;
	}

	template<class... RefArgs>
	void push_back(RefArgs&&... args)
	{
		emplace_back<(sizeof(Args), 1)...>(std::forward<RefArgs>(args)...);
	}

	void push_back()
	{
		emplace_back<(sizeof(Args), 0)...>();
	}

	template<size_t... NumArgs, class... EmplaceArgs>
	void emplace(size_t idx, EmplaceArgs&&... emplace_args)
	{
		static_assert(sizeof...(NumArgs) == sizeof...(Args), "提供的构造参数个数的数量与要构造的元素数量不符");
		static_assert(NumPack<NumArgs...>::Total == sizeof...(EmplaceArgs), "指定的构造参数个数和与提供的构造参数数量不符");
		assert(idx <= m_size);
		if (idx == 0)
		{
			emplace_front<NumArgs...>(std::forward<EmplaceArgs>(emplace_args)...);
			return;
		}
		if (idx == m_size)
		{
			emplace_back<NumArgs...>(std::forward<EmplaceArgs>(emplace_args)...);
			return;
		}
		size_t addr_idx;
		if (m_size == capacity())
		{
			inc_capacity(idx);
			addr_idx = idx;
		}
		else
		{
			if (2 * idx > m_size)
			{
				addr_idx = (m_begin + idx) & m_idx_mask;
				size_t addr_end = (m_begin + m_size - 1) & m_idx_mask;
				Allocator<Args...>::MoveConstructElement(this, (m_begin + m_size) & m_idx_mask, addr_end);
				Allocator<Args...>::MoveBackward(this, addr_idx, addr_end);
			}
			else
			{
				size_t old_begin = m_begin;
				m_begin = (m_begin - 1) & m_idx_mask;
				addr_idx = (m_begin + idx) & m_idx_mask;
				Allocator<Args...>::MoveConstructElement(this, m_begin, old_begin);
				Allocator<Args...>::MoveForward(this, old_begin, addr_idx);
			}
			Allocator<Args...>::DestructElement(this, addr_idx);
		}
		Allocator<Args...>::template Emplacer<void, NumPack<NumArgs...>, ArgPack<>, EmplaceArgs...>::Emplace(this, addr_idx, std::forward<EmplaceArgs>(emplace_args)...);
		m_size++;
	}

	template<class... RefArgs>
	void insert(size_t idx, RefArgs&&... args)
	{
		emplace<(sizeof(Args), 1)...>(idx, std::forward<RefArgs>(args)...);
	}

	template<size_t ElementIndex>
	ElementType<ElementIndex>& front()
	{
		assert(m_size > 0);
		return static_cast<ElementType<ElementIndex>*>(m_data[ElementIndex])[m_begin];
	}

	template<size_t ElementIndex>
	const ElementType<ElementIndex>& front() const
	{
		assert(m_size > 0);
		return static_cast<ElementType<ElementIndex>*>(m_data[ElementIndex])[m_begin];
	}

	template<size_t ElementIndex>
	ElementType<ElementIndex>& back()
	{
		assert(m_size > 0);
		return static_cast<ElementType<ElementIndex>*>(m_data[ElementIndex])[(m_begin + m_size - 1) & m_idx_mask];
	}

	template<size_t ElementIndex>
	const ElementType<ElementIndex>& back() const
	{
		assert(m_size > 0);
		return static_cast<ElementType<ElementIndex>*>(m_data[ElementIndex])[(m_begin + m_size - 1) & m_idx_mask];
	}

	template<size_t ElementIndex>
	ElementType<ElementIndex>& get(size_t idx)
	{
		assert(idx < m_size);
		return static_cast<ElementType<ElementIndex>*>(m_data[ElementIndex])[(m_begin + idx) & m_idx_mask];
	}

	template<size_t ElementIndex>
	const ElementType<ElementIndex>& get(size_t idx) const
	{
		assert(idx < m_size);
		return static_cast<ElementType<ElementIndex>*>(m_data[ElementIndex])[(m_begin + idx) & m_idx_mask];
	}

	void pop_front()
	{
		assert(m_size > 0);
		Allocator<Args...>::DestructElement(this, m_begin);
		m_begin = (m_begin + 1) & m_idx_mask;
		m_size--;
	}

	void pop_back()
	{
		assert(m_size > 0);
		Allocator<Args...>::DestructElement(this, (m_begin + m_size - 1) & m_idx_mask);
		m_size--;
	}

	void remove(size_t idx)
	{
		assert(idx < m_size);
		if (2 * idx > m_size)
		{
			Allocator<Args...>::MoveForward(this, (m_begin + idx) & m_idx_mask, (m_begin + m_size - 1) & m_idx_mask);
			Allocator<Args...>::DestructElement(this, (m_begin + m_size - 1) & m_idx_mask);
		}
		else
		{
			Allocator<Args...>::MoveBackward(this, m_begin, (m_begin + idx) & m_idx_mask);
			Allocator<Args...>::DestructElement(this, m_begin);
			m_begin = (m_begin + 1) & m_idx_mask;
		}
		m_size--;
	}

	void clear()
	{
		Allocator<Args...>::DestructAllElements(this);
		m_begin = 0;
		m_size = 0;
	}

	template<size_t ElementIndex>
	ElementIterator<ElementIndex> begin()
	{
		return ElementIterator<ElementIndex>(this, 0);
	}

	template<size_t ElementIndex>
	ConstElementIterator<ElementIndex> begin() const
	{
		return ConstElementIterator<ElementIndex>(this, 0);
	}

	template<size_t ElementIndex>
	ConstElementIterator<ElementIndex> cbegin() const
	{
		return begin<ElementIndex>();
	}

	Iterator begin()
	{
		return Iterator(this, 0);
	}

	ConstIterator begin() const
	{
		return ConstIterator(this, 0);
	}

	ConstIterator cbegin() const
	{
		return begin();
	}

	template<size_t ElementIndex>
	ElementIterator<ElementIndex> end()
	{
		return ElementIterator<ElementIndex>(this, m_size);
	}

	template<size_t ElementIndex>
	ConstElementIterator<ElementIndex> end() const
	{
		return ConstElementIterator<ElementIndex>(this, m_size);
	}

	template<size_t ElementIndex>
	ConstElementIterator<ElementIndex> cend() const
	{
		return end<ElementIndex>();
	}

	Iterator end()
	{
		return Iterator(this, m_size);
	}

	ConstIterator end() const
	{
		return ConstIterator(this, m_size);
	}

	ConstIterator cend() const
	{
		return end();
	}

	template<size_t ElementIndex>
	SubQueue<ElementIndex> sub_queue()
	{
		return SubQueue<ElementIndex>(this);
	}

	template<size_t ElementIndex>
	ConstSubQueue<ElementIndex> sub_queue() const
	{
		return ConstSubQueue<ElementIndex>(this);
	}

	template<size_t ElementIndex>
	ElementType<ElementIndex>* get()
	{
		if (m_begin + m_size <= capacity())
		{
			return static_cast<ElementType<ElementIndex>*>(m_data[ElementIndex]) + m_begin;
		}
		if (2 * m_size > static_cast<size_t>(m_idx_mask))
		{
			inc_capacity(m_size);
			return static_cast<ElementType<ElementIndex>*>(m_data[ElementIndex]);
		}
		for (size_t i = (m_begin + m_size) & m_idx_mask; i < m_size; i++)
		{
			Allocator<Args...>::MoveConstructElement(this, i, (m_begin + i) & m_idx_mask);
		}
		for (size_t i = (m_begin + m_size) & m_idx_mask; i > 0; )
		{
			i--;
			Allocator<Args...>::MoveCopyElement(this, i, (m_begin + i) & m_idx_mask);
		}
		for (size_t i = m_begin; i < capacity(); i++)
		{
			Allocator<Args...>::DestructElement(this, i);
		}
		m_begin = 0;
		return static_cast<ElementType<ElementIndex>*>(m_data[ElementIndex]);
	}

protected:
	void allocate()
	{
		size_t offsets[sizeof...(Args) + 1];
		offsets[0] = 0;
		Allocator<Args...>::CalcOffsets(capacity(), offsets);
		m_data[0] = malloc(offsets[sizeof...(Args)]);
		ASSERT_ALIGNED16(m_data[0]);
		for (size_t i = 1; i < sizeof...(Args); i++)
		{
			m_data[i] = static_cast<char*>(m_data[0]) + offsets[i];
		}
	}

	void reallocate(size_t count, size_t insert_idx)
	{
		size_t offsets[sizeof...(Args) + 1];
		offsets[0] = 0;
		Allocator<Args...>::CalcOffsets(count, offsets);
		void* dst[sizeof...(Args)];
		dst[0] = malloc(offsets[sizeof...(Args)]);
		for (size_t i = 1; i < sizeof...(Args); i++)
		{
			dst[i] = static_cast<char*>(dst[0]) + offsets[i];
		}
		Allocator<Args...>::Reallocate(this, dst, insert_idx);
		free(m_data[0]);
		memcpy(m_data, dst, sizeof(dst));
		m_idx_mask = count - 1;
		m_begin = 0;
	}

	void inc_capacity(size_t insert_idx)
	{
		if (m_idx_mask == -1)
		{
			m_idx_mask = 0;
			allocate();
		}
		else
		{
			reallocate(capacity() << 1, insert_idx);
		}
	}
};
