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

	// ��ʼ�����ӳ�
	void Run(const SDB_CONNECTION& sInfo);

	// �����ӳ��л�ȡһ�����õĿ�������
	std::shared_ptr<Connection> GetConnection();

private:

	// �����ڶ������߳��У�ר�Ÿ�������������
	void ProduceConnectionTask();

	// ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����ж������ӵĻ���
	void ScannerConnectionTask();

	SDB_CONNECTION			m_sAttr;			// ���ӻ�������
	bool					m_bSetAttr;			// ���������Ƿ��������
	std::queue<Connection*> m_queConn;			// �洢MySQL���ӵĶ���
	std::mutex				m_lock;				// ά�����Ӷ��е��̰߳�ȫ������
	std::atomic_int			m_nConnCnt;			// ��¼ connection ���ӵ�������
	std::condition_variable m_cv;				// ���������������������������̺߳����������̵߳�ͨ��
};

#define GConnectionPool		(ConnectionPool::GetInstance())
