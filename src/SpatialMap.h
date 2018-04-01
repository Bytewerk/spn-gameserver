#pragma once
#include <array>
#include <vector>
#include <tuple>
#include <functional>
#include "types.h"

template <class T, size_t TILES_X, size_t TILES_Y> class SpatialMap
{
	public:
		typedef std::vector<T> TileVector;

		class Iterator
		{
			public:
				Iterator(const SpatialMap& map, size_t tileNum, size_t positionInTile)
					: _map(map)
					, _tileNum(tileNum)
					, _positionInTile(positionInTile)
				{}

				Iterator& operator++()
				{
					auto numTiles = _map.m_tiles.size();
					if (_tileNum >= numTiles)
					{
						return *this;
					}

					if (++_positionInTile < _map.m_tiles[_tileNum].size())
					{
						return *this;
					}

					_positionInTile = 0;
					while (++_tileNum < numTiles)
					{
						if (_map.m_tiles[_tileNum].size()>0)
						{
							return *this;
						}
					}

					return *this;
				}

				bool operator!=(const Iterator& other)
				{
					return (&other._map!=&_map)
							|| (other._tileNum!=_tileNum)
							|| (other._positionInTile!=_positionInTile);
				}

				const T& operator*()
				{
					return _map.m_tiles[_tileNum][_positionInTile];
				}

			private:
				const SpatialMap& _map;
				size_t _tileNum;
				size_t _positionInTile;
		};

		struct Region
		{
			const SpatialMap& map;
			const int x1, y1, x2, y2;

			class Iterator
			{
				public:
					Iterator(const Region* region, bool atEnd)
						: _r(region)
						, _atEnd(atEnd)
					{
						if (_atEnd)
						{
							moveToEnd();
						}
						else
						{
							_tileX = _r->x1;
							_tileY = _r->y1;
							skipEmptyTiles();
						}
					}

					Iterator& operator++()
					{
						if (_atEnd) { return *this; }
						if (++_positionInTile >= getCurrentTile().size())
						{
							_positionInTile = 0;
							_tileX++;
							skipEmptyTiles();
						}
						return *this;
					}

					bool operator!=(const Iterator& other)
					{
						if (_atEnd)
						{
							return (other._r != _r) || (!other._atEnd);
						}

						return (other._r != _r)
							|| (other._atEnd!=_atEnd)
							|| (other._tileX != _tileX)
							|| (other._tileY != _tileY)
							|| (other._positionInTile != _positionInTile);
					}

					const T& operator*()
					{
						return getCurrentTile()[_positionInTile];
					}

				private:
					const Region* _r;
					bool _atEnd = false;
					int _tileX = 0;
					int _tileY = 0;
					size_t _positionInTile = 0;

					void skipEmptyTiles()
					{
						while (_tileY <= _r->y2)
						{
							while (_tileX <= _r->x2)
							{
								if (getCurrentTile().size()>0)
								{
									return;
								}
								_tileX++;
							}
							_tileX = _r->x1;
							_tileY++;
						}
						moveToEnd();
					}

					void moveToEnd()
					{
						_atEnd = true;
						_tileX = 0;
						_tileY = 0;
						_positionInTile = 0;
					}

					const TileVector& getCurrentTile()
					{
						return _r->map.getTileVector(_tileX, _tileY);
					}
			};

			Region(const SpatialMap& aMap, int ax1, int ay1, int ax2, int ay2)
				: map(aMap), x1(ax1), y1(ay1), x2(ax2), y2(ay2)
			{}

			Iterator begin() const
			{
				return Iterator(this, false);
			}

			Iterator end() const
			{
				return Iterator(this, true);
			}
		};

	public:
		SpatialMap(size_t fieldSizeX, size_t fieldSizeY, size_t reserveCount)
			: m_fieldSizeX(fieldSizeX)
			, m_fieldSizeY(fieldSizeY)
			, m_tileSizeX(fieldSizeX/TILES_X)
			, m_tileSizeY(fieldSizeY/TILES_Y)
		{
			for (auto &v: m_tiles)
			{
				v.reserve(reserveCount);
			}
		}

		Iterator begin() const
		{
			for (size_t i=0; i<m_tiles.size(); ++i)
			{
				if (m_tiles[i].size()>0)
				{
					return Iterator(*this, i, 0);
				}
			}
			return end();
		}

		Iterator end() const
		{
			return Iterator(*this, m_tiles.size(), 0);
		}

		void clear()
		{
			for (auto &v: m_tiles)
			{
				v.clear();
			}
		}

		size_t size() const
		{
			size_t retval = 0;
			for (auto &tile: m_tiles)
			{
				retval += tile.size();
			}
			return retval;
		}

		void addElement(const T& element)
		{
			getTileVectorForPosition(element.pos()).push_back(element);
		}

		void erase_if(std::function<bool(const T&)> predicate)
		{
			for (auto &tile: m_tiles)
			{
				tile.erase(std::remove_if(tile.begin(), tile.end(), predicate), tile.end());
			}
		}

		Region getRegion(const Vector2D& center, real_t radius) const
		{
			const Vector2D topLeft = center - Vector2D { radius, radius };
			const Vector2D bottomRight = center + Vector2D { radius, radius };
			return {
				*this,
				static_cast<int>(topLeft.x() / m_tileSizeX),
				static_cast<int>(topLeft.y() / m_tileSizeY),
				static_cast<int>(bottomRight.x() / m_tileSizeX),
				static_cast<int>(bottomRight.y() / m_tileSizeY)
			};
		}

	private:
		size_t m_fieldSizeX, m_fieldSizeY;
		real_t m_tileSizeX, m_tileSizeY;
		std::array<TileVector, TILES_X*TILES_Y> m_tiles;

		const TileVector& getTileVector(int tileX, int tileY) const
		{
			return m_tiles[wrap<TILES_Y>(tileY)*TILES_X + wrap<TILES_X>(tileX)];
		}

		TileVector& getTileVectorForPosition(const Vector2D& pos)
		{
			size_t tileX = wrap<TILES_X>(pos.x() / m_tileSizeX);
			size_t tileY = wrap<TILES_Y>(pos.y() / m_tileSizeY);
			return m_tiles[tileY*TILES_X + tileX];
		}

		template <size_t SIZE> static size_t wrap(int unwrapped)
		{
			int result = (unwrapped % SIZE);
			if (result<0) { result += SIZE; }
			return static_cast<size_t>(result);
		}

};
