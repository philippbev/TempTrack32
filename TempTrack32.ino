#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <time.h>

// Projektname: TempTrack32
#define PROJECT_NAME "TempTrack32"

// DHT11 Sensor Setup
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// WiFi Access Point
const char* ssid = "TempTrack32";
const char* password = "PhilippIstToll!";

WebServer server(80);

// Zeitverwaltung
unsigned long customEpoch = 0;
unsigned long deviceStartMillis = 0;

// Struktur für Messwerte
struct SensorData {
  float temperature;
  float humidity;
  unsigned long timestamp; // UNIX-Zeit
};

const int MAX_MEASUREMENTS = 4320; // 30 Tage * 24h * 6 Messungen/h
SensorData dataLog[MAX_MEASUREMENTS];
int dataIndex = 0;

// 10 Minuten Intervall
const unsigned long MEASURE_INTERVAL = 10 * 60 * 1000UL;
unsigned long lastMeasureTime = 0;

// Zeitformat
String formatDateTime(unsigned long ts) {
  time_t t = ts;
  struct tm *timeinfo = gmtime(&t);
  char buffer[25];
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
           timeinfo->tm_year + 1900,
           timeinfo->tm_mon + 1,
           timeinfo->tm_mday,
           timeinfo->tm_hour,
           timeinfo->tm_min,
           timeinfo->tm_sec);
  return String(buffer);
}

// Setup
void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.softAP(ssid, password);
  Serial.println("Access Point started: " + WiFi.softAPIP().toString());

  deviceStartMillis = millis();

  server.on("/", handleRoot);
  server.on("/measure", handleManualMeasure);
  server.on("/settime", handleSetTime);
  server.on("/data.csv", handleDownloadCSV);

  server.begin();
  Serial.println("Webserver running.");
  lastMeasureTime = millis() - MEASURE_INTERVAL;
}

void loop() {
  server.handleClient();

  if (millis() - lastMeasureTime >= MEASURE_INTERVAL) {
    lastMeasureTime = millis();
    logSensorData();
  }
}

void logSensorData() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println("Sensor error!");
    return;
  }

  unsigned long ts = customEpoch + (millis() - deviceStartMillis) / 1000;

  dataLog[dataIndex] = { t, h, ts };
  dataIndex = (dataIndex + 1) % MAX_MEASUREMENTS;

  Serial.print("Measurement saved: ");
  Serial.print(t); Serial.print(" °C, ");
  Serial.print(h); Serial.print(" %, Time: ");
  Serial.println(formatDateTime(ts));
}

void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html><html><head><meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>TempTrack32</title>
    <style>
      body { font-family: Arial; background: #f0f2f5; margin: 0; padding: 20px; color: #333; }
      h1 { color: #2c3e50; }
      .card { background: white; padding: 20px; margin-bottom: 20px; border-radius: 8px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
      .button { background: #3498db; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; }
      .button:hover { background: #2980b9; }
      input[type="text"] { padding: 8px; width: 250px; border: 1px solid #ccc; border-radius: 4px; }
      a { color: #3498db; text-decoration: none; }
    </style>
    </head><body>
    <h1>TempTrack32 - ESP Climate Logger</h1>

    <div class="card">
      <form action="/settime" method="GET">
        <label>Set Date and Time (YYYY-MM-DD HH:MM:SS):</label><br><br>
        <input type="text" name="datetime" value="2025-06-20 17:30:00">
        <input type="submit" value="Set Time" class="button">
      </form>
    </div>

    <div class="card">
      <form action="/measure" method="POST">
        <input type="submit" value="Start Manual Measurement" class="button">
      </form>
    </div>

    <div class="card">
      <a href="/data.csv" class="button">Download CSV</a>
    </div>

    <div class="card">
      <h2>Today's Summary</h2>
  )rawliteral";

  // Statistik
  float sumT = 0, sumH = 0;
  float minT = 999, maxT = -999, minH = 999, maxH = -999;
  int count = 0;
  unsigned long today = (customEpoch + (millis() - deviceStartMillis) / 1000) / 86400;

  for (int i = 0; i < MAX_MEASUREMENTS; i++) {
    SensorData& d = dataLog[i];
    if (d.timestamp == 0) continue;
    if (d.timestamp / 86400 == today) {
      sumT += d.temperature;
      sumH += d.humidity;
      if (d.temperature < minT) minT = d.temperature;
      if (d.temperature > maxT) maxT = d.temperature;
      if (d.humidity < minH) minH = d.humidity;
      if (d.humidity > maxH) maxH = d.humidity;
      count++;
    }
  }

  if (count > 0) {
    html += "Avg Temp: " + String(sumT / count, 1) + " °C<br>";
    html += "Avg Humidity: " + String(sumH / count, 1) + " %<br>";
    html += "Min Temp: " + String(minT, 1) + " °C, Max Temp: " + String(maxT, 1) + " °C<br>";
    html += "Min Humidity: " + String(minH, 1) + " %, Max Humidity: " + String(maxH, 1) + " %<br>";
  } else {
    html += "No measurements for today.<br>";
  }

  html += R"rawliteral(
    </div>
  </body></html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleManualMeasure() {
  logSensorData();
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSetTime() {
  if (server.hasArg("datetime")) {
    String datetime = server.arg("datetime"); // e.g., "2025-06-20 17:30:00"
    struct tm tm;
    if (sscanf(datetime.c_str(), "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon,
               &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec) == 6) {
      tm.tm_year -= 1900;
      tm.tm_mon -= 1;
      time_t t = mktime(&tm);
      if (t != -1) {
        customEpoch = t;
        deviceStartMillis = millis();
        Serial.print("Time set: ");
        Serial.println(formatDateTime(t));
      }
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleDownloadCSV() {
  String csv = "Date,Time,Temperature_C,Humidity_percent\n";
  for (int i = 0; i < MAX_MEASUREMENTS; i++) {
    SensorData& d = dataLog[(dataIndex + i) % MAX_MEASUREMENTS];
    if (d.timestamp == 0) continue;

    String datetime = formatDateTime(d.timestamp);
    int split = datetime.indexOf(' ');
    String date = datetime.substring(0, split);
    String time = datetime.substring(split + 1);

    csv += date + "," + time + "," + String(d.temperature, 1) + "," + String(d.humidity, 1) + "\n";
  }
  server.sendHeader("Content-Disposition", "attachment; filename=\"TempTrack32_data.csv\"");
  server.send(200, "text/csv", csv);
}