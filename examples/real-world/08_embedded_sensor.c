/**
 * 08_embedded_sensor.c - Embedded / Firmware Hardware Sensor Driver with nur.h
 *
 * Demonstrates:
 * - Zero-allocation, stack-only memory footprint for microcontrollers / firmware
 * - Hardware communication validation (CRC-8 checksum verification)
 * - Hardware timeout and physical bounds checking
 * - Explicit error reporting without exceptions or dynamic allocation
 */

#include "nur.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Sensor reading structure */
typedef struct SensorReading {
    float temperature_c;
    float humidity_pct;
} SensorReading;

NUR_RESULT_TYPE(SensorReading, sensor_reading);

/**
 * Standard CRC-8 polynomial (x^8 + x^5 + x^4 + 1 -> 0x31)
 */
static uint8_t crc8(const uint8_t *data, size_t len) {
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b) {
            if (crc & 0x80) {
                crc = (uint8_t)((crc << 1) ^ 0x31);
            } else {
                crc = (uint8_t)(crc << 1);
            }
        }
    }
    return crc;
}

/**
 * Raw hardware packet received over I2C/SPI bus.
 * Layout:
 *   [0..1]: Temperature raw (16-bit big-endian)
 *   [2..3]: Humidity raw (16-bit big-endian)
 *   [4]:    CRC-8 checksum
 */
static NurResult_sensor_reading decode_sensor_packet(const uint8_t *packet, size_t len, int sim_bus_timeout) {
    /* 1. Hardware bus timeout check */
    if (sim_bus_timeout) {
        return NUR_ERR(sensor_reading, NUR_ERROR(NUR_E_TIMEOUT, "I2C ACK timeout from sensor address 0x44"));
    }

    /* 2. Packet length validation */
    if (!packet || len < 5) {
        return NUR_ERR(sensor_reading, NUR_ERROR_CTX(NUR_E_TRUNCATED, "Sensor packet truncated", "I2C_RX_BUF"));
    }

    /* 3. CRC-8 integrity verification */
    uint8_t expected_crc = crc8(packet, 4);
    if (packet[4] != expected_crc) {
        return NUR_ERR(sensor_reading, NUR_ERROR_CTX(NUR_E_CHECKSUM, "CRC-8 validation failed on raw packet", "0x44"));
    }

    /* 4. Convert raw register values to physical units */
    uint16_t raw_temp = (uint16_t)((packet[0] << 8) | packet[1]);
    uint16_t raw_hum  = (uint16_t)((packet[2] << 8) | packet[3]);

    /* Standard formula: Temp = -45 + 175 * (raw / 65535) */
    float temp_c = -45.0f + 175.0f * ((float)raw_temp / 65535.0f);
    /* Formula: Hum = 100 * (raw / 65535) */
    float hum_pct = 100.0f * ((float)raw_hum / 65535.0f);

    /* 5. Physical bounds sanity check */
    if (temp_c < -40.0f || temp_c > 125.0f) {
        return NUR_ERR(sensor_reading, NUR_ERROR(NUR_E_OUT_OF_RANGE, "Temperature exceeds physical sensor range [-40C, 125C]"));
    }
    if (hum_pct < 0.0f || hum_pct > 100.0f) {
        return NUR_ERR(sensor_reading, NUR_ERROR(NUR_E_OUT_OF_RANGE, "Humidity exceeds physical bounds [0%, 100%]"));
    }

    SensorReading reading;
    reading.temperature_c = temp_c;
    reading.humidity_pct = hum_pct;

    return NUR_OK(sensor_reading, reading);
}

static void test_packet(const char *scenario, const uint8_t *pkt, size_t len, int sim_timeout) {
    printf("--- Scenario: %s ---\n", scenario);
    NurResult_sensor_reading res = decode_sensor_packet(pkt, len, sim_timeout);

    if (NUR_IS_OK(res)) {
        SensorReading r = NUR_VALUE(res);
        printf("[SUCCESS] Reading decoded:\n");
        printf("  Temperature: %.2f C\n", (double)r.temperature_c);
        printf("  Humidity:    %.2f %%\n", (double)r.humidity_pct);
    } else {
        printf("[EXPECTED ERROR] ");
        nur_error_eprint(&NUR_ERROR_OF(res));
    }
    printf("\n");
}

int main(void) {
    printf("============================================\n");
    printf("  nur.h Embedded Hardware Sensor Driver     \n");
    printf("============================================\n\n");

    /* 1. Valid sensor packet: raw temp = 26214 (~25.0 C), raw hum = 32768 (~50.0 %) */
    uint8_t valid_packet[5] = { 0x66, 0x66, 0x80, 0x00, 0 };
    valid_packet[4] = crc8(valid_packet, 4); /* compute valid CRC */
    test_packet("Valid I2C Sensor Transmission", valid_packet, 5, 0);

    /* 2. Corrupted packet (CRC failure) */
    uint8_t corrupted_packet[5] = { 0x66, 0x66, 0x80, 0x00, 0xDE /* invalid CRC */ };
    test_packet("Corrupted Bit on Hardware Bus (CRC mismatch)", corrupted_packet, 5, 0);

    /* 3. Bus Timeout */
    test_packet("Hardware Sensor I2C Bus Timeout", valid_packet, 5, 1);

    /* 4. Truncated Packet */
    uint8_t short_packet[3] = { 0x66, 0x66, 0x80 };
    test_packet("Truncated Packet (short read)", short_packet, 3, 0);

    return 0;
}
