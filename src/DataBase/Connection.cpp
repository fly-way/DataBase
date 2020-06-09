#include "Connection.h"


Connection::Connection()
{
	m_pConn = mysql_init(nullptr);
}

Connection::~Connection()
{
	if (m_pConn != nullptr)
		mysql_close(m_pConn);
}

// 连接数据库
bool Connection::Connect(std::string strIP, unsigned short uPort, std::string strUserName, std::string strPassword, std::string strDbName)
{
	MYSQL *p = mysql_real_connect(m_pConn, strIP.c_str(), strUserName.c_str(),
		strPassword.c_str(), strDbName.c_str(), uPort, nullptr, 0);

	return p != nullptr;
}

// 更新操作 insert delete update
bool Connection::Update(std::string strSql)
{
	if (mysql_query(m_pConn, strSql.c_str()))
	{
		return false;
	}
	return true;
}

// 查询操作 select
void Connection::Query(std::string strSql, SDB_CONNECTION_TASK::QUERY_BACK& sInfo)
{
	MYSQL_ROW mysql_row;
	MYSQL_RES *mysql_res;

	if (mysql_query(m_pConn, strSql.c_str()) != 0)
	{
		return;
	}

	mysql_res = mysql_store_result(m_pConn);
	if (mysql_res == NULL)
	{
		return;
	}

	vecMysqlRes vecRes;
	std::string str("");
	while (mysql_row = mysql_fetch_row(mysql_res))
	{
		vecMysqlParam vecParam;
		for (UINT i = 0; i < mysql_num_fields(mysql_res); i++)
			vecParam.push_back(mysql_row[i]);

		vecRes.push_back(vecParam);
	}

	sInfo.CallBack(sInfo.strTaskType, vecRes);
	mysql_free_result(mysql_res);
}

// 刷新连接的起始空闲时刻
void Connection::RefreshAliveTime()
{
	m_nAlivetime = clock();
}

// 返回连接空闲的时长
clock_t Connection::GetAliveTime()
{
	return clock() - m_nAlivetime;
}

