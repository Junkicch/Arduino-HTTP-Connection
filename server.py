#!/usr/bin/env python3
"""Servidor simples: grava valores enviados pelo Arduinoe exibe no navegador em tempo real."""
from http.server import BaseHTTPRequestHandler, HTTPServer
import urllib.parse as urlparse
import time

HOST = ('0.0.0.0', 8080) 

# ------ armazenamento em memória ------
last_value = None
history = []          # lista de tuplas (timestamp, valor)
class Handler(BaseHTTPRequestHandler):
    def log_message(self, fmt, *args):   # silencia logs
        pass
    # ------ rota raiz: página HTML ------
    def do_GET(self):
        global last_value, history
        parsed = urlparse.urlparse(self.path)
        path = parsed.path
        params = urlparse.parse_qs(parsed.query)
        # ----- /update?valor=123 (Arduino) -----
        if path == '/update':
            if 'valor' in params:
                try:
                    v = float(params['valor'][0])
                    last_value = v
                    history.append((time.strftime('%H:%M:%S'), v))
                    # mantém apenas os 50 últimos
                    if len(history) > 50:
                        history.pop(0)
                    print(f"[{history[-1][0]}]  valor = {v}")
                except ValueError:
                    pass
            # responde rápido ao Arduino
            self.send_response(200)
            self.send_header('Content-type', 'text/plain')
            self.end_headers()
            self.wfile.write(b'OK\n')
        # ----- / (página para o navegador) -----
        elif path == '/':
            self.send_response(200)
            self.send_header('Content-type', 'text/html; charset=utf-8')
            self.end_headers()
            html = f'''<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>Dados do Arduino</title>
  <meta http-equiv="refresh" content="3">
  <style>
    body{{font-family:Arial;margin:40px;}}
    .big{{font-size:48px;color:#0a0;}}
    table{{border-collapse:collapse;margin-top:20px;}}
    th,td{{padding:8px 12px;border:1px solid #ccc;}}
  </style>
</head>
<body>
  <h1>Último valor recebido</h1>
  <p class="big">{last_value if last_value is not None else '---'}</p>
  <h2>Histórico (50 últimos)</h2>
  <table>
    <tr><th>Horário</th><th>Valor</th></tr>
    {''.join(f'<tr><td>{t}</td><td>{v}</td></tr>' for t,v in reversed(history))}
  </table>
</body>
</html>'''
            self.wfile.write(html.encode())
        # ----- qualquer outra rota -----
        else:
            self.send_response(404)
            self.end_headers()
if __name__ == '__main__':
    print(f'Servidor rodando em http://{HOST[0]}:{HOST[1]}')
    HTTPServer(HOST, Handler).serve_forever()
