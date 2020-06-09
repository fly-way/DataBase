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

	// �������ݿ�
	bool Connect(std::string strIP, unsigned short uPort, std::string strUserName, std::string strPassword, std::string strDbName);

	// ���²��� insert delete update
	bool Update(std::string strSql);

	// ��ѯ���� select
	void Query(std::string strSql, SDB_CONNECTION_TASK::QUERY_BACK& sInfo);

	// ˢ�����ӵ���ʼ����ʱ��
	void RefreshAliveTime();

	// �������ӿ��е�ʱ��
	clock_t GetAliveTime();

private:
	MYSQL* m_pConn;			// ��ʾ��MySQL Server��һ������
	clock_t m_nAlivetime;	// ��¼�������״̬�����ʼ���ʱ�̣����ڶ����г��ֵ�ʱ�̣�
};