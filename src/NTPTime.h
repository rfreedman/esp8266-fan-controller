/*
NTPTime.h

Sync System Time with NTP Server, Get Current Time Formatted
*/

#ifndef NTP_TIME_H
#define NTP_TIME_H

#include <NTPClient.h>
#include <time.h>
#include <Timezone.h> // https://github.com/JChristensen/Timezone

class NTPTime {
        
        private:


        public:
            static const long utcOffsetInSeconds = 3600;

            static tm getDateTimeByParams(long time);

            static String getDateTimeStringByParams(tm *newtime, const char* pattern);
 
            static String getEpochStringByParams(long time, const char* pattern);

            static String getCurrentDateTimeFormatted();
};

#endif // NTP_TIME_H
