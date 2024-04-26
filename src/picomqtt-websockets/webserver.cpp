#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFiMulti.h>
#include <LittleFS.h>

#include <FS.h>
#include <LittleFS.h>

static WiFiMulti wifiMulti;
static WebServer *http_server;
static MDNSResponder *mdns_responder;

void webserver_loop(void) {
    if (http_server)
        http_server->handleClient();
}

String getContentType(String filename) {
    if (http_server->hasArg("download")) {
        return "application/octet-stream";
    } else if (filename.endsWith(".htm")) {
        return "text/html";
    } else if (filename.endsWith(".html")) {
        return "text/html";
    } else if (filename.endsWith(".css")) {
        return "text/css";
    } else if (filename.endsWith(".js")) {
        return "application/javascript";
    } else if (filename.endsWith(".png")) {
        return "image/png";
    } else if (filename.endsWith(".gif")) {
        return "image/gif";
    } else if (filename.endsWith(".jpg")) {
        return "image/jpeg";
    } else if (filename.endsWith(".ico")) {
        return "image/x-icon";
    } else if (filename.endsWith(".xml")) {
        return "text/xml";
    } else if (filename.endsWith(".pdf")) {
        return "application/x-pdf";
    } else if (filename.endsWith(".zip")) {
        return "application/x-zip";
    } else if (filename.endsWith(".gz")) {
        return "application/x-gzip";
    }
    return "text/plain";
}

bool exists(String path) {
    bool yes = false;
    File file = LittleFS.open(path, "r");
    if(!file.isDirectory() && (file.size() > 0)) {
        yes = true;
    }
    file.close();
    log_i("exists: %s  %d", path.c_str(), yes);
    return yes;
}


bool handleFileRead(String path) {
    log_i("handleFileRead: '%s'", path.c_str());
    if (path.endsWith("/")) {
        path += "index.html";
    }
    String contentType = getContentType(path);
    String pathWithGz = path + ".gz";
    if (exists(pathWithGz) || exists(path)) {
        if (exists(pathWithGz)) {
            path += ".gz";
        }
        File file = LittleFS.open(path, "r");
        log_i("handleFileRead using '%s' size %u", path.c_str(), file.size());

        http_server->streamFile(file, contentType);
        file.close();
        return true;
    }
    return false;
}

void webserver_setup(void) {

    log_i("mounting LittleFS");
    LittleFS.begin(false,"/littlefs", 10);

    log_i("Connecting to WiFi");
    WiFi.mode(WIFI_STA);
    WiFi.enableProv(true);

    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
#if defined(WIFI_SSID11)
    wifiMulti.addAP(WIFI_SSID1, WIFI_PASSWORD1);
#endif
#if defined(WIFI_SSID2)
    wifiMulti.addAP(WIFI_SSID2, WIFI_PASSWORD2);
#endif
#if defined(WIFI_SSID3)
    wifiMulti.addAP(WIFI_SSID3, WIFI_PASSWORD3);
#endif
#if defined(WIFI_SSID4)
    wifiMulti.addAP(WIFI_SSID4, WIFI_PASSWORD4);
#endif
    wifiMulti.run();
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
    }
    log_i("WiFi connected, IP: %s", WiFi.localIP().toString().c_str());

    http_server = new WebServer(HTTP_PORT);
    http_server->onNotFound([]() {
        if (!handleFileRead( // String("/littlefs") +
                    http_server->uri())) {
            http_server->send(404, "text/plain", "FileNotFound");
        }
    });
    http_server->begin();
    log_i("HTTP server started");

    mdns_responder = new MDNSResponder();
    if (mdns_responder->begin("broker")) {
        log_i("MDNS responder started");
    }
    mdns_responder->addService("mqtt", "tcp", MQTT_TCP);
    mdns_responder->addService("mqtt-ws", "tcp", MQTT_WS);
    mdns_responder->addService("http", "tcp", HTTP_PORT);
}
