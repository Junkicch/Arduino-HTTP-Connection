#pragma once
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10, 100, 0, 50);        // livre na faixa do roteador
IPAddress gateway(10, 100, 0, 1);    // gateway do roteador
IPAddress subnet(255, 255, 0, 0);
IPAddress serverIp(10, 100, 0, 145); // IP que seu notebook recebeu
#define SERVER_PORT 8080            // porta que vamos abrir no PC