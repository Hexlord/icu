/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-1999, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/

#include "tzbdtest.h"
#include <math.h>
#include "unicode/timezone.h"
#include "unicode/simpletz.h"
#include "unicode/gregocal.h"

void TimeZoneBoundaryTest::runIndexedTest( int32_t index, UBool exec, char* &name, char* par )
{
    if (exec) logln("TestSuite TestTimeZoneBoundary");
    switch (index) {
        case 0:
            name = "TestBoundaries";
            if (exec) {
                logln("TestBoundaries---"); logln("");
                TestBoundaries();
            }
            break;
        case 1:
            name = "TestNewRules";
            if (exec) {
                logln("TestNewRules---"); logln("");
                TestNewRules();
            }
            break;
        case 2:
            name = "TestStepwise";
            if (exec) {
                logln("TestStepwise---"); logln("");
                TestStepwise();
            }
            break;
        default: name = ""; break;
    }
}

// *****************************************************************************
// class TimeZoneBoundaryTest
// *****************************************************************************
 
UDate TimeZoneBoundaryTest::ONE_SECOND = 1000;
 
UDate TimeZoneBoundaryTest::ONE_MINUTE = 60 * ONE_SECOND;
 
UDate TimeZoneBoundaryTest::ONE_HOUR = 60 * ONE_MINUTE;
 
UDate TimeZoneBoundaryTest::ONE_DAY = 24 * ONE_HOUR;
 
UDate TimeZoneBoundaryTest::ONE_YEAR = uprv_floor(365.25 * ONE_DAY);
 
UDate TimeZoneBoundaryTest::SIX_MONTHS = ONE_YEAR / 2;
 
int32_t TimeZoneBoundaryTest::MONTH_LENGTH[] = {
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
 
UDate TimeZoneBoundaryTest::PST_1997_BEG = 860320800000.0;
 
UDate TimeZoneBoundaryTest::PST_1997_END = 877856400000.0;
 
UDate TimeZoneBoundaryTest::INTERVAL = 10;
 
// -------------------------------------
 
void
TimeZoneBoundaryTest::findDaylightBoundaryUsingDate(UDate d, const char* startMode, UDate expectedBoundary)
{
    UnicodeString str;
    if (dateToString(d, str).indexOf(startMode) == - 1) {
        logln(UnicodeString("Error: ") + startMode + " not present in " + str);
    }
    UDate min = d;
    UDate max = min + SIX_MONTHS;
    while ((max - min) > INTERVAL) {
        UDate mid = (min + max) / 2;
        UnicodeString* s = &dateToString(mid, str);
        if (s->indexOf(startMode) != - 1) {
            min = mid;
        }
        else {
            max = mid;
        }
    }
    logln("Date Before: " + showDate(min));
    logln("Date After:  " + showDate(max));
    UDate mindelta = expectedBoundary - min;
    UDate maxdelta = max - expectedBoundary;
    if (mindelta >= 0 &&
        mindelta <= INTERVAL &&
        mindelta >= 0 &&
        mindelta <= INTERVAL) logln(UnicodeString("PASS: Expected boundary at ") + expectedBoundary);
    else errln(UnicodeString("FAIL: Expected boundary at ") + expectedBoundary);
}
 
// -------------------------------------
 
void
TimeZoneBoundaryTest::findDaylightBoundaryUsingTimeZone(UDate d, UBool startsInDST, UDate expectedBoundary)
{
    TimeZone *zone = TimeZone::createDefault();
    findDaylightBoundaryUsingTimeZone(d, startsInDST, expectedBoundary, zone);
    delete zone;
}
 
// -------------------------------------
 
void
TimeZoneBoundaryTest::findDaylightBoundaryUsingTimeZone(UDate d, UBool startsInDST, UDate expectedBoundary, TimeZone* tz)
{
    UErrorCode status = U_ZERO_ERROR;
    UnicodeString str;
    UDate min = d;
    UDate max = min + SIX_MONTHS;
    if (tz->inDaylightTime(d, status) != startsInDST) {
        errln("FAIL: " + tz->getID(str) + " inDaylightTime(" + dateToString(d) + ") != " + (startsInDST ? "true" : "false"));
        startsInDST = !startsInDST;
    }
    if (failure(status, "TimeZone::inDaylightTime")) return;
    if (tz->inDaylightTime(max, status) == startsInDST) {
        errln("FAIL: " + tz->getID(str) + " inDaylightTime(" + dateToString(max) + ") != " + (startsInDST ? "false" : "true"));
        return;
    }
    if (failure(status, "TimeZone::inDaylightTime")) return;
    while ((max - min) > INTERVAL) {
        UDate mid = (min + max) / 2;
        UBool isIn = tz->inDaylightTime(mid, status);
        if (failure(status, "TimeZone::inDaylightTime")) return;
        if (isIn == startsInDST) {
            min = mid;
        }
        else {
            max = mid;
        }
    }
    logln(tz->getID(str) + " Before: " + showDate(min));
    logln(tz->getID(str) + " After:  " + showDate(max));
    UDate mindelta = expectedBoundary - min;
    UDate maxdelta = max - expectedBoundary;
    if (mindelta >= 0 &&
        mindelta <= INTERVAL &&
        mindelta >= 0 &&
        mindelta <= INTERVAL) logln(UnicodeString("PASS: Expected boundary at ") + expectedBoundary);
    else errln(UnicodeString("FAIL: Expected boundary at ") + expectedBoundary);
}
 
// -------------------------------------
/*
UnicodeString*
TimeZoneBoundaryTest::showDate(int32_t l)
{
    return showDate(new Date(l));
}
*/
// -------------------------------------
 
UnicodeString
TimeZoneBoundaryTest::showDate(UDate d)
{
    int32_t y, m, day, h, min, sec;
    dateToFields(d, y, m, day, h, min, sec);
    return UnicodeString("") + y + "/" + showNN(m + 1) + "/" +
        showNN(day) + " " + showNN(h) + ":" + showNN(min) +
        " \"" + dateToString(d) + "\" = " + uprv_floor(d+0.5);
}
 
// -------------------------------------
 
UnicodeString
TimeZoneBoundaryTest::showNN(int32_t n)
{
    return ((n < 10) ? UnicodeString("0"): UnicodeString("")) + n;
}
 
// -------------------------------------
 
void
TimeZoneBoundaryTest::verifyDST(UDate d, TimeZone* time_zone, UBool expUseDaylightTime, UBool expInDaylightTime, UDate expZoneOffset, UDate expDSTOffset)
{
    UnicodeString str;
    UErrorCode status = U_ZERO_ERROR;
    logln("-- Verifying time " + dateToString(d) + " in zone " + time_zone->getID(str));
    if (time_zone->inDaylightTime(d, status) == expInDaylightTime)
        logln(UnicodeString("PASS: inDaylightTime = ") + (time_zone->inDaylightTime(d, status)?"true":"false"));
    else errln(UnicodeString("FAIL: inDaylightTime = ") + (time_zone->inDaylightTime(d, status)?"true":"false"));
    if (failure(status, "TimeZone::inDaylightTime")) return;
    if (time_zone->useDaylightTime() == expUseDaylightTime)
        logln(UnicodeString("PASS: useDaylightTime = ") + (time_zone->useDaylightTime()?"true":"false"));
    else errln(UnicodeString("FAIL: useDaylightTime = ") + (time_zone->useDaylightTime()?"true":"false"));
    if (time_zone->getRawOffset() == expZoneOffset) logln(UnicodeString("PASS: getRawOffset() = ") + (expZoneOffset / ONE_HOUR));
    else errln(UnicodeString("FAIL: getRawOffset() = ") + (time_zone->getRawOffset() / ONE_HOUR) + "; expected " + (expZoneOffset / ONE_HOUR));
    GregorianCalendar *gc = new GregorianCalendar(time_zone->clone(), status);
    gc->setTime(d, status);
    if (failure(status, "GregorianCalendar::setTime")) return;
    int32_t offset = time_zone->getOffset((uint8_t)gc->get(gc->ERA, status),
        gc->get(gc->YEAR, status), gc->get(gc->MONTH, status),
        gc->get(gc->DAY_OF_MONTH, status), (uint8_t)gc->get(gc->DAY_OF_WEEK, status),
        ((gc->get(gc->HOUR_OF_DAY, status) * 60 + gc->get(gc->MINUTE, status)) * 60 + gc->get(gc->SECOND, status)) * 1000 + gc->get(gc->MILLISECOND, status));
    if (failure(status, "GregorianCalendar::get")) return;
    if (offset == expDSTOffset) logln(UnicodeString("PASS: getOffset() = ") + (offset / ONE_HOUR));
    else errln(UnicodeString("FAIL: getOffset() = ") + (offset / ONE_HOUR) + "; expected " + (expDSTOffset / ONE_HOUR));
    delete gc;
}
 
// -------------------------------------
 
/**
 * Test the behavior of SimpleTimeZone at the transition into and out of DST.
 * Use a binary search to find boundaries.
 */
void
TimeZoneBoundaryTest::TestBoundaries()
{
    if (TRUE) {
        logln("--- Test a ---");
        UDate d = date(97, Calendar::APRIL, 6);
        TimeZone *z = TimeZone::createTimeZone("PST");
        for (int32_t i = 60; i <= 180; i += 15) {
            UBool inDST = (i >= 120);
            UDate e = d + i * 60 * 1000;
            verifyDST(e, z, TRUE, inDST, - 8 * ONE_HOUR, inDST ? - 7 * ONE_HOUR: - 8 * ONE_HOUR);
        }
        delete z;
    }
    if (TRUE) {
        logln("--- Test b ---");
        TimeZone *tz;
        TimeZone::setDefault(*(tz = TimeZone::createTimeZone("PST")));
        delete tz;
        logln("========================================");
        findDaylightBoundaryUsingDate(date(97, 0, 1), "PST", PST_1997_BEG);
        logln("========================================");
        findDaylightBoundaryUsingDate(date(97, 6, 1), "PDT", PST_1997_END);
    }
    if (TRUE) {
        logln("--- Test c ---");
        logln("========================================");
        TimeZone* z = TimeZone::createTimeZone("Australia/Adelaide");
        findDaylightBoundaryUsingTimeZone(date(97, 0, 1), TRUE, 859653000000.0, z);
        logln("========================================");
        findDaylightBoundaryUsingTimeZone(date(97, 6, 1), FALSE, 877797000000.0, z);
        delete z;
    }
    if (TRUE) {
        logln("--- Test d ---");
        logln("========================================");
        findDaylightBoundaryUsingTimeZone(date(97, 0, 1), FALSE, PST_1997_BEG);
        logln("========================================");
        findDaylightBoundaryUsingTimeZone(date(97, 6, 1), TRUE, PST_1997_END);
    }
    if (FALSE) {
        logln("--- Test e ---");
        TimeZone *z = TimeZone::createDefault();
        logln(UnicodeString("") + z->getOffset(1, 97, 3, 4, 6, 0) + " " + date(97, 3, 4));
        logln(UnicodeString("") + z->getOffset(1, 97, 3, 5, 7, 0) + " " + date(97, 3, 5));
        logln(UnicodeString("") + z->getOffset(1, 97, 3, 6, 1, 0) + " " + date(97, 3, 6));
        logln(UnicodeString("") + z->getOffset(1, 97, 3, 7, 2, 0) + " " + date(97, 3, 7));
        delete z;
    }
}
 
// -------------------------------------
 
void
TimeZoneBoundaryTest::testUsingBinarySearch(SimpleTimeZone* tz, UDate d, UDate expectedBoundary)
{
    UErrorCode status = U_ZERO_ERROR;
    UDate min = d;
    UDate max = min + SIX_MONTHS;
    UBool startsInDST = tz->inDaylightTime(d, status);
    if (failure(status, "SimpleTimeZone::inDaylightTime")) return;
    if (tz->inDaylightTime(max, status) == startsInDST) {
        logln("Error: inDaylightTime(" + dateToString(max) + ") != " + ((!startsInDST)?"true":"false"));
    }
    if (failure(status, "SimpleTimeZone::inDaylightTime")) return;
    while ((max - min) > INTERVAL) {
        UDate mid = (min + max) / 2;
        if (tz->inDaylightTime(mid, status) == startsInDST) {
            min = mid;
        }
        else {
            max = mid;
        }
        if (failure(status, "SimpleTimeZone::inDaylightTime")) return;
    }
    logln("Binary Search Before: " + showDate(min));
    logln("Binary Search After:  " + showDate(max));
    UDate mindelta = expectedBoundary - min;
    UDate maxdelta = max - expectedBoundary;
    if (mindelta >= 0 &&
        mindelta <= INTERVAL &&
        mindelta >= 0 &&
        mindelta <= INTERVAL) logln(UnicodeString("PASS: Expected boundary at ") + expectedBoundary);
    else errln(UnicodeString("FAIL: Expected boundary at ") + expectedBoundary);
}
 
// -------------------------------------
 
/**
 * Test the handling of the "new" rules; that is, rules other than nth Day of week.
 */
void
TimeZoneBoundaryTest::TestNewRules()
{
    UErrorCode status = U_ZERO_ERROR;
    if (TRUE) {
        SimpleTimeZone *tz;
        logln("-----------------------------------------------------------------");
        logln("Aug 2ndTues .. Mar 15");
        tz = new SimpleTimeZone(- 8 * (int32_t)ONE_HOUR, "Test_1", Calendar::AUGUST, 2, Calendar::TUESDAY, 2 * (int32_t)ONE_HOUR, Calendar::MARCH, 15, 0, 2 * (int32_t)ONE_HOUR, status);
        logln("========================================");
        testUsingBinarySearch(tz, date(97, 0, 1), 858416400000.0);
        logln("========================================");
        testUsingBinarySearch(tz, date(97, 6, 1), 871380000000.0);
        delete tz;
        logln("-----------------------------------------------------------------");
        logln("Apr Wed>=14 .. Sep Sun<=20");
        tz = new SimpleTimeZone(- 8 * (int32_t)ONE_HOUR, "Test_2", Calendar::APRIL, 14, - Calendar::WEDNESDAY, 2 *(int32_t)ONE_HOUR, Calendar::SEPTEMBER, - 20, - Calendar::SUNDAY, 2 * (int32_t)ONE_HOUR, status);
        logln("========================================");
        testUsingBinarySearch(tz, date(97, 0, 1), 861184800000.0);
        logln("========================================");
        testUsingBinarySearch(tz, date(97, 6, 1), 874227600000.0);
        delete tz;
    }
}
 
// -------------------------------------
 
void
TimeZoneBoundaryTest::findBoundariesStepwise(int32_t year, UDate interval, TimeZone* z, int32_t expectedChanges)
{
    UErrorCode status = U_ZERO_ERROR;
    UnicodeString str;
    UDate d = date(year - 1900, Calendar::JANUARY, 1);
    UDate time = d;
    UDate limit = time + ONE_YEAR + ONE_DAY;
    UBool lastState = z->inDaylightTime(d, status);
    if (failure(status, "TimeZone::inDaylightTime")) return;
    int32_t changes = 0;
    logln(UnicodeString("-- Zone ") + z->getID(str) + " starts in " + year + " with DST = " + (lastState?"true":"false"));
    logln(UnicodeString("useDaylightTime = ") + (z->useDaylightTime()?"true":"false"));
    while (time < limit) {
        d = time;
        UBool state = z->inDaylightTime(d, status);
        if (failure(status, "TimeZone::inDaylightTime")) return;
        if (state != lastState) {
            logln(UnicodeString(state ? "Entry ": "Exit ") + "at " + d);
            lastState = state;++changes;
        }
        time += interval;
    }
    if (changes == 0) {
        if (!lastState &&
            !z->useDaylightTime()) logln("No DST");
        else errln("FAIL: DST all year, or no DST with true useDaylightTime");
    }
    else if (changes != 2) {
        errln(UnicodeString("FAIL: ") + changes + " changes seen; should see 0 or 2");
    }
    else if (!z->useDaylightTime()) {
        errln("FAIL: useDaylightTime false but 2 changes seen");
    }
    if (changes != expectedChanges) {
        errln(UnicodeString("FAIL: ") + changes + " changes seen; expected " + expectedChanges);
    }
}
 
// -------------------------------------

/**
 * This test is problematic. It makes assumptions about the behavior
 * of specific zones. Since ICU's zone table is based on the Olson
 * zones (the UNIX zones), and those change from time to time, this
 * test can fail after a zone table update. If that happens, the
 * selected zones need to be updated to have the behavior
 * expected. That is, they should have DST, not have DST, and have DST
 * -- other than that this test isn't picky. 12/3/99 aliu
 *
 * Test the behavior of SimpleTimeZone at the transition into and out of DST.
 * Use a stepwise march to find boundaries.
 */
void
TimeZoneBoundaryTest::TestStepwise()
{
    TimeZone *zone =  TimeZone::createTimeZone("EST");
    findBoundariesStepwise(1997, ONE_DAY, zone, 2);
    delete zone;
    zone = TimeZone::createTimeZone("UTC"); // updated 12/3/99 aliu
    findBoundariesStepwise(1997, ONE_DAY, zone, 0);
    delete zone;
    zone = TimeZone::createTimeZone("Australia/Adelaide");
    findBoundariesStepwise(1997, ONE_DAY, zone, 2);
    delete zone;
}
