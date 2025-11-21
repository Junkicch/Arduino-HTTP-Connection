#!/usr/bin/env python3
"""Grava valores via GET/POST e mostra no navegador."""
from http.server import BaseHTTPRequestHandler, HTTPServer
import urllib.parse as urlparse
import time

HOST = ('0.0.0.0', 8080)

last_value = None
history = []          # lista (horário, valor)

class Handler(BaseHTTPRequestHandler):
    def log_message(self, fmt, *args): pass   # silencia log

    # ---------- POST ----------
    def do_POST(self):
        cl = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(cl).decode()
        self._trata_valor(body)
        self.send_response(200)
        self.end_headers()
        self.wfile.write(b'OK\n')

    # ---------- GET ----------
    def do_GET(self):
        parsed = urlparse.urlparse(self.path)
        self._trata_valor(parsed.query)
        self._envia_html()

    # ---------- lógica comum ----------
    def _trata_valor(self, texto):
        global last_value, history
        params = urlparse.parse_qs(texto)
        if 'valor' in params:
            try:
                v = float(params['valor'][0])
                last_value = v
                history.append((time.strftime('%H:%M:%S'), v))
                if len(history) > 50:
                    history.pop(0)
                print(f"[{history[-1][0]}]  valor = {v}")
            except ValueError:
                pass

    # ---------- página HTML ----------
    def _envia_html(self):
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
    {''.join(f'<tr><td>{t}</td><td>{v}</td></tr>' for t, v in reversed(history))}
  </table>
</body>
</html>'''
        self.wfile.write(html.encode())

if __name__ == '__main__':
    print(f'Servidor rodando em http://{HOST[0]}:{HOST[1]}')
    HTTPServer(HOST, Handler).serve_forever()
