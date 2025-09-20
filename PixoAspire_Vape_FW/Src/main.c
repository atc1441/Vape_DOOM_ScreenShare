#include "main.h"
#include "usb_config.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include <stdbool.h>
#include <string.h>
#include "image.h"

static void system_clock_config(void);
static void usb_init(void);
static void gpio_config(void);
void error_handler(void);

IWDG_HandleTypeDef watchdog_handle;
SPI_HandleTypeDef spi1_handle;
DMA_HandleTypeDef dma_handle_ch1;
DMA_HandleTypeDef dma_handle_ch2;

#define LCD_HEIGHT 173
#define LCD_WIDTH 323
#define BYTES_PER_PIXEL 2
#define IMAGE_TOTAL_BYTES (LCD_WIDTH * LCD_HEIGHT)
#define IMAGE_HEADER_SIZE 4
const uint8_t image_header[IMAGE_HEADER_SIZE] = {0xDE, 0xAD, 0xBE, 0xEF};

typedef enum
{
    STATE_WAITING_FOR_HEADER,
    STATE_RECEIVING_IMAGE
} ReceiveState;

volatile ReceiveState current_receive_state = STATE_WAITING_FOR_HEADER;

#define RECEPTION_TIMEOUT_MS 10
volatile uint32_t last_packet_timestamp = 0;
volatile uint32_t received_byte_count = 0;

extern uint8_t read_buffer[];

#define SPI_FIFO_DEPTH 128
#define SPI_FIFO_BUFFER_SIZE 128
uint8_t spi_fifo_buffer[SPI_FIFO_DEPTH][SPI_FIFO_BUFFER_SIZE];
volatile uint32_t spi_fifo_nbytes[SPI_FIFO_DEPTH];
volatile uint32_t fifo_head = 0;
volatile uint32_t fifo_tail = 0;

const uint8_t lcd_init_data[] = {
    0x11, 0x00, 0x36, 0x01, 0xC0, 0x3A, 0x01, 0x05, 0xB2, 0x05, 0x0C, 0x0C,
    0x00, 0x33, 0x33, 0xB7, 0x01, 0x35, 0xBB, 0x01, 0x35, 0xC0, 0x01, 0x2C,
    0xC2, 0x01, 0x01, 0xC3, 0x01, 0x13, 0xC4, 0x01, 0x20, 0xC6, 0x01, 0x0F,
    0xD0, 0x02, 0xA4, 0xA1, 0xD6, 0x02, 0xA4, 0xA1, 0xE0, 0x0E, 0xF0, 0x00,
    0x04, 0x04, 0x04, 0x05, 0x29, 0x33, 0x3E, 0x38, 0x12, 0x12, 0x28, 0x30,
    0xE1, 0x0E, 0xF0, 0x07, 0x0A, 0x0D, 0x0B, 0x07, 0x28, 0x33, 0x3E, 0x36,
    0x14, 0x14, 0x29, 0x32, 0x21, 0x00, 0x29, 0x00, 0x00, 0x00};

#define TRANSMIT_BUFFER_LENGTH (LCD_HEIGHT * 2)
uint8_t transmit_buffer[TRANSMIT_BUFFER_LENGTH];

void spi_send_byte(uint8_t data)
{
    transmit_buffer[0] = data;
    HAL_SPI_Transmit_DMA(&spi1_handle, (uint8_t *)transmit_buffer, 1);
}

void lcd_send_command(uint8_t command)
{
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
    spi_send_byte(command);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
}

void lcd_send_data(uint8_t data)
{
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_5, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
    spi_send_byte(data);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
}

void lcd_send_command_with_data(uint8_t command, const uint8_t *data, size_t length)
{
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
    spi_send_byte(command);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_5, GPIO_PIN_SET);
    HAL_SPI_Transmit_DMA(&spi1_handle, (uint8_t *)data, length);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
}

void lcd_execute_init_sequence(const uint8_t *init_data)
{
    const uint8_t *ptr = init_data;
    uint8_t command, length;

    while (1)
    {
        command = *ptr++;
        if (command == 0x00)
        {
            break;
        }
        length = (*ptr++) & 0x7F;
        lcd_send_command_with_data(command, ptr, length);
        ptr += length;
        if (command == 0x11)
        {
            HAL_Delay(120);
        }
    }
}

void lcd_set_address_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    width -= 1;
    height -= 1;
    x += 34;
    width += 34;
    lcd_send_command(0x2A);
    lcd_send_data(x >> 8);
    lcd_send_data(x & 0xFF);
    lcd_send_data(width >> 8);
    lcd_send_data(width & 0xFF);
    lcd_send_command(0x2B);
    lcd_send_data(y >> 8);
    lcd_send_data(y & 0xFF);
    lcd_send_data(height >> 8);
    lcd_send_data(height & 0xFF);
    lcd_send_command(0x2C);
}

void lcd_initialize()
{
    spi1_handle.Instance = SPI1;
    spi1_handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    spi1_handle.Init.Direction = SPI_DIRECTION_2LINES;
    spi1_handle.Init.CLKPolarity = SPI_POLARITY_HIGH;
    spi1_handle.Init.CLKPhase = SPI_PHASE_2EDGE;
    spi1_handle.Init.DataSize = SPI_DATASIZE_8BIT;
    spi1_handle.Init.FirstBit = SPI_FIRSTBIT_MSB;
    spi1_handle.Init.NSS = SPI_NSS_SOFT;
    spi1_handle.Init.Mode = SPI_MODE_MASTER;
    spi1_handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;

    if (HAL_SPI_DeInit(&spi1_handle) != HAL_OK)
    {
        error_handler();
    }
    if (HAL_SPI_Init(&spi1_handle) != HAL_OK)
    {
        error_handler();
    }

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    HAL_Delay(2);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_Delay(15);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    HAL_Delay(120);

    lcd_execute_init_sequence(lcd_init_data);
    lcd_set_address_window(0, 0, LCD_HEIGHT, LCD_WIDTH);

    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_5, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);

    for (int i = 0; i < (LCD_HEIGHT); i += LCD_WIDTH * 2)
    {
        HAL_SPI_Transmit(&spi1_handle, (uint8_t *)image_data[i], LCD_WIDTH * 2, 5000);
    }
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
}

#define CDC_IN_EP 0x81
#define CDC_OUT_EP 0x02
#define CDC_INT_EP 0x83

const uint8_t ack_signal[] = "A";

void usbd_cdc_acm_bulk_out(uint8_t ep, uint32_t nbytes)
{
    if (nbytes == 0)
    {
        usbd_ep_start_read(CDC_OUT_EP, read_buffer, 128);
        return;
    }

    uint32_t next_head = (fifo_head + 1) % SPI_FIFO_DEPTH;
    if (next_head == fifo_tail)
    {
        usbd_ep_start_read(CDC_OUT_EP, read_buffer, 128);
        return;
    }

    bool is_image_header = (nbytes >= IMAGE_HEADER_SIZE && memcmp(read_buffer, image_header, IMAGE_HEADER_SIZE) == 0);

    if (is_image_header)
    {
        current_receive_state = STATE_RECEIVING_IMAGE;
        received_byte_count = 0;
        last_packet_timestamp = HAL_GetTick();
        fifo_tail = fifo_head;
        usbd_ep_start_write(CDC_IN_EP, (uint8_t *)ack_signal, 1);
    }
    else if (current_receive_state == STATE_RECEIVING_IMAGE)
    {
        memcpy(spi_fifo_buffer[fifo_head], read_buffer, nbytes);
        spi_fifo_nbytes[fifo_head] = nbytes;
        fifo_head = next_head;
        last_packet_timestamp = HAL_GetTick();
        usbd_ep_start_write(CDC_IN_EP, (uint8_t *)ack_signal, 1);
    }
    usbd_ep_start_read(CDC_OUT_EP, read_buffer, 128);
}

#define CONVERSION_BUFFER_SIZE 128
uint16_t rgb565_buffer[CONVERSION_BUFFER_SIZE];

int main(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    __HAL_RCC_GPIOB_CLK_ENABLE();

    gpio_init_struct.Pin = GPIO_PIN_13;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_OD;
    gpio_init_struct.Pull = GPIO_NOPULL;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio_init_struct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);

    HAL_Init();
    system_clock_config();

    (*(volatile unsigned int *)0x4002101C) |= 0x8000000u;
    (*(volatile unsigned int *)0x40006C30) &= ~1u;

    __HAL_RCC_LSI_ENABLE();
    while (READ_BIT(RCC->CSR, RCC_CSR_LSIRDY) == 0U)
    {
    }

    watchdog_handle.Instance = IWDG;
    watchdog_handle.Init.Prescaler = IWDG_PRESCALER_32;
    watchdog_handle.Init.Reload = 3000;
    HAL_IWDG_Init(&watchdog_handle);
    HAL_IWDG_Refresh(&watchdog_handle);

    gpio_config();

    GPIO_InitTypeDef gpio_init_struct_1 = {0};
    gpio_init_struct_1.Pin = GPIO_PIN_14;
    gpio_init_struct_1.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct_1.Pull = GPIO_PULLUP;
    gpio_init_struct_1.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOE, &gpio_init_struct_1);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, GPIO_PIN_SET);

    usb_init();
    lcd_initialize();

    while (1)
    {
        HAL_IWDG_Refresh(&watchdog_handle);
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);

        if (current_receive_state == STATE_RECEIVING_IMAGE)
        {
            if ((HAL_GetTick() - last_packet_timestamp) > RECEPTION_TIMEOUT_MS)
            {
                current_receive_state = STATE_WAITING_FOR_HEADER;
                received_byte_count = 0;
                fifo_tail = fifo_head;
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
            }
        }

        if (fifo_head != fifo_tail)
        {
            if (current_receive_state == STATE_RECEIVING_IMAGE)
            {
                if (received_byte_count == 0)
                {
                    lcd_set_address_window(0, 0, LCD_HEIGHT, LCD_WIDTH);
                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_5, GPIO_PIN_SET);
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
                }

                uint32_t chunk_size = spi_fifo_nbytes[fifo_tail];
                for (uint32_t i = 0; i < chunk_size; i++)
                {
                    uint8_t pixel_rgb332 = spi_fifo_buffer[fifo_tail][i];

                    uint8_t red_3bit = (pixel_rgb332 & 0xE0) >> 5;
                    uint8_t green_3bit = (pixel_rgb332 & 0x1C) >> 2;
                    uint8_t blue_2bit = (pixel_rgb332 & 0x03);

                    uint16_t red_5bit = (red_3bit * 255 / 7) >> 3;
                    uint16_t green_6bit = (green_3bit * 255 / 7) >> 2;
                    uint16_t blue_5bit = (blue_2bit * 255 / 3) >> 3;

                    uint16_t pixel_rgb565 = (red_5bit << 11) | (green_6bit << 5) | blue_5bit;
                    pixel_rgb565 = ~pixel_rgb565;

                    rgb565_buffer[i] = (pixel_rgb565 >> 8) | (pixel_rgb565 << 8);
                }

                HAL_SPI_Transmit(&spi1_handle, (uint8_t *)rgb565_buffer, chunk_size * 2, 5000);
                received_byte_count += chunk_size;

                if (received_byte_count >= IMAGE_TOTAL_BYTES)
                {
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
                    current_receive_state = STATE_WAITING_FOR_HEADER;
                }

                fifo_tail = (fifo_tail + 1) % SPI_FIFO_DEPTH;
            }
            else
            {
                fifo_tail = (fifo_tail + 1) % SPI_FIFO_DEPTH;
            }
        }
    }
}

static void gpio_config(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);

    gpio_init_struct.Pin = GPIO_PIN_6;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOC, &gpio_init_struct);

    gpio_init_struct.Pin = GPIO_PIN_13;
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;
    gpio_init_struct.Pull = GPIO_PULLDOWN;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init_struct.Alternate = 0;
    HAL_GPIO_Init(GPIOA, &gpio_init_struct);

    gpio_init_struct.Pin = GPIO_PIN_14;
    gpio_init_struct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio_init_struct);

    gpio_init_struct.Pin = GPIO_PIN_13;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_OD;
    gpio_init_struct.Pull = GPIO_NOPULL;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio_init_struct);

    gpio_init_struct.Pin = GPIO_PIN_9;
    gpio_init_struct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &gpio_init_struct);

    gpio_init_struct.Pin = GPIO_PIN_3;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_OD;
    gpio_init_struct.Pull = GPIO_NOPULL;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOC, &gpio_init_struct);

    gpio_init_struct.Pin = GPIO_PIN_8;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &gpio_init_struct);

    gpio_init_struct.Pin = GPIO_PIN_15;
    HAL_GPIO_Init(GPIOA, &gpio_init_struct);
    gpio_init_struct.Pin = GPIO_PIN_10;
    HAL_GPIO_Init(GPIOE, &gpio_init_struct);
    gpio_init_struct.Pin = GPIO_PIN_1;
    HAL_GPIO_Init(GPIOC, &gpio_init_struct);
    gpio_init_struct.Pin = GPIO_PIN_12;
    HAL_GPIO_Init(GPIOE, &gpio_init_struct);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, GPIO_PIN_RESET);

    gpio_init_struct.Pin = GPIO_PIN_11;
    HAL_GPIO_Init(GPIOE, &gpio_init_struct);
    gpio_init_struct.Pin = GPIO_PIN_5;
    HAL_GPIO_Init(GPIOA, &gpio_init_struct);
    gpio_init_struct.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOA, &gpio_init_struct);
    gpio_init_struct.Pin = GPIO_PIN_12;
    HAL_GPIO_Init(GPIOB, &gpio_init_struct);
    gpio_init_struct.Pin = GPIO_PIN_13;
    HAL_GPIO_Init(GPIOE, &gpio_init_struct);
    gpio_init_struct.Pin = GPIO_PIN_14;
    HAL_GPIO_Init(GPIOB, &gpio_init_struct);
    gpio_init_struct.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIOB, &gpio_init_struct);
    gpio_init_struct.Pin = GPIO_PIN_5;
    HAL_GPIO_Init(GPIOD, &gpio_init_struct);
    gpio_init_struct.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOD, &gpio_init_struct);
    gpio_init_struct.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOB, &gpio_init_struct);

    gpio_init_struct.Pin = GPIO_PIN_15;
    gpio_init_struct.Mode = GPIO_MODE_INPUT;
    gpio_init_struct.Pull = GPIO_NOPULL;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio_init_struct);

    gpio_init_struct.Pin = GPIO_PIN_13;
    HAL_GPIO_Init(GPIOC, &gpio_init_struct);
    gpio_init_struct.Pin = GPIO_PIN_6;
    gpio_init_struct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOE, &gpio_init_struct);

    gpio_init_struct.Pin = GPIO_PIN_5;
    gpio_init_struct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &gpio_init_struct);

    gpio_init_struct.Pin = GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &gpio_init_struct);

    gpio_init_struct.Pin = GPIO_PIN_2;
    gpio_init_struct.Mode = GPIO_MODE_ANALOG;
    gpio_init_struct.Pull = GPIO_NOPULL;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOC, &gpio_init_struct);

    gpio_init_struct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4;
    HAL_GPIO_Init(GPIOA, &gpio_init_struct);

    gpio_init_struct.Pin = GPIO_PIN_14;
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;
    gpio_init_struct.Pull = GPIO_NOPULL;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOE, &gpio_init_struct);

    gpio_init_struct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    gpio_init_struct.Pull = GPIO_PULLUP;
    gpio_init_struct.Alternate = 2;
    HAL_GPIO_Init(GPIOA, &gpio_init_struct);

    gpio_init_struct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init_struct.Alternate = 3;
    HAL_GPIO_Init(GPIOB, &gpio_init_struct);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
}

static void usb_init(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    SET_BIT(RCC->CFGR1, RCC_CFGR1_USBSELHSI48_Msk);
    __HAL_RCC_USB_CLK_ENABLE();
    cdc_acm_init();
    NVIC_EnableIRQ(USBD_IRQn);
}

static void system_clock_config(void)
{
    RCC_OscInitTypeDef osc_init_struct = {0};
    RCC_ClkInitTypeDef clk_init_struct = {0};

    osc_init_struct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSI48M;
    osc_init_struct.HSEState = RCC_HSE_OFF;
    osc_init_struct.HSEFreq = RCC_HSE_16_32MHz;
    osc_init_struct.HSI48MState = RCC_HSI48M_ON;
    osc_init_struct.HSIState = RCC_HSI_ON;
    osc_init_struct.LSEState = RCC_LSE_OFF;
    osc_init_struct.LSEDriver = RCC_LSEDRIVE_HIGH;
    osc_init_struct.LSIState = RCC_LSI_ON;
    osc_init_struct.PLL.PLLState = RCC_PLL_ON;
    osc_init_struct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    osc_init_struct.PLL.PLLMUL = RCC_PLL_MUL18;
    if (HAL_RCC_OscConfig(&osc_init_struct) != HAL_OK)
    {
        error_handler();
    }

    clk_init_struct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk_init_struct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk_init_struct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk_init_struct.APB1CLKDivider = RCC_HCLK_DIV1;
    clk_init_struct.APB2CLKDivider = RCC_CFGR_PPRE1_DIV2;
    if (HAL_RCC_ClockConfig(&clk_init_struct, 6) != HAL_OK)
    {
        error_handler();
    }

    SystemCoreClockUpdate();
    HAL_InitTick(7u);
}

void error_handler(void)
{
    while (1)
    {
    }
}