#include "ThreadPoolMgr.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <assert.h>

/*
	此文档仅提供案例参考

	======================================

	测试用到的数据库：db_test
	字符集：utf-8
	排序规则：utf8_general_ci

	测试案例需要用到的表：
	DROP TABLE IF EXISTS `tb_test`;
	CREATE TABLE `tb_test`  (
	  `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '自增ID',
	  `val` bigint(20) NOT NULL DEFAULT 0 COMMENT '',
	  PRIMARY KEY (`id`) USING BTREE
	) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8 COLLATE = utf8_general_ci COMMENT = '测试表' ROW_FORMAT = Compact;

	======================================

	所需包含要头文件：
	ThreadPoolMgr.h
	Def.h

	开放接口：(参数列表参照 Def.h 说明，详细使用方法均可参照本文件下的案例)
	1、创建连接池：			void ThreadPool::InitConnection(const SDB_CONNECTION& sInfo);
	2、启动线程：			void ThreadPool::ThreadRun(int nThreadNum);
	3、添加任务：			void ThreadPool::AddTask(const SDB_CONNECTION_TASK& sInfo);
	4、回调函数格式宏：		GGetConnCallBack(类成员函数，类对象)，若为静态函数，则无需格式化

	注：目前没有提供错误输出提示，可根据业务规范自行添加
*/


// tb_test 表数据映射
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
				// 查询案例中的 tb_test 只有两列，不可能存在其它情况
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

	// 回调类非静态成员函数测试
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
/* 测试结果
	线程池 + 连接池：任务随着线程数的增加，任务耗时线性递减。
	连接池：避免了频繁创建、释放连接引起的大量性能开销，但是耗时方面和普通连接没看出有多大差别（可能量小，测不出差异，或者代码需要改进）
	普通连接：每次查询都会创建连接，查询后销毁连接，性能应该是最低的
*/


	// 线程池+连接池测试
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


	// 连接池测试
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
	// 普通连接测试
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