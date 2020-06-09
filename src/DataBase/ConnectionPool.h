#pragma once

#include "Connection.h"
#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>
#include "Def.h"


class ConnectionPool
{
public:
	
	ConnectionPool();
	~ConnectionPool();

public: // interface

	// 初始化连接池
	void Run(const SDB_CONNECTION& sInfo);

	// 从连接池中获取一个可用的空闲连接
	std::shared_ptr<Connection> GetConnection();

private:

	// 运行在独立的线程中，专门负责生产新连接
	void ProduceConnectionTask();

	// 扫描超过maxIdleTime时间的空闲连接，进行对于连接的回收
	void ScannerConnectionTask();

	SDB_CONNECTION			m_sAttr;			// 连接基本属性
	bool					m_bSetAttr;			// 基本参数是否设置完成
	std::queue<Connection*> m_queConn;			// 存储MySQL连接的队列
	std::mutex				m_lock;				// 维护连接队列的线程安全互斥锁
	std::atomic_int			m_nConnCnt;			// 记录 connection 连接的总数量
	std::condition_variable m_cv;				// 设置条件变量，用于连接生产线程和连接消费线程的通信
};

#define GConnectionPool		(ConnectionPool::GetInstance())
