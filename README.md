# Cancela Automática com Reconhecimento Facial - UniFECAF

Este repositório contém o firmware de controle para uma cancela automatizada baseada em Arduino. O sistema foi projetado para atuar em conjunto com um software de reconhecimento facial, gerenciando a abertura física da cancela e garantindo o fechamento seguro após a passagem do usuário.

## ⚙️ Arquitetura e Funcionamento

A inteligência do sistema é dividida em duas partes: o processamento visual (software externo) e o controle eletromecânico (Arduino). 

1. **Validação (Reconhecimento Facial):** Um sistema externo responsável pela visão computacional reconhece o rosto autorizado e envia o caractere `1` via comunicação Serial para o Arduino.
2. **Abertura (0° para 90°):** Ao receber o comando, o Arduino aciona o driver TB6612FNG, e o motor de passo NEMA 17 ergue a haste da cancela em exatos 90 graus.
3. **Monitoramento de Passagem:** Com a cancela aberta, o sensor ultrassônico começa a medir a distância continuamente. 
4. **Fechamento Automático:** Assim que o usuário passa pela cancela (detectado a uma distância <= 15cm) e aguarda um breve *delay* de segurança, o motor executa o movimento reverso (-90 graus), retornando à posição 0° exata e desligando as bobinas para economizar energia.

## 🛠️ Hardware Utilizado

* Placa: **Arduino Nano**
* Atuador: **Motor de Passo NEMA 17**
* Controlador do Motor: **Driver TB6612FNG**
* Sensor de Presença: **Módulo Ultrassônico HC-SR04**

## 📌 Pinagem (Mapeamento)

| Componente | Pino no Arduino | Função |
| :--- | :--- | :--- |
| **Driver TB6612FNG** | D5 | AIN1 (Fase A) |
| | D4 | AIN2 (Fase A) |
| | D7 | BIN1 (Fase B) |
| | D8 | BIN2 (Fase B) |
| | D3 | PWMA (Controle de Potência A) |
| | D11 | PWMB (Controle de Potência B) |
| | D6 | STBY (Standby / Economia de Energia) |
| **Sensor HC-SR04** | D9 | TRIG (Emissor) |
| | D10 | ECHO (Receptor) |

## 🚀 Inicialização e Cuidados

* **Calibração Física:** Como motores de passo não possuem feedback de posição absoluta, **a haste da cancela deve estar obrigatoriamente posicionada na horizontal (0°)** antes de o Arduino ser energizado.
* **Teste Manual:** É possível testar o acionamento da placa abrindo o Monitor Serial (baud rate `9600`) e enviando o número `1` manualmente, simulando o gatilho do sistema de reconhecimento facial.

```C++
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
```
