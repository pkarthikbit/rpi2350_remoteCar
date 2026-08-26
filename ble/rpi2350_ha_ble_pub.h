#ifndef _RPI2350_HA_BLE_PUB_H
#define _RPI2350_HA_BLE_PUB_H

extern int rpi2350_ha_ble_st;
extern char rpi2350_ha_ble_ssid[33];
extern char rpi2350_ha_ble_password[64];

extern void rpi2350_ha_ble_init();
extern void rpi2350_ha_ble_10ms();

#endif  /* _RPI2350_HA_BLE_PUB_H */