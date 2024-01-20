#ifndef _BLE_SERVICE_H_
#define _BLE_SERVICE_H_

#pragma once

#include "esp_log.h"
#include "nvs_flash.h"
/* BLE */
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nimble/ble.h"
#include "esp_nimble_hci.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {

class BLE_Service {
    public:
        BLE_Service();
        ~BLE_Service();
        esp_err_t Start();
        esp_err_t Stop();
        bool SendNotification();
        bool AreThereSubscribers(); 
    private:
        esp_err_t startNVS(void);

        static void bleprph_host_task(void *param);
        static void bleprph_on_sync(void);
        static void bleprph_on_reset(int reason);
        static int  bleprph_gap_event(struct ble_gap_event *event, void *arg);
        static void bleprph_advertise(void);

        static int gatt_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt, void *arg);

        static int gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len, 
                                void *dst, uint16_t *len); 

        int gatt_svr_init(void);

    private:
        static bool notify_enable_;
        static const struct ble_gatt_svc_def gatt_svr_svcs_[];
};

} // extern "C"
#endif

#endif
