extern "C" {
#include "generic_radio_device.h"
}
#include "iris_radio_power.hpp"
#include <array>
#include <cassert>
#include <cstring>
#include <iostream>
#include <algorithm>

static Nos3::IrisRadioPower simulator_power;
static std::array<uint8_t, 9> sent;
static std::array<uint8_t, 21> response;
static size_t send_size, receive_size;
static int32_t send_status, receive_status;
static unsigned sends, receives, delays;
static bool simulate;

static void reset()
{
    send_size = 9;
    receive_size = 20;
    send_status = receive_status = 0;
    sends = receives = delays = 0;
    simulate = false;
    response = {{0xDE, 0xAD, 0x89, 0xAB, 0xCD, 0xEF,
                 0x12, 0x34, 0x56, 0x78, 0xFE, 0xDC, 0xBA, 0x98,
                 0, 0, 0, 50, 0xBE, 0xEF, 0}};
}

extern "C" int32_t socket_send(socket_info_t *, uint8_t *buffer, size_t size,
                               size_t *bytes, char *ip, int port)
{
    ++sends;
    assert(size == sent.size());
    assert(std::strcmp(ip, GENERIC_RADIO_CFG_DEVICE_IP) == 0);
    assert(port == GENERIC_RADIO_CFG_UDP_FSW_TO_RADIO);
    std::copy(buffer, buffer + size, sent.begin());
    *bytes = send_size;
    if (simulate && send_status == 0 && send_size == size)
    {
        if (buffer[2] == Nos3::IrisRadioPower::SET_POWER_COMMAND)
            assert(simulator_power.set(&buffer[3]));
        else
        {
            assert(buffer[2] == Nos3::IrisRadioPower::REQUEST_POWER_HK_COMMAND);
            simulator_power.extend_housekeeping(response.data());
        }
    }
    return send_status;
}

extern "C" int32_t socket_recv(socket_info_t *, uint8_t *buffer, size_t size, size_t *bytes)
{
    ++receives;
    *bytes = std::min(size, receive_size);
    std::copy(response.begin(), response.begin() + *bytes, buffer);
    return receive_status;
}

extern "C" int32_t OS_TaskDelay(uint32_t milliseconds)
{
    ++delays;
    assert(milliseconds == GENERIC_RADIO_CFG_DEVICE_DELAY_MS);
    return 0;
}

static void expect_bad_reply(socket_info_t *device)
{
    IRIS_RADIO_PowerHK_t data = {11, 22, 33, 44};
    assert(IRIS_RADIO_RequestPowerHK(device, &data) != OS_SUCCESS);
    assert(data.DeviceCounter == 11 && data.DeviceConfig == 22 &&
           data.ProxSignal == 33 && data.TransmitPowerMilliwatts == 44);
}

int main()
{
    socket_info_t device = {};
    IRIS_RADIO_PowerHK_t hk = {};
    const std::array<uint8_t, 9> set50 = {{0xDE, 0xAD, 2, 0, 0, 0, 50, 0xBE, 0xEF}};
    const std::array<uint8_t, 9> request = {{0xDE, 0xAD, 3, 0, 0, 0, 0, 0xBE, 0xEF}};
    reset();
    assert(IRIS_RADIO_SetTransmitPower(&device, 50) == 0 && sent == set50);
    assert(IRIS_RADIO_RequestPowerHK(&device, &hk) == 0 && sent == request);
    assert(hk.DeviceCounter == 0x89ABCDEF && hk.DeviceConfig == 0x12345678);
    assert(hk.ProxSignal == 0xFEDCBA98 && hk.TransmitPowerMilliwatts == 50);
    assert(delays == 1 && receives == 1);

    // Exercise the actual simulator's power encoder with the C driver's decoder.
    for (uint32_t power : {0U, 50U, 100U})
    {
        reset();
        simulate = true;
        assert(IRIS_RADIO_SetTransmitPower(&device, power) == 0);
        assert(IRIS_RADIO_RequestPowerHK(&device, &hk) == 0);
        assert(hk.TransmitPowerMilliwatts == power);
    }
    reset();
    assert(IRIS_RADIO_SetTransmitPower(&device, 101) == OS_ERROR);
    assert(IRIS_RADIO_SetTransmitPower(&device, UINT32_MAX) == OS_ERROR);
    assert(IRIS_RADIO_SetTransmitPower(nullptr, 50) == OS_ERROR);
    assert(IRIS_RADIO_RequestPowerHK(nullptr, &hk) == OS_ERROR);
    assert(IRIS_RADIO_RequestPowerHK(&device, nullptr) == OS_ERROR);
    assert(sends == 0);

    reset(); send_status = -7;
    assert(IRIS_RADIO_SetTransmitPower(&device, 50) == -7);
    assert(IRIS_RADIO_RequestPowerHK(&device, &hk) == -7);
    assert(receives == 0 && delays == 0);
    reset(); send_size = 8;
    assert(IRIS_RADIO_SetTransmitPower(&device, 50) == OS_ERROR);
    expect_bad_reply(&device);
    assert(receives == 0 && delays == 0);
    reset(); receive_status = -8;
    assert(IRIS_RADIO_RequestPowerHK(&device, &hk) == -8);
    expect_bad_reply(&device);
    for (size_t size : {0U, 16U, 19U, 21U})
    {
        reset(); receive_size = size; expect_bad_reply(&device);
    }
    for (unsigned offset : {0U, 1U, 18U, 19U})
    {
        reset(); response[offset] ^= 1; expect_bad_reply(&device);
    }
    reset(); response[17] = 101; expect_bad_reply(&device);
    reset(); response[14] = 0xFF; expect_bad_reply(&device);
    std::cout << "IRIS driver tests passed (mock transport, real simulator power encoder)\n";
}
