#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>

LOG_MODULE_REGISTER(main);

#define MAX30009_REG_PARTID  0xFF
#define CSB_NODE DT_NODELABEL(gpio0)
#define CSB_PIN 15

void main(void)
{
    const struct device *spi_dev = DEVICE_DT_GET(DT_NODELABEL(spi1));
    const struct device *gpio_dev = DEVICE_DT_GET(CSB_NODE);

    if (!device_is_ready(spi_dev) || !device_is_ready(gpio_dev)) {
        LOG_ERR("SPI or GPIO device not ready!");
        return;
    }

    gpio_pin_configure(gpio_dev, CSB_PIN, GPIO_OUTPUT_ACTIVE); // CS high
    gpio_pin_set(gpio_dev, CSB_PIN, 1);

    struct spi_config spi_cfg = {
        .frequency = 125000U,
        .operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB |
                     SPI_WORD_SET(8) | SPI_MODE_CPOL | SPI_MODE_CPHA,
        .slave = 0,
        .cs = NULL,
    };

    uint8_t tx_buf_data[] = { 0xFF, 0x80, 0x00 };
    uint8_t rx_buf_data[] = { 0x00, 0x00 , 0x00};

    struct spi_buf tx_buf = {
        .buf = tx_buf_data,
        .len = sizeof(tx_buf_data),
    };
    struct spi_buf rx_buf = {
        .buf = rx_buf_data,
        .len = sizeof(rx_buf_data),
    };

    struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
    struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

    gpio_pin_set(gpio_dev, CSB_PIN, 0);
    k_busy_wait(100); // short delay to ensure CSB settled

    int err = spi_transceive(spi_dev, &spi_cfg, &tx_set, &rx_set);

    gpio_pin_set(gpio_dev, CSB_PIN, 1);

    if (err) {
        LOG_ERR("SPI transceive failed: %d", err);
    } else {
        LOG_INF("WHO_AM_I = 0x%02X", rx_buf_data[1]);
    }
}
