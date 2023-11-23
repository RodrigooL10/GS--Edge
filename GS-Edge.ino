#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <MPU6050.h>

#define AO_PIN 34 

#define PULSE_PER_BEAT    1           // Número de pulsos por batimento cardíaco
#define INTERRUPT_PIN     35           // Pino de interrupção
#define SAMPLING_INTERVAL 1000  

volatile uint16_t pulse;              // Variável que será incrementada na interrupção
uint16_t count;                       // Variável para armazenar o valor atual de pulse

float heartRate;                      // Frequência cardíaca calculada a partir de count

const char* default_SSID = "Wokwi-GUEST";
const char* default_PASSWORD = "";
const char* default_BROKER_MQTT = "46.17.108.113";
const int default_BROKER_PORT = 1883;
const char* default_ID_MQTT = "dreamwatch002";

const char* default_TOPICO_SUBSCRIBE = "/TEF/dreamwatch/cmd";
const char* default_TOPICO_PUBLISH_1 = "/TEF/dreamwatch002/attrs";
const char* default_TOPICO_PUBLISH_2 = "/TEF/dreamwatch002/attrs/ax";
const char* default_TOPICO_PUBLISH_3 = "/TEF/dreamwatch002/attrs/ay";
const char* default_TOPICO_PUBLISH_4 = "/TEF/dreamwatch002/attrs/az";
const char* default_TOPICO_PUBLISH_5 = "/TEF/dreamwatch002/attrs/gx";
const char* default_TOPICO_PUBLISH_6 = "/TEF/dreamwatch002/attrs/gy";
const char* default_TOPICO_PUBLISH_7 = "/TEF/dreamwatch002/attrs/gz";
const char* default_TOPICO_PUBLISH_8 = "/TEF/dreamwatch002/attrs/lumi";
const char* default_TOPICO_PUBLISH_9 = "/TEF/dreamwatch002/attrs/pulse";

// Variáveis auxiliares para o LDR
const float GAMMA = 0.7;
const float RL10 = 50;

// Definição de um limiar de aceleração para detectar movimento
const float ACCEL_MOVEMENT_THRESHOLD = 1.5; // Ajuste conforme necessário

// Definição de um limiar de rotação para detectar giro
const float GYRO_MOVEMENT_THRESHOLD = 30.0;

char EstadoSaida = '0';

WiFiClient espClient;
PubSubClient MQTT(espClient);

MPU6050 mpu;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;  // Mutex para garantir acesso seguro a pulse


void IRAM_ATTR HeartRateInterrupt() {
  portENTER_CRITICAL_ISR(&mux);  // Entra em uma seção crítica de interrupção
  pulse++;  // Incrementa a variável pulse de maneira segura
  portEXIT_CRITICAL_ISR(&mux);   // Sai da seção crítica de interrupção
}

void initSerial() {
    Serial.begin(115200);
}

void initWiFi() {
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(default_SSID);
    Serial.println("Aguarde");
    reconectWiFi();
}

void initMQTT() {
    MQTT.setServer(default_BROKER_MQTT, default_BROKER_PORT);
}

void setup() {
    initSerial();
    initWiFi();
    initMQTT();
    Wire.begin();

    mpu.initialize();

    Serial.println("Testing MPU6050 connections...");
    Serial.println(mpu.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

    pinMode(INTERRUPT_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), HeartRateInterrupt, RISING);  // Configura a interrupção no pino

    delay(5000);
    MQTT.publish(default_TOPICO_PUBLISH_1, "s|on");
}

void loop() {
    VerificaConexoesWiFIEMQTT();
    HeartRate();
    MQTT.loop();
}

void reconectWiFi() {
    if (WiFi.status() == WL_CONNECTED)
        return;
    WiFi.begin(default_SSID, default_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Conectado com sucesso na rede ");
    Serial.print(default_SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}


void VerificaConexoesWiFIEMQTT() {
    if (!MQTT.connected())
        reconnectMQTT();
    reconectWiFi();
}


void reconnectMQTT() {
    while (!MQTT.connected()) {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(default_BROKER_MQTT);
        if (MQTT.connect(default_ID_MQTT)) {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(default_TOPICO_SUBSCRIBE);
        } else {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Haverá nova tentativa de conexão em 2s");
            delay(2000);
        }
    }
}

void handleLuminosity() {
    int lightValue = analogRead(AO_PIN);
    lightValue = map(lightValue, 4095, 0, 1024, 0); 
    float voltage = lightValue / 1024.*5;
    float resistance = 2000 * voltage / (1-voltage/5);
    float lux = pow(RL10*1e3*pow(10,GAMMA)/resistance,(1/GAMMA));

    String mensagem = String(lux);
    Serial.print("Valor da luminosidade: ");
    Serial.println(mensagem.c_str());
    MQTT.publish(default_TOPICO_PUBLISH_8, mensagem.c_str());
}

bool detectAccelerationMovement(float accelX, float accelY, float accelZ) {
  // Calcular a magnitude da aceleração
  float magnitude = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

  // Comparar com o limiar de movimento
  return magnitude > ACCEL_MOVEMENT_THRESHOLD;
}

bool detectGyroMovement(float gyroX, float gyroY, float gyroZ) {
  // Calcular a magnitude da rotação
  float magnitude = sqrt(gyroX * gyroX + gyroY * gyroY + gyroZ * gyroZ);

  // Comparar com o limiar de rotação
  return magnitude > GYRO_MOVEMENT_THRESHOLD;
}

void HeartRate() {
    static unsigned long startTime;
    if (millis() - startTime < 1000) return;
    startTime = millis();

    portENTER_CRITICAL(&mux);
    int count = pulse;
    pulse = 0;
    portEXIT_CRITICAL(&mux);

    float heartRate = map(count, 0, 220, 0, 220);
    String mensagem1 = String(heartRate);
    Serial.print("Frequência cardíaca: ");
    Serial.println(mensagem1.c_str());
    MQTT.publish(default_TOPICO_PUBLISH_9, mensagem1.c_str());

    //Quando o sensor detectar pulsação menor que 60bpm, entende-se que o usuário está dormindo
    //Inicia o monitoramento pelos sensores de luminosidade e os acelerômetro e giroscópio
    if (heartRate <= 60) {
          MonitorarSono();
          handleLuminosity();
    }

}

void MonitorarSono(){
  // Ler dados do acelerômetro, giroscópio e temperatura
  int16_t ax, ay, az, gx, gy, gz, temp;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  temp = mpu.getTemperature() / 340.00 + 36.53;  // Fórmula para converter o valor para Celsius

  // Converter dados do acelerômetro para m/s²
  float accel_x_m_s2 = ax / 16384.0 * 9.8;
  float accel_y_m_s2 = ay / 16384.0 * 9.8;
  float accel_z_m_s2 = az / 16384.0 * 9.8;

  // Converter dados do giroscópio para º/s
  float gyro_x_deg_s = gx / 131.0;
  float gyro_y_deg_s = gy / 131.0;
  float gyro_z_deg_s = gz / 131.0;

  // Verificar se há movimento no acelerômetro
  if (detectAccelerationMovement(accel_x_m_s2, accel_y_m_s2, accel_z_m_s2)) {
    Serial.println("Movimento detectado no acelerômetro!");
  }

  // Verificar se há rotação no giroscópio
  if (detectGyroMovement(gyro_x_deg_s, gyro_y_deg_s, gyro_z_deg_s)) {
    Serial.println("Rotação detectada no giroscópio!");
  }

  Serial.println();


    //Enviando os dados para o servidor
    String mensagemax = String(accel_x_m_s2);
    Serial.print("Aceleração (x): ");
    Serial.println(mensagemax.c_str());
    MQTT.publish(default_TOPICO_PUBLISH_2, mensagemax.c_str());

    String mensagemay = String(accel_y_m_s2);
    Serial.print("Aceleração (y): ");
    Serial.println(mensagemay.c_str());
    MQTT.publish(default_TOPICO_PUBLISH_3, mensagemay.c_str());

    String mensagemaz = String(accel_z_m_s2);
    Serial.print("Aceleração (z): ");
    Serial.println(mensagemaz.c_str());
    MQTT.publish(default_TOPICO_PUBLISH_4, mensagemaz.c_str());

    String mensagemgx = String(gyro_x_deg_s);
    Serial.print("Giroscópio (x): ");
    Serial.println(mensagemgx.c_str());
    MQTT.publish(default_TOPICO_PUBLISH_5, mensagemgx.c_str());

    String mensagemgy = String(gyro_y_deg_s);
    Serial.print("Giroscópio (y): ");
    Serial.println(mensagemgy.c_str());
    MQTT.publish(default_TOPICO_PUBLISH_6, mensagemgy.c_str());

    String mensagemgz = String(gyro_z_deg_s);
    Serial.print("Giroscópio (z): ");
    Serial.println(mensagemgz.c_str());
    MQTT.publish(default_TOPICO_PUBLISH_7, mensagemgz.c_str());

  delay(1000);
}