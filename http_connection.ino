#include <SPI.h>
#include <Ethernet.h>
#include "network_config.h"

EthernetClient client;

const unsigned long POST_INTERVAL = 5000; // 5 s entre envios
unsigned long lastPost = 0;

int valorLido = 0;
bool novoValor = false;

void setup() {
  Serial.begin(9600);
  delay(200);
  Serial.println("Arduino pronto. Digite um número e tecle ENTER.");

  SPI.begin();
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  Ethernet.init(10);

  // DHCP já testado – podemos usar estático ou DHCP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("DHCP falhou – usando estático");
    Ethernet.begin(mac, ip, gateway, subnet);
  }

  Serial.print("Meu IP: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  /* ---------- 1) ler valor da Serial ---------- */
  if (Serial.available()) {
    valorLido = Serial.parseInt();
    if (Serial.read() == '\n') {          // consome ENTER
      novoValor = true;
      Serial.print("Valor recebido: ");
      Serial.println(valorLido);
    }
    // limpa possíveis caracteres que sobrem
    while (Serial.available()) Serial.read();
  }

  /* ---------- 2) enviar POST ---------- */
  if (novoValor && (millis() - lastPost >= POST_INTERVAL)) {
    novoValor = false;
    lastPost  = millis();

    String corpo = "valor=" + String(valorLido);

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

      Serial.println("POST enviado.");
    } else {
      Serial.println("Falha ao conectar – servidor rodando?");
    }
  }

  /* ---------- 3) ler resposta e fechar ---------- */
  while (client.available()) {
    char c = client.read();
    // opcional: mostra o que chegou
    // Serial.write(c);
  }

  // se servidor fechou, libera o socket
  if (!client.connected()) {
    client.stop();
  }
}