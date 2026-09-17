/*******************************************************************************
** File: generic_radio_device.h
**
** Purpose:
**   This is the header file for the GENERIC_RADIO device.
**
*******************************************************************************/
#ifndef _GENERIC_RADIO_DEVICE_H_
#define _GENERIC_RADIO_DEVICE_H_

/*
** Required header files.
*/
#include "device_cfg.h"
#include "generic_radio_platform_cfg.h"
#include "hwlib.h"

/*
** Type definitions
*/
#define GENERIC_RADIO_DEVICE_HDR_0 0xDE
#define GENERIC_RADIO_DEVICE_HDR_1 0xAD

#define GENERIC_RADIO_DEVICE_REQ_HK_CMD 0x00
#define GENERIC_RADIO_DEVICE_CFG_CMD    0x01
#define IRIS_RADIO_DEVICE_SET_POWER_CMD 0x02
#define IRIS_RADIO_DEVICE_REQ_POWER_HK_CMD 0x03
#define IRIS_RADIO_MAX_POWER_MW 100U
#define IRIS_RADIO_POWER_HK_SIZE 20U

#define GENERIC_RADIO_DEVICE_TRAILER_0 0xBE
#define GENERIC_RADIO_DEVICE_TRAILER_1 0xEF

#define GENERIC_RADIO_DEVICE_HDR_TRL_LEN 4
#define GENERIC_RADIO_DEVICE_CMD_SIZE    9

/*
** GENERIC_RADIO device housekeeping telemetry definition
*/
typedef struct
{
    uint32_t DeviceCounter;
    uint32_t DeviceConfig;
    uint32_t ProxSignal;

} __attribute__((packed)) GENERIC_RADIO_Device_HK_tlm_t;
#define GENERIC_RADIO_DEVICE_HK_LNGTH sizeof(GENERIC_RADIO_Device_HK_tlm_t)
#define GENERIC_RADIO_DEVICE_HK_SIZE  GENERIC_RADIO_DEVICE_HK_LNGTH + GENERIC_RADIO_DEVICE_HDR_TRL_LEN

/*
** Prototypes
*/
/* Host-side decoded data; wire fields are explicitly decoded, not memcpy'd. */
typedef struct
{
    uint32_t DeviceCounter;
    uint32_t DeviceConfig;
    uint32_t ProxSignal;
    uint32_t TransmitPowerMilliwatts;
} IRIS_RADIO_PowerHK_t;

/* Success means the datagram was sent, not that the device accepted it. */
int32_t IRIS_RADIO_SetTransmitPower(socket_info_t *device, uint32_t milliwatts);
/* On failure, leaves *data unchanged. Serialize with other device HK requests. */
int32_t IRIS_RADIO_RequestPowerHK(socket_info_t *device, IRIS_RADIO_PowerHK_t *data);

int32_t GENERIC_RADIO_SetConfiguration(socket_info_t *device, uint32_t config);
int32_t GENERIC_RADIO_ProximityForward(socket_info_t *device, uint16_t scid, uint8_t *data, uint16_t data_len);
int32_t GENERIC_RADIO_RequestHK(socket_info_t *device, GENERIC_RADIO_Device_HK_tlm_t *data);

#endif /* _GENERIC_RADIO_DEVICE_H_ */
