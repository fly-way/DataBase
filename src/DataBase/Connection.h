#pragma once

#include <string>
#include <winsock.h>
#include <libmysql\mysql.h>
#include <ctime>
#include "Def.h"

class Connection
{
public:
	Connection();
	~Connection();

	// 连接数据库
	bool Connect(std::string strIP, unsigned short uPort, std::string strUserName, std::string strPassword, std::string strDbName);

	// 更新操作 insert delete update
	bool Update(std::string strSql);

	// 查询操作 select
	void Query(std::string strSql, SDB_CONNECTION_TASK::QUERY_BACK& sInfo);

	// 刷新连接的起始空闲时刻
	void RefreshAliveTime();

	// 返回连接空闲的时长
	clock_t GetAliveTime();

private:
	MYSQL* m_pConn;			// 表示和MySQL Server的一条连接
	clock_t m_nAlivetime;	// 记录进入空闲状态后的起始存活时刻（即在队列中出现的时刻）
};