#include <stdio.h>
#include "NTPTime.h"
#include <Timezone.h>

/**
 * Input time in epoch format and return tm time format
 * by Renzo Mischianti <www.mischianti.org> 
 */
tm NTPTime::getDateTimeByParams(long time){
    struct tm *newtime;
    const time_t tim = time;
    newtime = localtime(&tim);
    return *newtime;
}
/**
 * Input tm time format and return String with format pattern
 * by Renzo Mischianti <www.mischianti.org>
 */
String NTPTime::getDateTimeStringByParams(tm *newtime, const char* pattern){
    char buffer[30];
    strftime(buffer, 30, pattern, newtime);
    return buffer;
}
 
/**
 * Input time in epoch format format and return String with format pattern
 * by Renzo Mischianti <www.mischianti.org> 
 */
String NTPTime::getEpochStringByParams(long time, const char* pattern) {
    tm newtime;
    newtime = getDateTimeByParams(time);
    return getDateTimeStringByParams(&newtime, pattern);
}

String NTPTime::getCurrentDateTimeFormatted() {
  // US Eastern Timezone
  TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  // Eastern Daylight Time = UTC - 4 hours
  TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   // Eastern Standard Time = UTC - 5 hours
  // Timezone usET = Timezone(usEDT, usEST);
  // return getEpochStringByParams(usET.toLocal(now()), (const char *) "%Y/%m/%d %H:%M:%S");
  return getEpochStringByParams(
      Timezone(usEDT, usEST).toLocal(now()),
      (const char *) "%Y/%m/%d %H:%M:%S"
  );
}
