#pragma once

#include <string>
#include <functional>
#include <vector>

typedef std::vector<std::string /*��*/> vecMysqlParam;
typedef std::vector<vecMysqlParam /*��*/> vecMysqlRes;
typedef std::function<void(const std::string /*strTaskType*/, const vecMysqlRes& /*vecResult*/)> ConnCallBack;

#define GGetConnCallBack(func, obj)	std::bind(&func, obj, std::placeholders::_1, std::placeholders::_2);


enum ECONN_TYPE
{
	CONN_QUERY,
	CONN_UPDATE,
};

struct SDB_CONNECTION
{
	long long		nConnID;			// ���ӳ��������û����ж��壩
	std::string		strIp;				// MySQL��ip��ַ
	unsigned short	uPort;				// MySQL�Ķ˿ںţ�Ĭ��Ϊ3306
	std::string		strUserName;		// MySQL��½�û���				
	std::string		strPassword;		// MySQL��½����
	std::string		strDbName;			// ���ӵ����ݿ�����
	int				nMinSize;			// ���ӳص���С����������ʼ���ʹ��������ӣ���һֱ���֣�
	int				nMaxSize;			// ���ӳص������������ҵ��Ƶ��ʱ������⿪���µ����ӣ����ܵ����Ӳ��ᳬ����ֵ��
	int				nMaxIdleTime;		// ���ӳص�������ʱ�䣨�룬��������ڹ涨ʱ����û�б�ʹ�ã���ᱻϵͳ���գ�ֱ������������С��������
	int				nConnTimeout;		// ���ӳػ�ȡ���ӵĳ�ʱʱ��(���룬ҵ��Ƶ��ʱ��ȡ�����������ӣ���ʱ�ᴦ������״̬�����ڿ����µ����ӣ���ʱ�䳬����ֵ������ͷ�����)
};

struct SDB_CONNECTION_TASK
{
	long long		nConnID;			// ���ӳ�������Ӧ�ó�����ܴ������Ӷ�����ݿ⣩
	ECONN_TYPE		eType;				// ��������
	std::string		strSql;				// �������

	struct QUERY_BACK
	{
		std::string		strTaskType;	// �����ʶ���û��Զ��壬�첽�������ʱ��֪ͨ�û�����һ������
		ConnCallBack	CallBack;		// �ص��������첽������أ�

		QUERY_BACK() : CallBack(nullptr) {}
	};

	QUERY_BACK		sQueryBack;			// �ص���Ϣ

	bool ChkFormat() const
	{
		return (eType == CONN_UPDATE) || (eType == CONN_QUERY && sQueryBack.CallBack != nullptr);
	}
};	
