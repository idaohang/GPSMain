#include "sendworker.h"


int hh=10,mm=0,QF=4,sat_No=15,stn_ID=4;
double ss=0,heading=044.56,pitch=-09.74,roll=0,age=0,hdop=1,alt=50.22;
double lat=3110.4693903,lon=12123.2621695;


SendWorker::SendWorker(QObject *parent) :
    QThread(parent)
{
}


void SendWorker::run()
{
    QString strtemp = "", temp1 = "";
    while (!bThread)
    {
        sleep(1);
        if(ss>=60){ss=0;mm++;} //字符型能直接与数字比较？？？？
        if(mm>=60){mm=0;hh++;}//hhmmss.ss
        if(hh>=24)hh=0;

        if(hh<10)strtemp+='0';
        temp1.sprintf("%d", hh);
        strtemp+=temp1;

        if(mm<10)strtemp+='0';
        temp1.sprintf("%d", mm);
        strtemp+=temp1;

        if(ss<10)strtemp+='0';
        temp1.sprintf("%.2f", ss);
        strtemp+=temp1;strtemp+=",";

        if(heading<100)strtemp+='0'; //heading方向角hhh.hh(0 - 360)
        if(heading<10)strtemp+='0';
        temp1.sprintf("%.2f", heading);
        strtemp+=temp1;strtemp+=",";

        if(pitch<0)strtemp+="-";
        if((fabs)(pitch)<10)strtemp+='0'; //pitch俯仰角ppp.pp(-90 - 90)
        temp1.sprintf("%.2f", (fabs)(pitch));
        strtemp+=temp1;strtemp+=",";

        if(roll<0)strtemp+="-";
        if((fabs)(roll)<10)strtemp+='0'; //roll横滚角rrr.rr(-90 - 90)
        temp1.sprintf("%.2f", (fabs)(roll));
        strtemp+=temp1;strtemp+=",";

        temp1.sprintf("%d", QF);	//QF解状态
        strtemp+=temp1;strtemp+=",";
        temp1.sprintf("%d", sat_No);//卫星数
        strtemp+=temp1;strtemp+=",";

        if(age<10)strtemp+='0';
        temp1.sprintf("%.2f", age);//差分延迟
        strtemp+=temp1;strtemp+=",";
        temp1.sprintf("%d", stn_ID);	//基站号
        strtemp+=temp1;
        strtemp+=check;strtemp+=end;//chek是定值？？？？？？？？？？

    }
}