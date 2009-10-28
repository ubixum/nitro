/**
 * Copyright (C) 2009 Ubixum, Inc. 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **/


#ifdef WIN32
#include <windows.h>
#endif

#include <cstdio> // NULL

#include "hr_time.h"



#ifdef WIN32
double LIToSecs( LARGE_INTEGER & L, LARGE_INTEGER & frequency) {
	return ((double)L.QuadPart /(double)frequency.QuadPart);
}

CStopWatch::CStopWatch(){
	timer.start.QuadPart=0;
	timer.stop.QuadPart=0;	
	QueryPerformanceFrequency( &frequency );
}

void CStopWatch::startTimer( ) {
    QueryPerformanceCounter(&timer.start);
}

void CStopWatch::stopTimer( ) {
    QueryPerformanceCounter(&timer.stop);
}


double CStopWatch::getElapsedTime() {
	LARGE_INTEGER time;
	time.QuadPart = timer.stop.QuadPart - timer.start.QuadPart;
    return LIToSecs( time, frequency) ;
}


void nitro_sleep(uint32 usecs) {
    LARGE_INTEGER time;
    LARGE_INTEGER freq;
    LARGE_INTEGER step;
    LARGE_INTEGER elapsed;
    
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&time);
    
    do {
        QueryPerformanceCounter(&step);
        elapsed.QuadPart = step.QuadPart - time.QuadPart ;
    } while ( LIToSecs ( elapsed, freq ) * 1000000 < usecs );

}


#else

void CStopWatch::startTimer( ) {
	gettimeofday(&(timer.start),NULL);
}

void CStopWatch::stopTimer( ) {
	gettimeofday(&(timer.stop),NULL);
}

double CStopWatch::getElapsedTime() {	
	timeval res;
	timersub(&(timer.stop),&(timer.start),&res);
	return res.tv_sec + res.tv_usec/1000000.0; // 10^6 uSec per second
}




void nitro_sleep(uint32 usecs) {
    timespec ts = { 0 , usecs*1000 };
    nanosleep ( &ts, NULL);
}

#endif
