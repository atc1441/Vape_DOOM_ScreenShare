/*
 * Copyright (c) 2022, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "usbd_core.h"
#include "usbd_printer.h"

struct printer_cfg_priv {
    uint8_t *device_id;
    uint8_t port_status;
} usbd_printer_cfg;

uint8_t printer_id[] = 
{
  0x00,
  '0','M','F','G',':','P','r','i','n','t','e','r',';','C','M','D',':',
  'E','P','S','O','N',';','M','D','L',':','P','O','S','-','8','0',';',
  'C','L','S',':','P','R','I','N','T','E','R',';',
  0x00
};

static int printer_class_interface_request_handler(struct usb_setup_packet *setup, uint8_t **data, uint32_t *len)
{
    USB_LOG_DBG("Printer Class request: "
                "bRequest 0x%02x\r\n",
                setup->bRequest);

    switch (setup->bRequest) {
        case PRINTER_REQUEST_GET_DEVICE_ID:
            memcpy(*data, printer_id, sizeof(printer_id));
            *len = sizeof(printer_id);
            break;
        case PRINTER_REQUEST_GET_PORT_SATTUS:

            break;
        case PRINTER_REQUEST_SOFT_RESET:

            break;
        default:
            USB_LOG_WRN("Unhandled Printer Class bRequest 0x%02x\r\n", setup->bRequest);
            return -1;
    }

    return 0;
}

static void printer_notify_handler(uint8_t event, void *arg)
{
    switch (event) {
        case USBD_EVENT_RESET:
            break;

        default:
            break;
    }
}

struct usbd_interface *usbd_printer_alloc_intf(void)
{
    struct usbd_interface *intf = usb_malloc(sizeof(struct usbd_interface));
    if (intf == NULL) {
        USB_LOG_ERR("no mem to alloc intf\r\n");
        return NULL;
    }

    intf->class_interface_handler = printer_class_interface_request_handler;
    intf->class_endpoint_handler = NULL;
    intf->vendor_handler = NULL;
    intf->notify_handler = printer_notify_handler;

    return intf;
}
