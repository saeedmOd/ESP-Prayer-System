#ifndef PRAYER_TIMES_H
#define PRAYER_TIMES_H


#include <Arduino.h>


// =================================
// Prayer Times Calculator
// =================================

class PrayerTimes
{

public:


    // =================================
    // Constructor
    // =================================

    PrayerTimes();


    // =================================
    // حساب مواقيت الصلاة
    // =================================

    void calculate();


    // =================================
    // الحصول على أوقات الصلاة
    // =================================

    String getFajr();

    String getDhuhr();

    String getAsr();

    String getMaghrib();

    String getIsha();



private:


    // =================================
    // تخزين أوقات الصلاة
    // =================================

    String fajr;

    String dhuhr;

    String asr;

    String maghrib;

    String isha;


};


#endif