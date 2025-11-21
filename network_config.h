#pragma once
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(, , , );        // ip livre na faixa do roteador, para ip fixo
IPAddress gateway(, , , );    // gateway do roteador
IPAddress subnet(255, 255, 0, 0);
IPAddress serverIp(, , , ); // IP do seu servidor

#define SERVER_PORT 8080            // porta que vamos abrir no PC
