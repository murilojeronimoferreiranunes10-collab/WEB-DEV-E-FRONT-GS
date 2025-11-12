#include <Servo.h>

#define LDR A0
#define MQ135 A1
#define TRIG 6
#define ECHO 7
#define LED_R 9
#define LED_G 10
#define LED_B 11
#define BUZZER 12
#define SERVO_PIN 3
#define TEMP_PIN A2 // TMP36

Servo servo;

unsigned long lastMoveTime = 0;
bool userPresent = true;

void setup() {
  Serial.begin(9600);
  pinMode(LDR, INPUT);
  pinMode(MQ135, INPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(TEMP_PIN, INPUT);
  servo.attach(SERVO_PIN);
  servo.write(90); // posição neutra
  Serial.println("=== SmartDesk 2050 Iniciado ===");
}

void loop() {
  // --- Sensores ---
  float temp = lerTemperatura();
  int luz = analogRead(LDR);
  int airQuality = analogRead(MQ135);

  // --- Sensor ultrassônico (presença) ---
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long duration = pulseIn(ECHO, HIGH);
  float distance = duration * 0.034 / 2;

  if (distance < 50 && distance > 0) {
    userPresent = true;
    lastMoveTime = millis();
  } else if (millis() - lastMoveTime > 30000) { // 30s sem presença
    userPresent = false;
  }

  // --- Exibir dados ---
  Serial.print("Temp: "); Serial.print(temp);
  Serial.print("°C | Luz: "); Serial.print(luz);
  Serial.print(" | Ar: "); Serial.print(airQuality);
  Serial.print(" | Presença: "); Serial.println(userPresent ? "Sim" : "Não");

  // --- Condições ambientais ---
  bool ambienteRuim = (temp > 30 || airQuality > 600);
  bool poucaLuz = (luz < 300);

  // --- LED RGB ---
  if (ambienteRuim) setColor(255, 0, 0);       // Vermelho = risco
  else if (poucaLuz) setColor(255, 255, 0);    // Amarelo = pouca luz
  else setColor(0, 255, 0);                    // Verde = ambiente normal

  // --- Servo motor ---
  if (poucaLuz) servo.write(120); // abre persiana / liga luz
  else servo.write(60);           // fecha persiana / luz adequada

  // --- Alertas sonoros ---
  if (ambienteRuim) tone(BUZZER, 1000);
  else if (!userPresent) tone(BUZZER, 700); // lembrete de pausa
  else noTone(BUZZER);

  delay(1000);
}

// --- Função para ler TMP36 ---
float lerTemperatura() {
  int valor = analogRead(TEMP_PIN);
  float tensao = valor * 5.0 / 1023.0;
  float temperatura = (tensao - 0.5) * 100.0; // Conversão TMP36
  return temperatura;
}

// --- Controle do LED RGB ---
void setColor(int r, int g, int b) {
  analogWrite(LED_R, r);
  analogWrite(LED_G, g);
  analogWrite(LED_B, b);
}
