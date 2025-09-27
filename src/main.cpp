#include <WiFi.h>
#include <WebServer.h>
#include "secrets.h"  



// definição do pino do sensor de umidade
const int pinoSensorUmidade = 34;

// Servidor HTTP na porta 80
WebServer server(80);

// Função que retorna leitura atual no sensor de umidade de solo (4095 = seco, 0 = molhado)
int leituraUmidade()
{
  return analogRead(pinoSensorUmidade);
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

  server.begin();
  Serial.println("Servidor iniciado");
}

void loop()
{
  server.handleClient();
}
