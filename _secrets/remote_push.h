#pragma once

// Endpoint voor kwartierpiek notificaties
#define REMOTE_PUSH_URL "http://192.168.0.245:2586/alert"

// Optioneel: API key
// #define REMOTE_PUSH_API_KEY "JOUW_API_KEY_HIER"

// Drempel in Watt (3500 = 3.5 kW)
#define REMOTE_PUSH_THRESHOLD_W 3500

// Testmodus (1 = aan, 0 = uit)
#define REMOTE_PUSH_TEST 1
