from http.server import HTTPServer, SimpleHTTPRequestHandler


class MEEDRequestHandler(SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, OPTIONS")
        self.send_header(
            "Access-Control-Allow-Headers", "X-Requested-With, Content-Type"
        )
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(200, "ok")
        self.end_headers()


if __name__ == "__main__":
    port = 8080
    server_address = ("", port)
    httpd = HTTPServer(server_address, MEEDRequestHandler)
    print(f"Serving on port {port}...")
    httpd.serve_forever()
