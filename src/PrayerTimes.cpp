#include "PrayerTimes.h"
#include <math.h>


static int calcMethod = UmmAlQura;
static int asrMethod = Shafii;



struct MethodConfig
{
    float fajr;
    float isha;
};


static MethodConfig methods[] =
{
    {16.0,14.0},     // Jafari
    {18.0,18.0},     // Karachi
    {15.0,15.0},     // ISNA
    {18.0,17.0},     // MWL
    {18.5,0},        // Makkah
    {19.5,17.5},     // Egypt
    {17.7,14.0},     // Tehran
    {18.5,0}         // Umm AlQura
};



float degToRad(float d)
{
    return d * PI / 180.0;
}


float radToDeg(float r)
{
    return r * 180.0 / PI;
}



int dayOfYear(
    int y,
    int m,
    int d
)
{

    static int days[] =
    {
        0,31,59,90,120,151,
        181,212,243,273,304,334
    };


    int n =
        days[m-1]+d;


    if(
        m>2 &&
        ((y%4==0 && y%100!=0)||y%400==0)
    )
        n++;


    return n;

}



void setCalcMethod(int method)
{
    calcMethod=method;
}



void setAsrMethod(int method)
{
    asrMethod=method;
}




float sunTime(
    float angle,
    float lat,
    float decl
)
{

    float cosH =
    (
        -sin(degToRad(angle))
        -
        sin(degToRad(lat))
        *
        sin(degToRad(decl))
    )
    /
    (
        cos(degToRad(lat))
        *
        cos(degToRad(decl))
    );


    if(cosH>1)
        cosH=1;

    if(cosH<-1)
        cosH=-1;


    return radToDeg(
        acos(cosH)
    )/15.0;

}





void getPrayerTimes(

int year,
int month,
int day,

float latitude,
float longitude,
float timezone,

float result[]

)
{

    int n =
    dayOfYear(
    year,
    month,
    day
    );

    float julianDate = (367 * year) - floor((7.0/4.0) * (year + floor((month + 9.0)/12.0))) + floor((275.0 * month)/9.0) + day - 730531.5;
    float L = fmod(280.461 + 0.9856474 * julianDate, 360.0);
    float g = fmod(357.528 + 0.9856003 * julianDate, 360.0);
    float lambda = L + 1.915 * sin(degToRad(g)) + 0.020 * sin(degToRad(2*g));
    float obliquity = 23.439 - 0.0000004 * julianDate;
    float alpha = radToDeg(atan2(cos(degToRad(obliquity)) * sin(degToRad(lambda)), cos(degToRad(lambda))));
    float equationOfTime = (L - alpha) / 15.0;
    float decl = radToDeg(asin(sin(degToRad(obliquity)) * sin(degToRad(lambda))));
    
    float noon = 12 - (longitude / 15.0) - equationOfTime + timezone;


MethodConfig cfg =
methods[calcMethod];



// Fajr

result[0] =
noon -
sunTime(
cfg.fajr,
latitude,
decl
);




// Sunrise

result[1]=
noon -
sunTime(
0.833,
latitude,
decl
);




// Dhuhr

result[2]=
noon;




// Asr


float shadow =
(
asrMethod==Hanafi
)
?2:1;



float angle =
radToDeg(
atan(
1/
(
shadow+
tan(
fabs(
degToRad(latitude-decl)
)
)
)
)
);



result[3]=
noon+
sunTime(
-angle,
latitude,
decl
);




// Maghrib

result[4]=
noon+
sunTime(
0.833,
latitude,
decl
);




// Isha

if(cfg.isha>0)
{

result[5]=
noon+
sunTime(
cfg.isha,
latitude,
decl
);

}
else
{

result[5]=
result[4]+1.5;

}


}




void getHourMinute(
float value,
int &hour,
int &minute
)
{

hour=(int)value;


minute=
round(
(value-hour)*60
);



if(minute>=60)
{
hour++;
minute-=60;
}


while(hour>=24)
hour-=24;


while(hour<0)
hour+=24;

}