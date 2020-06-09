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

	std::queue<SDB_CONNECTION_TASK>			m_queTask;		// �������
	std::map<long long, ConnectionPool*>	m_mapConn;		// ���ӳ�

	std::mutex								m_lock;			// ά�����Ӷ��е��̰߳�ȫ������
	std::condition_variable					m_cv;			// ���������������������������̺߳����������̵߳�ͨ��
};

#define GThreadPool		(ThreadPool::GetInstance())
