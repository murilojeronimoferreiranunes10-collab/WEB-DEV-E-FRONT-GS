#include <Servo.h>

// === PINOS ===
#define TRIG 6
#define ECHO 7
#define LDR A0
#define LM35 A2
#define MQ135 A1
#define BUZZER A3
#define RED_PIN 9
#define GREEN_PIN 10
#define BLUE_PIN 11
#define SERVO_PIN 3

Servo servoMotor;

// === VARIÁVEIS GLOBAIS ===
unsigned long startTime;
int ldrMin = 1023;
int ldrMax = 0;
bool calibrated = false;

void setup() {
  Serial.begin(9600);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(LDR, INPUT);
  pinMode(LM35, INPUT);
  pinMode(MQ135, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  
  servoMotor.attach(SERVO_PIN);

  Serial.println("=== SMARTDESK 2050 (Simulacao Tinkercad) ===");
  Serial.println("Calibrando sensor de luz (6 segundos)...");
  startTime = millis();
}

float medirDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  float duracao = pulseIn(ECHO, HIGH);
  float distancia = duracao * 0.034 / 2;
  if (distancia <= 0 || distancia > 400) distancia = 0;
  return distancia;
}

void setRGB(int r, int g, int b) {
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}

int lerLuzPorcentagem() {
  int raw = analogRead(LDR);

  if (!calibrated) {
    if (raw < ldrMin) ldrMin = raw;
    if (raw > ldrMax) ldrMax = raw;
    if (millis() - startTime > 6000) {
      calibrated = true;
      if (ldrMax == ldrMin) ldrMax = ldrMin + 1;
      Serial.print("Calibracao concluida. Min=");
      Serial.print(ldrMin);
      Serial.print("  Max=");
      Serial.println(ldrMax);
    }
  }

  int pct = map(raw, ldrMin, ldrMax, 0, 100);
  pct = constrain(pct, 0, 100);

  // Agrupa em 0, 50 ou 100
  if (pct < 30) pct = 0;
  else if (pct < 70) pct = 50;
  else pct = 100;

  return pct;
}

float lerQualidadeAr() {
  int valorMQ = analogRead(MQ135);
  return valorMQ; // leitura bruta simulada
}

float simularUmidade(int luzPct) {
  // Simula umidade: escuro = mais úmido, claro = menos úmido
  if (luzPct == 0) return 70.0;
  else if (luzPct == 50) return 50.0;
  else return 30.0;
}

void loop() {
  float distancia = medirDistancia();
  int luzPct = lerLuzPorcentagem();

  float temperatura = (analogRead(LM35) * 5.0 * 100.0) / 1023.0; // temperatura LM35
  float umidade = simularUmidade(luzPct); // simula umidade
  float qualidadeAr = lerQualidadeAr();   // MQ135

  // Servo — abre se alguém perto (< 20 cm)
  if (distancia > 0 && distancia < 20) servoMotor.write(90);
  else servoMotor.write(0);

  // RGB e buzzer baseado na temperatura
  if (temperatura > 30) {
    setRGB(255, 0, 0);  // quente
    digitalWrite(BUZZER, HIGH);
  } else if (luzPct == 0) {
    setRGB(0, 0, 255);  // escuro
    digitalWrite(BUZZER, LOW);
  } else if (luzPct == 50) {
    setRGB(255, 255, 0); // média luz
    digitalWrite(BUZZER, LOW);
  } else {
    setRGB(0, 255, 0);  // luz alta
    digitalWrite(BUZZER, LOW);
  }

  // Serial monitor
  Serial.print("Dist: "); Serial.print(distancia); Serial.print(" cm  |  ");
  Serial.print("Temp: "); Serial.print(temperatura, 1); Serial.print(" C  |  ");
  Serial.print("Umid: "); Serial.print(umidade, 1); Serial.print("%  |  ");
  Serial.print("Luz: "); Serial.print(luzPct); Serial.print("%  |  ");
  Serial.print("Qualidade Ar: "); Serial.println(qualidadeAr);

  delay(1000);
}
