
#ifdef DEVKITC
    #include <Arduino.h>
#else
    #include <M5Unified.h>
#endif
#include <esp_task_wdt.h>

#include <stdlib.h>
#include <SPI.h>

#include "SD.h"
#include "math.h"
#include "Esp.h"
#include "protomap.hpp"
#include "slippytiles.hpp"

static demInfo_t *di;

int32_t dem_setup(fs::FS& fs, const String &dirName) {
    int32_t n_dems = 0;
    int rc;
    double lat,lon,ref;
    locInfo_t li = {};
    TIMESTAMP(now);

    File root = fs.open(dirName.c_str(), "r");
    if(!root) {
        log_e("%s: failed to open directory", dirName.c_str());
        return -1;
    }
    if(!root.isDirectory()) {
        log_e("%s: not a directory", dirName.c_str());
        return -1;
    }
    File file = root.openNextFile();
    while(file) {

        if (!file.isDirectory()) {
            String fn(file.name());
            if (fn.startsWith("._")) {
                file = root.openNextFile();
                continue;
            }
            if (!fn.endsWith(".pmtiles")) {
                file = root.openNextFile();
                continue;
            }
            String path =  SD.mountpoint() + dirName + "/" + fn;
            log_i("adding DEM %s \t(%d bytes)",path.c_str(), file.size());

            rc = addDEM(path.c_str(), &di);
            if (rc != 0) {
                log_e("addDEM fail: %d\n", rc);
            } else {
                log_i("%s: zoom %d..%d resolution: %.2fm/pixel coverage %.2f/%.2f..%.2f/%.2f type %s",
                      di->path, di->header.min_zoom, di->header.max_zoom,
                      meters_per_pixel(di),
                      min_lat(di), min_lon(di),
                      max_lat(di), max_lon(di),
                      tileType(di->header.tile_type));
                n_dems++;
            }
        }
        file = root.openNextFile();
    }

    return n_dems;

#ifdef DEM_TEST
    lat = 47.12925176802318;
    lon = 15.209778656353123;
    ref = 865.799987792969;

    STARTTIME(now);
    rc = getLocInfo(lat, lon, &li);
    log_i("8113 Stiwoll Kehrer:  %d %d %.2f %.2f - %lu uS cold", rc, li.status, li.elevation, ref,  LAPTIME(now));
    li = {};

    STARTTIME(now);
    rc = getLocInfo(lat, lon, &li);
    log_i("8113 Stiwoll Kehrer:  %d %d %.2f %.2f - %lu uS cached", rc, li.status, li.elevation, ref,  LAPTIME(now));

    li = {};
    lat = 48.2383409011934;
    lon = 16.299522929921253;
    ref = 333.0;
    rc = getLocInfo(lat, lon, &li);
    log_i("1180 Utopiaweg 1:  %d %d %.2f %.2f", rc, li.status, li.elevation, ref);

    li = {};
    lat = 48.2610837936095;
    lon = 16.289583084029545;
    ref = 403.6;
    rc = getLocInfo(lat, lon, &li);
    log_i("1190 Höhenstraße:  %d %d %.2f %.2f", rc, li.status, li.elevation, ref);

    li = {};
    lat = 48.208694143314325;
    lon =16.37255104738311;
    ref = 171.4;
    rc = getLocInfo(lat, lon, &li);
    log_i("1010 Stephansplatz:   %d %d %.2f %.2f", rc, li.status, li.elevation, ref);

    li = {};
    lat = 48.225003606677504;
    lon = 16.44120643847108;
    ref = 158.6;
    rc = getLocInfo(lat, lon, &li);
    log_i("1220 Industriestraße 81:  %d %d %.2f %.2f\n", rc, li.status, li.elevation, ref);

    log_i("cached tiles:");
    printCache();

    log_i("DEMS available:");
    printDems();

    log_i("free heap: %lu", ESP.getFreeHeap());
    log_i("used psram: %lu", ESP.getPsramSize() - ESP.getFreePsram());
#endif
}

void dem_loop(void) {

}
