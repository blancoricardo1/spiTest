#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main);

void main(void)
{
    const struct device *spi_dev = DEVICE_DT_GET(DT_NODELABEL(spi1));

    if (!device_is_ready(spi_dev)) {
        LOG_ERR("SPI device not ready!");
        return;
    }

    struct spi_config spi_cfg = {
        .frequency = 1000000U,
        .operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8),
        .slave = 0,
        .cs = NULL,  // If you don't have a chip-select GPIO, or manage it manually
    };

    uint8_t tx_buf_data[] = {0xAA, 0xBB};  // Example command
    uint8_t rx_buf_data[2] = {0};

    struct spi_buf tx_buf = {
        .buf = tx_buf_data,
        .len = sizeof(tx_buf_data),
    };
    struct spi_buf rx_buf = {
        .buf = rx_buf_data,
        .len = sizeof(rx_buf_data),
    };

    struct spi_buf_set tx_set = {
        .buffers = &tx_buf,
        .count = 1,
    };
    struct spi_buf_set rx_set = {
        .buffers = &rx_buf,
        .count = 1,
    };

    int err = spi_transceive(spi_dev, &spi_cfg, &tx_set, &rx_set);
    if (err) {
        LOG_ERR("SPI transceive failed: %d", err);
    } else {
        LOG_INF("Received: 0x%02X 0x%02X", rx_buf_data[0], rx_buf_data[1]);
    }
}
