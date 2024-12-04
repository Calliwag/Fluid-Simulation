#pragma once
#include <mutex>
#include <queue>

template<typename T>
class ThreadQ
{
private:
	std::queue<T> Q;
	std::mutex M;

public:
	void Push(T val)
	{
		std::lock_guard<std::mutex> lock(M);
		Q.push(val);
		return;
	}
	T Get()
	{
		std::lock_guard<std::mutex> lock(M);
		T val = Q.front();
		Q.pop();
		return val;
	}
	bool IsEmpty()
	{
		std::lock_guard<std::mutex> lock(M);
		return Q.size() == 0;
	}
};