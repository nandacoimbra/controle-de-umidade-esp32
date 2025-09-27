#include <Arduino.h>

// definição do pino do sensor de umidade
const int pinoSensorUmidade = 4;

void setup()
{
  // inicialização da comunicação serial
  Serial.begin(9600);
  // configuração do pino do sensor de umidade como entrada
  pinMode(pinoSensorUmidade, INPUT);
}

void loop()
{
  Serial.println(analogRead(pinoSensorUmidade));
  delay(1000); // aguarda 1 segundo antes da próxima leitura
}
