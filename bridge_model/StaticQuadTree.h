#pragma once
#include <stdint.h>

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
		ConstLevelProxy(uint32_t level, const T* data) :m_level(level), m_level_data(data + NumElementsBeforeLevel(level)) {}

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
	static constexpr size_t NumElementsBeforeLevel(uint32_t level = NumLevels)
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
	T m_data[NumElementsBeforeLevel()];

public:
	ConstLevelProxy operator[](uint32_t level) const
	{
		return ConstLevelProxy(level, m_data);
	}

	LevelProxy operator[](uint32_t level)
	{
		return LevelProxy(level, m_data);
	}
};