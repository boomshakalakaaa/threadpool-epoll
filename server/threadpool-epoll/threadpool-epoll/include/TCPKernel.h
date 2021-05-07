#ifndef _TCPKERNEL_H
#define _TCPKERNEL_H



#include "TCPNet.h"
#include "Mysql.h"
#include<map>
#include <list>

class TcpKernel;
typedef void (TcpKernel::*PFUN)(int,char*,int nlen);

typedef struct
{
    PackType m_type;
    PFUN m_pfun;
} ProtocolMap;



class TcpKernel:public IKernel
{
public:
    int Open();
    void Close();
    void DealData(int,char*,int);

    //登录
    void LoginRq(int,char*,int);

private:
    CMysql * m_sql;
    TcpNet * m_tcp;

    map<int , int> m_mapIDToUserFD;

};

#endif
