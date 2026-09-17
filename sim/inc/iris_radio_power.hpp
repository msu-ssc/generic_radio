#ifndef IRIS_RADIO_POWER_HPP
#define IRIS_RADIO_POWER_HPP

#include <cstdint>
#include <atomic>

namespace Nos3
{
    // Device payloads are unsigned 32-bit integers in network byte order.
    // This setting is reported telemetry only; it does not change link physics yet.
    class IrisRadioPower
    {
    public:
        enum { SET_POWER_COMMAND = 2, REQUEST_POWER_HK_COMMAND = 3, POWER_HK_SIZE = 20 };

        IrisRadioPower() : _milliwatts(0) {}

        bool set(const std::uint8_t payload[4])
        {
            const std::uint32_t value = (std::uint32_t(payload[0]) << 24) |
                                       (std::uint32_t(payload[1]) << 16) |
                                       (std::uint32_t(payload[2]) << 8) |
                                       std::uint32_t(payload[3]);
            if (value > 100)
                return false;
            _milliwatts = value;
            return true;
        }

        void reset() { _milliwatts = 0; }

        // The caller first writes legacy HK into bytes 0..15. Replace its
        // trailer with power, then append the trailer to the extended packet.
        void extend_housekeeping(std::uint8_t out[POWER_HK_SIZE]) const
        {
            const std::uint32_t milliwatts = _milliwatts.load();
            out[14] = (milliwatts >> 24) & 0xFF;
            out[15] = (milliwatts >> 16) & 0xFF;
            out[16] = (milliwatts >> 8) & 0xFF;
            out[17] = milliwatts & 0xFF;
            out[18] = 0xBE;
            out[19] = 0xEF;
        }

    private:
        // DISABLE arrives on the simulator command bus, on a different thread.
        std::atomic<std::uint32_t> _milliwatts;
    };
}

#endif
