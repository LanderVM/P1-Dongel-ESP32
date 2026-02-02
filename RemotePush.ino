#ifdef REMOTE_PUSH

#include "_secrets/remote_push.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>

#ifndef REMOTE_PUSH_URL
#define REMOTE_PUSH_URL ""
#endif

#ifndef REMOTE_PUSH_THRESHOLD_W
#define REMOTE_PUSH_THRESHOLD_W 3500
#endif

#ifndef REMOTE_PUSH_TEST
#define REMOTE_PUSH_TEST 0
#endif

// Eén melding per nieuwe hogere kwartierpiek boven de drempel
static float rp_lastReportedW = 0.0f;
static uint32_t rp_lastPushTs = 0; // epoch seconden

void CheckRemotePeakPush()
{
  static bool firstRun = true;
  static uint32_t rp_lastCheckTs = 0;

  uint32_t now_s = actT;

  // --- rate limit: max 1x per 10 seconden ---
  if ((now_s - rp_lastCheckTs) < 10)
    return;
  rp_lastCheckTs = now_s;
  // -----------------------------------------

  if (firstRun)
  {
    DebugTln(F("RemotePush: initialized"));
#if REMOTE_PUSH_TEST
    DebugTln(F("RemotePush: TEST MODE ENABLED"));
#endif
    firstRun = false;
  }

  if (skipNetwork)
    return;

  float current_w = 0.0f;

#if REMOTE_PUSH_TEST
  // random tussen 2000 en 6000 W
  current_w = 2000 + random(0, 4001); // 2000..6000 W
  Debugf("RemotePush TEST current_w: %.0f W\n", current_w);
#else
  if (!DSMRdata.peak_pwr_last_q_present)
    return;

  current_w = DSMRdata.peak_pwr_last_q.val() * 1000.0f; // P1 data is in kW? omrekenen naar W
  if (current_w <= 0.0f)
    return;
#endif

  const float threshold = (float)REMOTE_PUSH_THRESHOLD_W;

  // alleen doorgaan bij piek boven threshold
  if (current_w < threshold)
  {
    Debugf("RemotePush TEST: %.0f W below threshold %.0f W\n", current_w, threshold);
    return;
  }

  // spam protectie: alleen hogere piek of na 10 minuten
  if (current_w <= rp_lastReportedW + 0.01f &&
      (now_s - rp_lastPushTs) < 600)
  {
    DebugTln(F("RemotePush: spam protection active"));
    return;
  }

  if (!strlen(REMOTE_PUSH_URL))
    return;

  if (!WiFi.isConnected())
    return;

  HTTPClient http;
  if (!http.begin(wifiClient, REMOTE_PUSH_URL))
    return;

#ifdef REMOTE_PUSH_API_KEY
  if (strlen(REMOTE_PUSH_API_KEY))
    http.addHeader("X-API-Key", REMOTE_PUSH_API_KEY);
#endif

  http.addHeader("Content-Type", "text/plain");
  http.addHeader("Priority", "high");
  http.addHeader("Tags", "electricity,peak");
  http.addHeader("Title", "Elektricteit");

  String payload = "Current peak power: " + String(current_w / 1000, 3) + " kW";

  int code = http.POST(payload);
  DebugT(F("RemotePush HTTP code: "));
  Debugln(code);

  http.end();

  rp_lastReportedW = current_w;
  rp_lastPushTs = now_s;
}

#else

// Dummy zodat calls altijd geldig blijven
void CheckRemotePeakPush()
{
  static bool logged = false;
  if (!logged)
  {
    DebugTln(F("RemotePush: disabled"));
    logged = true;
  }
}

#endif
