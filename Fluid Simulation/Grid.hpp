#include <vector>

template <typename T>
class GridRow
{
private:
	std::vector<T> vGridRow = {};

public:
	T& operator[](int x);
	void resize(int x);

};

template<typename T>
T& GridRow<T>::operator[](int x)
{
	return vGridRow[x];
}

template<typename T>
void GridRow<T>::resize(int x)
{
	vGridRow.resize(x);
}

template <typename T>
class Grid
{
private:
	std::vector<GridRow<T>> vvGrid = {};

public:
	GridRow<T>& operator[](int x);
	void resize(int x, int y);

};

template <typename T>
GridRow<T>& Grid<T>::operator[](int x)
{
	return vvGrid[x];
}

template<typename T>
void Grid<T>::resize(int x, int y)
{
	vvGrid.resize(x);
	for (int i = 0; i < vvGrid.size(); i++)
	{
		vvGrid[i].resize(y);
	}
}
