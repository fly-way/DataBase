#include "ConnectionPool.h"


ConnectionPool::ConnectionPool()
{

}

ConnectionPool::~ConnectionPool()
{

}

// 初始化连接池
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

	// 创建初始数量的连接
	for (int i = 0; i < m_sAttr.nMinSize; ++i)
	{
		Connection *pConn = new Connection();
		pConn->Connect(m_sAttr.strIp, m_sAttr.uPort, m_sAttr.strUserName, m_sAttr.strPassword, m_sAttr.strDbName);
		pConn->RefreshAliveTime(); // 记录连接的起始空闲时刻
		m_queConn.push(pConn);
		m_nConnCnt ++;
	}

	// 启动一个新的线程，作为连接的生产者
	std::thread thdProduce(std::bind(&ConnectionPool::ProduceConnectionTask, this));
	thdProduce.detach();

	// 启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，并对其进行回收
	std::thread thdScanner(std::bind(&ConnectionPool::ScannerConnectionTask, this));
	thdScanner.detach();
}

// 从连接池中获取一个可用的空闲连接
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
	 * shared_ptr 智能指针析构时，会把 connection 资源直接 delete 掉，
	 * 相当于调用 connection 的析构函数， connection 就被 close 掉了。
	 * 这里需要自定义 shared_ptr 的释放资源的方式，把 connection 直接归还到 queue 当中*/
	std::shared_ptr<Connection> pConn(m_queConn.front(),
		[&](Connection *pcon)
		{
			// 这里是在服务器应用线程中调用的，所以一定要考虑队列的线程安全操作
			std::unique_lock<std::mutex> lock(m_lock);
			pcon->RefreshAliveTime(); //在归还回空闲连接队列之前要记录一下连接开始空闲的时刻
			m_queConn.push(pcon);
		});

	m_queConn.pop();

	// 消费者取出一个连接之后，通知生产者，生产者检查队列，如果为空则生产
	m_cv.notify_all();

	return pConn;
}

// 运行在独立的线程中，专门负责产生新连接
void ConnectionPool::ProduceConnectionTask()
{
	for (;;)
	{
		std::unique_lock<std::mutex> lock(m_lock);
		while (!m_queConn.empty())
		{
			// 队列非空时，此处生产线程进入等待状态
			m_cv.wait(lock);
		}

		// 连接数量没有到达上限，继续创建新的连接
		if (m_nConnCnt < m_sAttr.nMaxSize)
		{
			Connection *p = new Connection();
			p->Connect(m_sAttr.strIp, m_sAttr.uPort, m_sAttr.strUserName, m_sAttr.strPassword, m_sAttr.strDbName);
			m_queConn.push(p);
			m_nConnCnt++;
		}

		// 通知消费者线程，可以消费连接了
		m_cv.notify_all();
	}
}

// 扫描超过 m_nMaxIdleTime 时间的空闲连接，进行对于连接的回收
void ConnectionPool::ScannerConnectionTask()
{
	for (;;)
	{
		// 通过sleep实现定时效果
		std::this_thread::sleep_for(std::chrono::seconds(m_sAttr.nMaxIdleTime));

		// 扫描整个队列，释放多余的连接
		std::unique_lock<std::mutex> lock(m_lock);
		while (m_nConnCnt > m_sAttr.nMinSize)
		{
			Connection *pConn = m_queConn.front();
			if (pConn->GetAliveTime() >= (m_sAttr.nMaxIdleTime * 1000))
			{
				m_queConn.pop();
				m_nConnCnt--;
				delete pConn; // 调用~Connection()释放连接
			}
			else
			{
				// 队头的连接没有超过_maxIdleTime，其它连接肯定也没有
				break;
			}
		}
	}
}
