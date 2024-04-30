#ifndef ESPNOW_H
#define ESPNOW_H

extern void espnow_setup(void);
extern void espnow_send(bool ok, float pressure,
                        float v_baro, float a_baro,
                        double altVar, double varioVar);

#endif