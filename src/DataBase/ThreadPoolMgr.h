#pragma once

#include <iostream>
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <map>
#include "Def.h"
#include "ConnectionPool.h"


typedef void(*DBProcessFunc)(int, int);

class ThreadPool
{
public:
	static ThreadPool& GetInstance();

private:

	ThreadPool();
	~ThreadPool();

public:

	void ThreadRun(int nThreadNum);
	void InitConnection(const SDB_CONNECTION& sInfo);
	void AddTask(const SDB_CONNECTION_TASK& sInfo);

private:

	void RunTask();

private:

	std::queue<SDB_CONNECTION_TASK>			m_queTask;		// 任务队列
	std::map<long long, ConnectionPool*>	m_mapConn;		// 连接池

	std::mutex								m_lock;			// 维护连接队列的线程安全互斥锁
	std::condition_variable					m_cv;			// 设置条件变量，用于连接生产线程和连接消费线程的通信
};

#define GThreadPool		(ThreadPool::GetInstance())
