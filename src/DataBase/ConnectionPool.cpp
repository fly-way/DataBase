#include "ConnectionPool.h"


ConnectionPool::ConnectionPool()
{

}

ConnectionPool::~ConnectionPool()
{

}

// ��ʼ�����ӳ�
void ConnectionPool::Run(const SDB_CONNECTION& sInfo)
{
	m_sAttr.strIp = sInfo.strIp;
	m_sAttr.uPort = sInfo.uPort;
	m_sAttr.strUserName = sInfo.strUserName;
	m_sAttr.strPassword = sInfo.strPassword;
	m_sAttr.strDbName = sInfo.strDbName;
	m_sAttr.nMinSize = sInfo.nMinSize;
	m_sAttr.nMaxSize = sInfo.nMaxSize;
	m_sAttr.nMaxIdleTime = sInfo.nMaxIdleTime;
	m_sAttr.nConnTimeout = sInfo.nConnTimeout;

	// ������ʼ����������
	for (int i = 0; i < m_sAttr.nMinSize; ++i)
	{
		Connection *pConn = new Connection();
		pConn->Connect(m_sAttr.strIp, m_sAttr.uPort, m_sAttr.strUserName, m_sAttr.strPassword, m_sAttr.strDbName);
		pConn->RefreshAliveTime(); // ��¼���ӵ���ʼ����ʱ��
		m_queConn.push(pConn);
		m_nConnCnt ++;
	}

	// ����һ���µ��̣߳���Ϊ���ӵ�������
	std::thread thdProduce(std::bind(&ConnectionPool::ProduceConnectionTask, this));
	thdProduce.detach();

	// ����һ���µĶ�ʱ�̣߳�ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ���������л���
	std::thread thdScanner(std::bind(&ConnectionPool::ScannerConnectionTask, this));
	thdScanner.detach();
}

// �����ӳ��л�ȡһ�����õĿ�������
std::shared_ptr<Connection> ConnectionPool::GetConnection()
{
	std::unique_lock<std::mutex> lock(m_lock);
	while (m_queConn.empty())
	{
		if (std::cv_status::timeout == m_cv.wait_for(lock, std::chrono::milliseconds(m_sAttr.nConnTimeout)))
		{
			if (m_queConn.empty())
			{
				// LOG("Failed to get connection:got idle connection timeout!");
				return nullptr;
			}
		}
	}

	/*
	 * shared_ptr ����ָ������ʱ����� connection ��Դֱ�� delete ����
	 * �൱�ڵ��� connection ������������ connection �ͱ� close ���ˡ�
	 * ������Ҫ�Զ��� shared_ptr ���ͷ���Դ�ķ�ʽ���� connection ֱ�ӹ黹�� queue ����*/
	std::shared_ptr<Connection> pConn(m_queConn.front(),
		[&](Connection *pcon)
		{
			// �������ڷ�����Ӧ���߳��е��õģ�����һ��Ҫ���Ƕ��е��̰߳�ȫ����
			std::unique_lock<std::mutex> lock(m_lock);
			pcon->RefreshAliveTime(); //�ڹ黹�ؿ������Ӷ���֮ǰҪ��¼һ�����ӿ�ʼ���е�ʱ��
			m_queConn.push(pcon);
		});

	m_queConn.pop();

	// ������ȡ��һ������֮��֪ͨ�����ߣ������߼����У����Ϊ��������
	m_cv.notify_all();

	return pConn;
}

// �����ڶ������߳��У�ר�Ÿ������������
void ConnectionPool::ProduceConnectionTask()
{
	for (;;)
	{
		std::unique_lock<std::mutex> lock(m_lock);
		while (!m_queConn.empty())
		{
			// ���зǿ�ʱ���˴������߳̽���ȴ�״̬
			m_cv.wait(lock);
		}

		// ��������û�е������ޣ����������µ�����
		if (m_nConnCnt < m_sAttr.nMaxSize)
		{
			Connection *p = new Connection();
			p->Connect(m_sAttr.strIp, m_sAttr.uPort, m_sAttr.strUserName, m_sAttr.strPassword, m_sAttr.strDbName);
			m_queConn.push(p);
			m_nConnCnt++;
		}

		// ֪ͨ�������̣߳���������������
		m_cv.notify_all();
	}
}

// ɨ�賬�� m_nMaxIdleTime ʱ��Ŀ������ӣ����ж������ӵĻ���
void ConnectionPool::ScannerConnectionTask()
{
	for (;;)
	{
		// ͨ��sleepʵ�ֶ�ʱЧ��
		std::this_thread::sleep_for(std::chrono::seconds(m_sAttr.nMaxIdleTime));

		// ɨ���������У��ͷŶ��������
		std::unique_lock<std::mutex> lock(m_lock);
		while (m_nConnCnt > m_sAttr.nMinSize)
		{
			Connection *pConn = m_queConn.front();
			if (pConn->GetAliveTime() >= (m_sAttr.nMaxIdleTime * 1000))
			{
				m_queConn.pop();
				m_nConnCnt--;
				delete pConn; // ����~Connection()�ͷ�����
			}
			else
			{
				// ��ͷ������û�г���_maxIdleTime���������ӿ϶�Ҳû��
				break;
			}
		}
	}
}
