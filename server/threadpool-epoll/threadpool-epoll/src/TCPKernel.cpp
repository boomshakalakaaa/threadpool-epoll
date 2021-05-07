#include<TCPKernel.h>
#include "packdef.h"
#include<stdio.h>
#include<sys/time.h>


using namespace std;




static const ProtocolMap m_ProtocolMapEntries[] =
{
    {DEF_PACK_LOGIN_RQ , &TcpKernel::LoginRq},
    {0,0}
};

int TcpKernel::Open()
{
    m_sql = new CMysql;
    m_tcp = new TcpNet(this);
    m_tcp->SetpThis(m_tcp);
    pthread_mutex_init(&m_tcp->alock,NULL);
    pthread_mutex_init(&m_tcp->rlock,NULL);
    if(  !m_sql->ConnectMysql("localhost","root","111111","YouKu")  )
    {
        printf("Conncet Mysql Failed...\n");
        return FALSE;
    }
    else
    {
        printf("MySql Connect Success...\n");
    }
    if( !m_tcp->InitNetWork()  )
    {
        printf("InitNetWork Failed...\n");
        return FALSE;
    }
    else
    {
        printf("Init Net Success...\n");
    }

    return TRUE;
}

void TcpKernel::Close()
{

    m_sql->DisConnect();
    m_tcp->UnInitNetWork();
}


void TcpKernel::DealData(int clientfd,char *szbuf,int nlen)
{
    PackType *pType = (PackType*)szbuf;
    int i = 0;
    while(1)
    {
        if(*pType == m_ProtocolMapEntries[i].m_type)
        {
            auto fun= m_ProtocolMapEntries[i].m_pfun;
            (this->*fun)(clientfd,szbuf,nlen);
        }
        else if(m_ProtocolMapEntries[i].m_type == 0 &&
                m_ProtocolMapEntries[i].m_pfun == 0)
            return;
        ++i;
    }
    return;
}

//登录
void TcpKernel::LoginRq(int clientfd ,char* szbuf,int nlen)
{
    printf("clientfd:%d LoginRq\n", clientfd);

    STRU_LOGIN_RQ * rq = (STRU_LOGIN_RQ *)szbuf;
    STRU_LOGIN_RS rs;
    //登录请求结果
    //#define userid_no_exist      0
    //#define password_error       1
    //#define login_sucess         2
    //#define user_online          3
    char buf[_DEF_SQLIEN] = "";
    sprintf(buf, "select password ,id from t_UserData where name = '%s';", rq->m_szUser);
    list<string> resList;
    bool res = m_sql->SelectMysql(buf , 2 , resList);
    if( !res )
    {
        cout<< "SelectMysql errot" << buf <<endl;
    }
    if( resList.size() > 0 )
    {

        if(  strcmp( resList.front().c_str()  ,rq->m_szPassword ) == 0  )
        {
            rs.m_lResult = login_sucess;
            resList.pop_front();
            rs.m_UserID = atoi(resList.front().c_str());

            //存储映射关系
            this->m_mapIDToUserFD[rs.m_UserID ] = clientfd;

        }else
        {
            rs.m_lResult = password_error;
        }
    }else
    {
        rs.m_lResult = userid_no_exist;
    }

    m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );

}
