#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <Arduino.h>

bool api_fetch_prayer_times();


// Local (network-free) Hijri date computation - Kuwaiti algorithm
void compute_local_hijri();


String api_get_hijri_date();
String api_get_hijri_month();


String api_get_hijri_weekday();


String api_get_last_error();


// Non-blocking diagnostic test

void api_request_test(const String &url = "");


void api_process_test();


String api_get_test_result();


int api_raw_get(const String &url);

#endif
