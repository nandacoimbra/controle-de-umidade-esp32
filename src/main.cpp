#include <WiFi.h>
#include <WebServer.h>
#include "secrets.h"  

// definição do pino do sensor de umidade
const int pinoSensorUmidade = 34;
// Servidor HTTP na porta 80
WebServer server(80);
// Vetor para histórico de leituras
#define MAX_HISTORICO 20
int historico[MAX_HISTORICO];
int indexHistorico = 0;



// Função que retorna leitura atual no sensor de umidade de solo (4095 = totalmente seco, 0 = molhado)
int leituraUmidade()
{
  return analogRead(pinoSensorUmidade);
}

void handleUmidade() {
  int umidade = leituraUmidade();

  // adiciona ao histórico de leituras
  historico[indexHistorico] = umidade;
  indexHistorico = (indexHistorico + 1) % MAX_HISTORICO;

  String json = "{\"umidade\":" + String(umidade) + "}";
  server.send(200, "application/json", json);
}

void setup()
{

  Serial.begin(115200);
  pinMode(pinoSensorUmidade, INPUT);

  // Conectar ao Wi-Fi, usando as credenciais do arquivo secrets.h
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    // Aguarda 500 ms e tenta novamente
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  //busca o IP local
  Serial.print("Conectado! IP: ");
  Serial.println(WiFi.localIP());


  server.on("/umidade", handleUmidade);
  server.begin();
  Serial.println("Servidor iniciado");
}

void loop()
{
  server.handleClient();
}
