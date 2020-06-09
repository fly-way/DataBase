#include "ThreadPoolMgr.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <assert.h>

/*
	���ĵ����ṩ�����ο�

	======================================

	�����õ������ݿ⣺db_test
	�ַ�����utf-8
	�������utf8_general_ci

	���԰�����Ҫ�õ��ı�
	DROP TABLE IF EXISTS `tb_test`;
	CREATE TABLE `tb_test`  (
	  `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '����ID',
	  `val` bigint(20) NOT NULL DEFAULT 0 COMMENT '',
	  PRIMARY KEY (`id`) USING BTREE
	) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8 COLLATE = utf8_general_ci COMMENT = '���Ա�' ROW_FORMAT = Compact;

	======================================

	�������Ҫͷ�ļ���
	ThreadPoolMgr.h
	Def.h

	���Žӿڣ�(�����б���� Def.h ˵������ϸʹ�÷������ɲ��ձ��ļ��µİ���)
	1���������ӳأ�			void ThreadPool::InitConnection(const SDB_CONNECTION& sInfo);
	2�������̣߳�			void ThreadPool::ThreadRun(int nThreadNum);
	3���������			void ThreadPool::AddTask(const SDB_CONNECTION_TASK& sInfo);
	4���ص�������ʽ�꣺		GGetConnCallBack(���Ա�����������)����Ϊ��̬�������������ʽ��

	ע��Ŀǰû���ṩ���������ʾ���ɸ���ҵ��淶�������
*/


// tb_test ������ӳ��
struct MyStruct
{
	INT64 id;
	INT64 val;
};

class MyClass
{
public:
	MyClass() {}
	~MyClass() {}

	void ProcessDB(const std::string strTaskType, const vecMysqlRes& vecResult)
	{
		if (strTaskType == "query_tb_test")
		{
			/*
			for (auto vecParam : vecResult)
			{
				for (auto val : vecParam)
				{
					std::cout << val << std::endl;
				}
			}
			*/

			std::vector<MyStruct> vecTest;
			for (size_t i = 0; i < vecResult.size(); ++i)
			{
				// ��ѯ�����е� tb_test ֻ�����У������ܴ����������
				assert(vecResult[i].size() == 2);

				MyStruct sInfo;
				sInfo.id = std::stoi(vecResult[i][0]);
				sInfo.val = std::stoi(vecResult[i][1]);

				vecTest.push_back(sInfo);
			}

			std::cout << "query_tb_test call back" << std::endl;
		}
	}
};


int main()
{
	const long long idConn = 1;

	SDB_CONNECTION sInfo;
	sInfo.nConnID = idConn;
	sInfo.strIp = "192.168.6.238";
	sInfo.uPort = 3306;
	sInfo.strUserName = "root";
	sInfo.strPassword = "654321";
	sInfo.strDbName = "db_test";
	sInfo.nMinSize = 5;
	sInfo.nMaxSize = 1024;
	sInfo.nMaxIdleTime = 10;
	sInfo.nConnTimeout = 100;

//===============================================================================================

	// �ص���Ǿ�̬��Ա��������
	SDB_CONNECTION_TASK sTask;
	sTask.nConnID = idConn;
	sTask.eType = CONN_QUERY;
	sTask.strSql = "select * from tb_test where id <= 1010;";
	sTask.sQueryBack.strTaskType = "query_tb_test";

	MyClass* my;
	sTask.sQueryBack.CallBack = GGetConnCallBack(MyClass::ProcessDB, my);
	
	GThreadPool.InitConnection(sInfo);
	GThreadPool.ThreadRun(5);
	GThreadPool.AddTask(sTask);


//===============================================================================================
/* ���Խ��
	�̳߳� + ���ӳأ����������߳��������ӣ������ʱ���Եݼ���
	���ӳأ�������Ƶ���������ͷ���������Ĵ������ܿ��������Ǻ�ʱ�������ͨ����û�����ж���𣨿�����С���ⲻ�����죬���ߴ�����Ҫ�Ľ���
	��ͨ���ӣ�ÿ�β�ѯ���ᴴ�����ӣ���ѯ���������ӣ�����Ӧ������͵�
*/


	// �̳߳�+���ӳز���
	/*
	SDB_CONNECTION_TASK sTask;
	sTask.nConnID = idConn;
	sTask.eType = CONN_UPDATE;
	// sTask.strSql = "INSERT into tb_test value (0,1);";

	for (int i = 1; i <= 10000; ++i)
	{
		std::stringstream ss;
		ss << "INSERT into tb_test value (" << 1000+i << ",1);";
		sTask.strSql = ss.str();
		GThreadPool.AddTask(sTask);
	}

	std::cout << clock() << std::endl;

	GThreadPool.InitConnection(sInfo);
	GThreadPool.ThreadRun(5);
	*/


	// ���ӳز���
	/*
	std::cout << clock() << std::endl;
	ConnectionPool *pConnPool = new ConnectionPool();
	pConnPool->Run(sInfo);

	for (int i = 1; i <= 10000; ++i)
	{
		std::shared_ptr<Connection> pConn = pConnPool->GetConnection();
		if (pConn)
			pConn->Update("INSERT into tb_test value (0,1);");
	}
	std::cout << clock() << std::endl;
	*/

/* 
	// ��ͨ���Ӳ���
	std::cout << clock() << std::endl;
	for (int i = 1; i <= 10000; ++i)
	{
		Connection conn;
		conn.Connect("192.168.6.238", 3306, "root", "654321", "db_test");
		conn.Update("INSERT into tb_test value (0,1);");
	}
	std::cout << clock() << std::endl;
*/

	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	return 0;
}