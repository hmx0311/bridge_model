#pragma once
#include <stdint.h>
#include <utility>

struct QuadTreeIdx
{
	uint32_t level;
	size_t x;
	size_t y;
};

template<class T, uint32_t NumLevels, size_t RootSizeX, size_t RootSizeY>
class StaticQuadTree
{
private:
	class ConstLevelProxy
	{
	protected:
		const uint32_t m_level;
		const T* m_level_data;

	public:
		ConstLevelProxy(uint32_t level, const T* data) :m_level(level), m_level_data(data + LevelOffset(level)) {}

		const T* operator[](size_t idx) const
		{
			return m_level_data + idx * LevelSizeY(m_level);
		}
	};

	class LevelProxy :public ConstLevelProxy
	{
	private:
		using ConstLevelProxy::m_level;
		using ConstLevelProxy::m_level_data;
	public:
		LevelProxy(uint32_t level, T* data) : ConstLevelProxy(level, data) {}

		T* operator[](size_t idx)
		{
			return const_cast<T*>(ConstLevelProxy::operator[](idx));
		}
	};

public:
	static constexpr size_t LevelOffset(uint32_t level = NumLevels)
	{
		return RootSizeX * RootSizeY * (((1ULL << (2 * level)) - 1) & 0x5555555555555555ULL);
	}

	static constexpr size_t LevelSizeX(uint32_t level)
	{
		return RootSizeX * (1ULL << level);
	}

	static constexpr size_t LevelSizeY(uint32_t level)
	{
		return RootSizeY * (1ULL << level);
	}

private:
	T m_data[LevelOffset()];

public:
	ConstLevelProxy operator[](uint32_t level) const
	{
		return ConstLevelProxy(level, m_data);
	}

	LevelProxy operator[](uint32_t level)
	{
		return LevelProxy(level, m_data);
	}

	const T& operator[](const QuadTreeIdx& idx) const
	{
		return m_data[LevelOffset(idx.level) + idx.x * LevelSizeY(idx.level) + idx.y];
	}

	T& operator[](const QuadTreeIdx& idx)
	{
		return const_cast<T&>(std::as_const(*this)[idx]);
	}
};