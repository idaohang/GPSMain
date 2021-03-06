#include "worker.h"
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QSqlQuery>


Worker::Worker(QThread *parent) :
    QThread(parent),
    bThread(false),
    bAccept(true)
{
}

Worker::~Worker()
{
    db.close();
}

void Worker::Initialize()
{
    ReadCfgFile();
    OpenComm();
}


void Worker::ReadCfgFile()
{

    QString path = QCoreApplication::applicationDirPath();
    path.append("/GPS.cfg");
    qDebug()<<"path"<<path;
    if (!QFileInfo(path).exists())
    {
        emit SendInfo(QStringLiteral("无法找到GPS.cfg文件"));
        return;
    }
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE", "drive");//打开数据库
        db.setDatabaseName(path);
        db.open();
        if (!db.isOpen())
        {
            emit SendInfo(QStringLiteral("无法打开数据库"));
            return;
        }
        else
        {

            QSqlQuery query(db);
            query.exec("select Value from comm where Id = 20");
            query.next();
            m_portNo = query.value(0).toString();
            qDebug()<<"m_portNo"<<m_portNo;
            query.exec("select Value from comm where Id = 21");
            query.next();
            m_baudRate = query.value(0).toString();
            qDebug()<<"m_baudRate"<<m_baudRate;
            query.exec("select Value from comm where Id = 22");
            query.next();
            m_databits = query.value(0).toString();
            query.exec("select Value from comm where Id = 23");
            query.next();
            m_stopbits = query.value(0).toString();
            query.exec("select Value from comm where Id = 24");
            query.next();
            m_parity = query.value(0).toString();
            qDebug()<<"m_parity"<<m_parity;
            query.exec("select Value from comm where Id = 25");
            query.next();
            m_timeout = query.value(0).toString();
        }

    }

}


void Worker::OpenComm()
{
    if (m_portNo == tr(""))
    {
        return;
    }
    QString portName = "com";
    portName.append(m_portNo);
    myCom.reset(new Win_QextSerialPort(portName,QextSerialBase::EventDriven));
    qDebug()<<"start open";
    //定义串口对象，并传递参数，在构造函数里对其进行初始化
    bool com = myCom ->open(QIODevice::ReadWrite);

    if (m_baudRate==tr("9600")) //根据组合框内容对串口进行设置

        myCom->setBaudRate(BAUD9600);

    else if (m_baudRate==tr("115200"))

        myCom->setBaudRate(BAUD115200);
    else if (m_baudRate==tr("300"))

        myCom->setBaudRate(BAUD300);
    else if (m_baudRate==tr("600"))

        myCom->setBaudRate(BAUD600);
    else if (m_baudRate==tr("1200"))

        myCom->setBaudRate(BAUD1200);
    else if (m_baudRate==tr("2400"))

        myCom->setBaudRate(BAUD2400);
    else if (m_baudRate==tr("4800"))

        myCom->setBaudRate(BAUD4800);

    //设置波特率

    if (m_databits==tr("8"))

        myCom->setDataBits(DATA_8);

    else if (m_databits==tr("7"))

        myCom->setDataBits(DATA_7);

    //设置数据位

    if (m_parity==tr("0"))

        myCom->setParity(PAR_NONE);

    else if (m_parity==tr("1"))

        myCom->setParity(PAR_ODD);

    else if (m_parity==tr("2"))

        myCom->setParity(PAR_EVEN);
    else if (m_parity == tr("3"))
        myCom->setParity(PAR_MARK);

    //设置奇偶校验

    if (m_stopbits==tr("1"))

        myCom->setStopBits(STOP_1);

    else if (m_stopbits==tr("2"))

        myCom->setStopBits(STOP_2);
    else if (m_stopbits == tr("1.5"))
        myCom->setStopBits(STOP_1_5);

    //设置停止位

    myCom->setFlowControl(FLOW_OFF); //设置数据流控制，我们使用无数据流控制的默认设置
    int time = m_timeout.toInt();
    qDebug()<<"time"<<time;
    myCom->setTimeout(time); //设置延时

    QString string = m_databits;
    portName.append(" ");
    portName.append(string);
    portName.append("-");
    if(m_parity == "0")
    {
        portName.append("N");
    }
    else if(m_parity == "1")
    {
        portName.append("O");
    }
    else if(m_parity == "2")
    {
        portName.append("E");
    }
    else if(m_parity == "3")
    {
        portName.append("M");
    }
    portName.append("-");
    if (m_stopbits == "1")
        portName.append("1");
    else if(m_stopbits == "1.5")
        portName.append("1.5");
    else if (m_stopbits == "2")
        portName.append("2");
    if(!com)
    {
        qDebug()<<"!com";
        portName.append(QStringLiteral(" 打开失败"));
        emit SendInfo(portName);
    }
    else
    {
        qDebug()<<"portName"<<portName;
        portName.append(QStringLiteral(" 打开成功"));
        emit SendInfo(portName);
    }


}



void Worker::run()
{
    qDebug()<<"run";
    while (!bThread)
    {
        msleep(300);
        if (myCom->isOpen())
        {
            char data[1024] = "";
            myCom->read(data,1024);
            int length = strlen(data);
            if (length == 0)
            {
                continue;
            }
            if (bAccept)
            {
                //   emit ReceiveText(data, length);
                Decoding(data, length);
            }


        }
        else
        {
            emit SendInfo(QStringLiteral("串口没有打开"));
            bThread = true;
        }

    }
}

void Worker::Decoding(char *data, int length)
{
    if (*data != '$')
    {
        return;
    }

    char tempdata_tra[1024] = "";
    char tempdata_gga[1024] = "";
    bool bSave_tra = false;
    bool bSave_gga = false;
    int ggaIndex = 0;
    int traIndex = 0;
    for (int i = 0; i < 1024; ++i)
    {
        if (*(data + i) == '$')
        {
            char temp[7] = "";
            for (int j = 0; j < 6; ++j)
            {
                temp[j] = *(data + i + j);
            }

            if (strcmp(temp, "$GPTRA") == 0)
            {

                bSave_tra = true;
            }
            else if(strcmp(temp, "$GPGGA") == 0)
            {

                bSave_gga = true;
            }

        }
        if (bSave_gga)
        {
            if (*(data + i + 1) != '$')
            {
                tempdata_gga[ggaIndex] = *(data + i);
                ++ggaIndex;
            }
            else
            {
                bSave_gga = false;
            }
        }
        if (bSave_tra)
        {
            if (*(data + i + 1) != '$')
            {
                tempdata_tra[traIndex] = *(data + i);
                ++traIndex;
            }
            else
            {
                bSave_tra = false;
            }
        }

    }
    if (ggaIndex != 0)
    {
        emit ReceiveText(tempdata_gga, ggaIndex);
    }
    if (traIndex != 0)
    {
        emit ReceiveText(tempdata_tra, traIndex);
    }

}
