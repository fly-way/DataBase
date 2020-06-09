#pragma once

#include <string>
#include <functional>
#include <vector>

typedef std::vector<std::string /*列*/> vecMysqlParam;
typedef std::vector<vecMysqlParam /*行*/> vecMysqlRes;
typedef std::function<void(const std::string /*strTaskType*/, const vecMysqlRes& /*vecResult*/)> ConnCallBack;

#define GGetConnCallBack(func, obj)	std::bind(&func, obj, std::placeholders::_1, std::placeholders::_2);


enum ECONN_TYPE
{
	CONN_QUERY,
	CONN_UPDATE,
};

struct SDB_CONNECTION
{
	long long		nConnID;			// 连接池索引（用户自行定义）
	std::string		strIp;				// MySQL的ip地址
	unsigned short	uPort;				// MySQL的端口号，默认为3306
	std::string		strUserName;		// MySQL登陆用户名				
	std::string		strPassword;		// MySQL登陆密码
	std::string		strDbName;			// 连接的数据库名称
	int				nMinSize;			// 连接池的最小连接量（初始化就创建好连接，并一直保持）
	int				nMaxSize;			// 连接池的最大连接量（业务频繁时，会额外开辟新的连接，但总的连接不会超过该值）
	int				nMaxIdleTime;		// 连接池的最大空闲时间（秒，相关连接在规定时间内没有被使用，则会被系统回收，直到数量等于最小连接量）
	int				nConnTimeout;		// 连接池获取连接的超时时间(毫秒，业务频繁时，取不到空闲连接，此时会处于阻塞状态，用于开辟新的连接，若时间超过该值，则会释放阻塞)
};

struct SDB_CONNECTION_TASK
{
	long long		nConnID;			// 连接池索引（应用程序可能存在连接多个数据库）
	ECONN_TYPE		eType;				// 操作类型
	std::string		strSql;				// 操作语句

	struct QUERY_BACK
	{
		std::string		strTaskType;	// 任务标识（用户自定义，异步结果返回时会通知用户是哪一种任务）
		ConnCallBack	CallBack;		// 回调函数（异步结果返回）

		QUERY_BACK() : CallBack(nullptr) {}
	};

	QUERY_BACK		sQueryBack;			// 回调信息

	bool ChkFormat() const
	{
		return (eType == CONN_UPDATE) || (eType == CONN_QUERY && sQueryBack.CallBack != nullptr);
	}
};	
