#include <iris_radio_power.hpp>
#include <array>
#include <cassert>
#include <iostream>

int main()
{
    Nos3::IrisRadioPower power;
    std::array<std::uint8_t, 20> packet = {{
        0xDE, 0xAD, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 3, 0xBE, 0xEF
    }};
    const auto legacy = packet;
    power.extend_housekeeping(packet.data());
    assert(packet[17] == 0);

    const std::uint8_t fifty[] = {0, 0, 0, 50};
    assert(power.set(fifty));
    power.extend_housekeeping(packet.data());
    const std::array<std::uint8_t, 20> expected = {{
        0xDE, 0xAD, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 3,
        0, 0, 0, 50, 0xBE, 0xEF
    }};
    assert(packet == expected);
    for (unsigned i = 0; i < 14; ++i)
        assert(packet[i] == legacy[i]);

    // Invalid values must not change the previously accepted power.
    const std::uint8_t invalid[][4] = {
        {0, 0, 0, 101}, {0, 0, 1, 0}, {50, 0, 0, 0}, {255, 255, 255, 255}
    };
    for (const auto& value : invalid)
    {
        assert(!power.set(value));
        power.extend_housekeeping(packet.data());
        assert(packet == expected);
    }

    const std::uint8_t hundred[] = {0, 0, 0, 100};
    assert(power.set(hundred));
    power.extend_housekeeping(packet.data());
    assert(packet[17] == 100);
    const std::uint8_t zero[] = {0, 0, 0, 0};
    assert(power.set(zero));
    power.extend_housekeeping(packet.data());
    assert(packet[17] == 0);
    assert(power.set(fifty));
    power.reset();
    power.extend_housekeeping(packet.data());
    assert(packet[17] == 0);
    std::cout << "IRIS power protocol checks passed\n";
}
