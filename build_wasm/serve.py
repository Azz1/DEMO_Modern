#!/usr/bin/env python3
import http.server, socketserver, mimetypes, sys

# Ensure .wasm gets the correct MIME type
mimetypes.add_type('application/wasm', '.wasm')

PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 8080

class Handler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        # Allow cross-origin (needed for SharedArrayBuffer in some browsers)
        self.send_header('Cross-Origin-Opener-Policy',  'same-origin')
        self.send_header('Cross-Origin-Embedder-Policy','require-corp')
        super().end_headers()
    def log_message(self, fmt, *args):
        pass  # suppress access log noise

print(f"Serving http://192.168.111.85:{PORT}/demo.html")
with socketserver.TCPServer(("", PORT), Handler) as httpd:
    httpd.serve_forever()
