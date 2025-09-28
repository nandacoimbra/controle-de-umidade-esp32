#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "secrets.h"

// Pino do sensor de umidade
const int pinoSensorUmidade = 34;

// Servidor HTTP
WebServer server(80);

// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -10800); // GMT-3

// Histórico em memória
#define MAX_HISTORICO 50
int historicoMem[MAX_HISTORICO];
String timestampsMem[MAX_HISTORICO];
int indexHistorico = 0;

// Inicializa SPIFFS e cria arquivo de histórico se não existir
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Erro ao montar SPIFFS");
  } else {
    if (!SPIFFS.exists("/historico.txt")) {
      File file = SPIFFS.open("/historico.txt", "w");
      if (file) file.close();
    }
  }
}

// Obter timestamp completo: YYYY-MM-DD HH:MM:SS
String getTimestamp() {
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);
  char buffer[20];
  sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
          ptm->tm_year + 1900,
          ptm->tm_mon + 1,
          ptm->tm_mday,
          ptm->tm_hour,
          ptm->tm_min,
          ptm->tm_sec);
  return String(buffer);
}

// Leitura atual do sensor
int leituraUmidade() {
  return analogRead(pinoSensorUmidade);
}

// Salva leitura com timestamp no SPIFFS
void salvarHistorico(int umidade, String timestamp) {
  historicoMem[indexHistorico] = umidade;
  timestampsMem[indexHistorico] = timestamp;
  indexHistorico = (indexHistorico + 1) % MAX_HISTORICO;

  // Salva no SPIFFS (memória flash)
  File file = SPIFFS.open("/historico.txt", "a");
  if (file) {
    file.println("{\"umidade\":" + String(umidade) + ",\"timestamp\":\"" + timestamp + "\"}");
    file.close();
    Serial.println("Leitura salva: " + String(umidade) + " em " + timestamp);
  } else {
    Serial.println("Erro ao salvar histórico");
  }
}

// Handler para /umidade
void handleUmidade() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  int umidade = leituraUmidade();
  String timestamp = getTimestamp();
  salvarHistorico(umidade, timestamp);
  String json = "{\"umidade\":" + String(umidade) + ",\"timestamp\":\"" + timestamp + "\"}";
  server.send(200, "application/json", json);
}

// Handler para /historico
void handleHistorico() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  String json = "[";
  File file = SPIFFS.open("/historico.txt", "r");
  if (file) {
    bool first = true;
    while (file.available()) {
      String line = file.readStringUntil('\n');
      if (!first) json += ",";
      json += line;
      first = false;
    }
    file.close();
  }
  json += "]";
  server.send(200, "application/json", json);
}

// Handler para limpar histórico
void handleLimpar() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  
  // Limpa buffer circular
  for (int i = 0; i < MAX_HISTORICO; i++) {
    historicoMem[i] = 0;
    timestampsMem[i] = "";
  }
  indexHistorico = 0;

  // Limpa arquivo SPIFFS
  if (SPIFFS.exists("/historico.txt")) {
    SPIFFS.remove("/historico.txt");
    File file = SPIFFS.open("/historico.txt", "w"); // recria arquivo vazio
    if (file) file.close();
  }

  server.send(200, "application/json", "{\"status\":\"Histórico limpo\"}");
}

void setup() {
  Serial.begin(115200);
  pinMode(pinoSensorUmidade, INPUT);

  // Conectar Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado! IP: ");
  Serial.println(WiFi.localIP());

  // NTP
  timeClient.begin();
  timeClient.update();

  // mDNS
  if (!MDNS.begin("esp32")) Serial.println("Erro MDNS");
  else Serial.println("mDNS iniciado: esp32.local");

  // SPIFFS
  initSPIFFS();

  // Endpoints
  server.on("/umidade", handleUmidade);
  server.on("/historico", handleHistorico);
  server.on("/limpar", HTTP_POST, handleLimpar);

  server.begin();
  Serial.println("Servidor iniciado");
}

void loop() {
  server.handleClient();
  timeClient.update();
}
