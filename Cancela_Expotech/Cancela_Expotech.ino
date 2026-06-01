#include <Stepper.h>

// Motor 28BYJ-48 com ULN2003
const int passosPorVolta = 200;

// Pinos do motor na ordem solicitada: 8, 10, 9, 11
const int pino1 = 8;
const int pino2 = 10;
const int pino3 = 9;
const int pino4 = 11;

Stepper motor(passosPorVolta, pino1, pino2, pino3, pino4);

// 90 graus = 1/4 de volta
const int passos90Graus = passosPorVolta / 4;

// Sensor ultrassônico
const int trigPin = 6;
const int echoPin = 7;

// Distância para considerar que alguém passou pela catraca
const int distanciaDeteccao = 15; // em cm

bool catracaAberta = false;

void setup() {
  Serial.begin(9600);

  motor.setSpeed(35);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Garante que o motor comece desligado
  desligarMotor();

  Serial.println("Sistema iniciado.");
  Serial.println("Digite 1 para abrir a catraca.");
}

void loop() {
  // Verifica comando recebido pelo Serial
  if (Serial.available() > 0) {
    char comando = Serial.read();

    if (comando == '1' && !catracaAberta) {
      abrirCatraca();
    }
  }

  // Se a catraca estiver aberta, monitora o ultrassônico
  if (catracaAberta) {
    int distancia = medirDistancia();

    Serial.print("Distancia: ");
    Serial.print(distancia);
    Serial.println(" cm");

    if (distancia > 0 && distancia <= distanciaDeteccao) {
      Serial.println("Movimento detectado. Fechando catraca...");
      delay(1000); // pequena espera para a pessoa terminar de passar
      fecharCatraca();
    }

    delay(200);
  }
}

void abrirCatraca() {
  Serial.println("Abrindo catraca...");

  motor.step(-passos90Graus);
  desligarMotor(); // Desliga a ponte H após o movimento

  catracaAberta = true;

  Serial.println("Catraca aberta.");
}

void fecharCatraca() {
  Serial.println("Fechando catraca...");

  motor.step(passos90Graus);
  desligarMotor(); // Desliga a ponte H após o movimento

  catracaAberta = false;

  Serial.println("Catraca fechada.");
  Serial.println("Digite 1 para abrir novamente.");
}

// Função criada para cortar a energia das bobinas do motor
void desligarMotor() {
  digitalWrite(pino1, LOW);
  digitalWrite(pino2, LOW);
  digitalWrite(pino3, LOW);
  digitalWrite(pino4, LOW);
  Serial.println("Bobinas do motor desligadas (Ponte H em repouso).");
}

int medirDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracao = pulseIn(echoPin, HIGH, 30000);

  if (duracao == 0) {
    return -1; // nenhuma leitura válida
  }

  int distancia = duracao * 0.034 / 2;

  return distancia;
}