#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>   // UDP para NTP
#include "network_config.h"

EthernetClient client;
EthernetUDP  Udp;

const unsigned long POST_INTERVAL = 5000; // 5 s
unsigned long lastPost = 0;

int valorLido = 0;
bool novoValor = false;

/* ---------- NTP ---------- */
const char* ntpServer = "pool.ntp.org";
const int  ntpPort    = 123;
const long gmtOffset  = -3;      // GMT-3 (Brasil)
const int  daylight   = 0;       // 1 se houver horário de verão

unsigned long unixTime  = 0;     // último timestamp válido
unsigned long lastNTP   = 0;     // controle de renovação
const unsigned long NTP_INTERVAL = 3600000UL; // 1 h

/* ---------- protótipos ---------- */
void solicitarNTP();
void atualizarHora();

void setup() {
  Serial.begin(9600);
  delay(200);
  Serial.println("Arduino pronto. Digite um número e tecle ENTER.");

  SPI.begin();
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  Ethernet.init(10);

  if (Ethernet.begin(mac) == 0) {
    Serial.println("DHCP falhou – usando estático");
    Ethernet.begin(mac, ip, gateway, subnet);
  }
  Serial.print("Meu IP: ");
  Serial.println(Ethernet.localIP());

  Udp.begin(8888);          // porta local UDP
  solicitarNTP();           // primeira solicitação
}

void loop() {
  /* ---------- 1) ler valor ---------- */
  if (Serial.available()) {
    valorLido = Serial.parseInt();
    if (Serial.read() == '\n') {
      novoValor = true;
      Serial.print("Valor recebido: ");
      Serial.println(valorLido);
    }
    while (Serial.available()) Serial.read();
  }

  /* ---------- 2) manter hora atualizada ---------- */
  if (millis() - lastNTP >= NTP_INTERVAL) solicitarNTP();
  atualizarHora();          // atualiza unixTime

  /* ---------- 3) enviar POST com timestamp ---------- */
  if (novoValor && (millis() - lastPost >= POST_INTERVAL)) {
    novoValor = false;
    lastPost = millis();

    // monta corpo com valor e timestamp
    String corpo = "valor=" + String(valorLido) + "&timestamp=" + String(unixTime);

    if (client.connect(serverIp, SERVER_PORT)) {
      client.println("POST /update HTTP/1.1");
      client.print("Host: ");
      client.println(serverIp);
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.print("Content-Length: ");
      client.println(corpo.length());
      client.println("Connection: close");
      client.println();
      client.println(corpo);

      Serial.print("POST enviado – unixTime = ");
      Serial.println(unixTime);
    } else {
      Serial.println("Falha ao conectar – servidor rodando?");
    }
  }

  /* ---------- 4) descarta resposta e fecha ---------- */
  while (client.available()) Serial.write(client.read());
  if (!client.connected()) client.stop();
}

/* ---------- NTP ---------- */
void solicitarNTP() {
  Udp.beginPacket(ntpServer, ntpPort);
  byte buf[48] = {0};
  buf[0] = 0b11100011;   // LI, VN, Mode
  Udp.write(buf, 48);
  Udp.endPacket();
  lastNTP = millis();
}

void atualizarHora() {
  if (Udp.parsePacket() == 48) {
    byte buf[48];
    Udp.read(buf, 48);
    unsigned long highWord = word(buf[40], buf[41]);
    unsigned long lowWord  = word(buf[42], buf[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    unixTime = secsSince1900 - 2208988800UL; // converte p/ UNIX
  }
  // se ainda não chegou resposta, mantém unixTime = 0
}
