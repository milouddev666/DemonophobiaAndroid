import os
from http.server import BaseHTTPRequestHandler, HTTPServer

APK = "/app/Demonophobia-debug-armv7.apk"

class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path in ("/health", "/healthz"):
            self.send_response(200)
            self.end_headers()
            self.wfile.write(b"ok")
            return
        if self.path in ("/", "/index.html"):
            body = (
                "<!doctype html><html><head><meta name='viewport' content='width=device-width'>"
                "<title>Demonophobia Android build</title></head><body>"
                "<h2>Demonophobia Android prototype</h2>"
                "<p>The debug APK was built successfully and includes ARMv7 native code.</p>"
                "<p><a href='/Demonophobia-debug-armv7.apk'>Download APK</a></p>"
                "</body></html>"
            ).encode()
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        if self.path == "/Demonophobia-debug-armv7.apk":
            try:
                size = os.path.getsize(APK)
                self.send_response(200)
                self.send_header("Content-Type", "application/vnd.android.package-archive")
                self.send_header("Content-Disposition", 'attachment; filename="Demonophobia-debug-armv7.apk"')
                self.send_header("Content-Length", str(size))
                self.end_headers()
                with open(APK, "rb") as f:
                    while chunk := f.read(1024 * 1024):
                        self.wfile.write(chunk)
            except OSError:
                self.send_error(404, "APK not found")
            return
        self.send_error(404)

HTTPServer(("0.0.0.0", int(os.environ.get("PORT", "10000"))), Handler).serve_forever()
