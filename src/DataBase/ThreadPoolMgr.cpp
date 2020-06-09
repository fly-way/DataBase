#include <assert.h>
#include "ThreadPoolMgr.h"


ThreadPool& ThreadPool::GetInstance()
{
	static ThreadPool obj;
	return obj;
}

ThreadPool::ThreadPool()
{
}

ThreadPool::~ThreadPool()
{
}

void ThreadPool::ThreadRun(int nThreadNum)
{
	for (int i = 0; i < nThreadNum; i++)
	{ 
		std::thread thdRunTask(std::bind(&ThreadPool::RunTask, this));
		thdRunTask.detach();
	}
}

void ThreadPool::InitConnection(const SDB_CONNECTION& sInfo)
{
	assert(m_mapConn.find(sInfo.nConnID) == m_mapConn.end());

	ConnectionPool *pConnPool = new ConnectionPool();
	pConnPool->Run(sInfo);
	m_mapConn.insert(std::make_pair(sInfo.nConnID, pConnPool));
}

void ThreadPool::AddTask(const SDB_CONNECTION_TASK& sInfo)
{
	assert(sInfo.ChkFormat());

	m_queTask.push(sInfo);
	m_cv.notify_one();
}

void ThreadPool::RunTask()
{
	for (;;)
	{
		std::unique_lock<std::mutex> lock(m_lock);
		while (m_queTask.empty())
		{
			m_cv.wait(lock);
		}

		SDB_CONNECTION_TASK sTask = m_queTask.front();
		auto iter = m_mapConn.find(sTask.nConnID);
		assert(iter != m_mapConn.end());

		std::shared_ptr<Connection> pConn = iter->second->GetConnection();
		assert(nullptr != pConn);

		m_queTask.pop();

		// ≤‚ ‘”√ ±
		/*
		if (m_queTask.empty())
			std::cout << clock() << std::endl;
		*/

		lock.unlock();

		switch (sTask.eType)
		{
		case CONN_QUERY:
			pConn->Query(sTask.strSql, sTask.sQueryBack);
			break;
		case CONN_UPDATE:
			pConn->Update(sTask.strSql);
			break;
		default:
			break;
		}

		m_cv.notify_one();
	}
}

