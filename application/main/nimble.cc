#include "nimble.h"

#include <string>
/** GATT server. */
#define GATT_SVR_SVC_ALERT_UUID               0x1811

static const char *tag = "BLE NIMBLE";
static uint8_t own_addr_type;
static esp_err_t ret;
static uint16_t control_notif_handle;
static uint16_t notification_handle;
static uint16_t conn_handle;

static bool notify_enable = false;
static std::string notification = "Hello There";

// GATT Server Service,         B2BBC642-46DA-11ED-B878-0242AC120002
static const ble_uuid128_t gatt_svr_svc_uuid =
BLE_UUID128_INIT(0x02, 0x00, 0x12, 0xac, 0x42, 0x02, 0x78, 0xb8, 0xed, 0x11, 0xda, 0x46, 0x42, 0xc6, 0xbb, 0xb2);

static const ble_uuid16_t gatt_svr_chr_uuid16 = BLE_UUID16_INIT(GATT_SVR_SVC_ALERT_UUID);

// GATT Server Characteristic , C9AF9C76-46DE-11ED-B878-0242AC120002
static const ble_uuid128_t gatt_svr_chr_uuid =
BLE_UUID128_INIT(0x02, 0x00, 0x12, 0xac, 0x42, 0x02, 0x78, 0xb8, 0xed, 0x11, 0xde, 0x46, 0x76, 0x9c, 0xaf, 0xc9);

uint16_t min_length = 1;   //!! minimum length the client can write to a characterstic
uint16_t max_length = 700; //!! maximum length the client can write to a characterstic

//@_____________Forward declaration of some functions ___________
void ble_store_config_init(void);
static int bleprph_gap_event(struct ble_gap_event *event, void *arg);

//!! Callback function. When ever characrstic will be accessed by user, this function will execute
static int gatt_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt,
        void *arg); 

//!! Callback function. When ever user write to this characterstic,this function will execute
static int gatt_svr_chr_write(struct os_mbuf *om, 
        uint16_t min_len, uint16_t max_len, 
        void *dst, uint16_t *len); 

static void bleprph_on_reset(int reason);
void bleprph_host_task(void *param);
static void bleprph_on_sync(void);
//@___________________________Heart of nimble code _________________________________________

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        /*** Service: Control and config */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) { 
            {
                /*** Characteristic:  */
                .uuid = &gatt_svr_chr_uuid.u,
                .access_cb = gatt_svr_chr_access,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &notification_handle,
            }, 
            {
                0, /* No more characteristics in this service. */ 
            }
        },
    },

    {
        0, /* No more services. this is necessary */
    },
};

static int gatt_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt,
        void *arg) 
{
    int rc;

    //TODO: add name to service and characteristic
    switch (ctxt->op) {
        // In case user accessed this characterstic to read its value, bellow lines will execute
        case BLE_GATT_ACCESS_OP_READ_CHR: 
            // In its simplest form, an mbuf is a memory block with some space reserved for internal information 
            // and a pointer which is used to “chain” memory blocks together in order to create a “packet”. 
            // This is a very important aspect of the mbuf: 
            //              the ability to chain mbufs together to create larger “packets” (chains of mbufs)
            std::string characteristic_value = "I am characteristic value";
            rc = os_mbuf_append(ctxt->om, characteristic_value.c_str(), characteristic_value.size());
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

        case BLE_GATT_ACCESS_OP_WRITE_CHR: 
            char characteristic_received_value[50];
            rc = gatt_svr_chr_write(ctxt->om, 
                    min_length, max_length, 
                    &characteristic_received_value, NULL);
            ESP_LOGI(tag, "Received=%s\n", characteristic_received_value);  // Print the received value
            if( strcmp(characteristic_received_value, "stop") == 0){
                stopBLE();
            }
            return rc;
        default:
            assert(0);
            return BLE_ATT_ERR_UNLIKELY;
    }
}

void sendNotification() {
    int rc;
    struct os_mbuf *om;

    // This value is checked so that we don't send notifications if user has not subscribed to our notification handle.

    if (notify_enable){
        // Value of variable "notification" will be sent as notification.
        om = ble_hs_mbuf_from_flat(notification.c_str(), notification.length());
        rc = ble_gattc_notify_custom(conn_handle, notification_handle, om);

        if (rc != 0) {
            printf("\n error notifying; rc\n");
        }
    }
    else {
        printf("user not subscribed to notifications.\n");
    }
}

void vTasksendNotification(void *pvParameters) {
    int rc;
    struct os_mbuf *om;
    while (1)
    {
        if (notify_enable)  {
            om = ble_hs_mbuf_from_flat(notification.c_str(), notification.length());
            rc = ble_gattc_notify_custom(conn_handle, notification_handle, om);
            printf("\n rc=%d\n", rc);

            if (rc != 0) {
                printf("\n error notifying; rc\n");
            }
        }
        else {
            printf("No one subscribed to notifications\n");
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void startBLE() {
    //! Below is the sequence of APIs to be called to init/enable NimBLE host and ESP controller:
    printf("\n Staring BLE \n");
    int rc;

    ESP_ERROR_CHECK(ret);

    nimble_port_init();
    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = bleprph_on_reset;
    ble_hs_cfg.sync_cb = bleprph_on_sync;
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_NO_IO; // No I/O capabilities
    ble_hs_cfg.sm_mitm = 1; // Man in the middle protection
    ble_hs_cfg.sm_sc = 1; // Secure connections

    rc = gatt_svr_init();
    assert(rc == 0);

    /* Set the default device name. */
    rc = ble_svc_gap_device_name_set("nimble-ble"); //!! Set the name of this device
    assert(rc == 0);

    nimble_port_freertos_init(bleprph_host_task);
}


void stopBLE() {
    //! Below is the sequence of APIs to be called to disable/deinit NimBLE host and ESP controller:
    printf("\n Stoping BLE and notification task \n");
    // vTaskDelete(xHandle);
    int ret = nimble_port_stop();
    if (ret == 0) {
        nimble_port_deinit();
        if (ret != ESP_OK) {
            ESP_LOGE(tag, "esp_nimble_hci_and_controller_deinit() failed with error: %d", ret);
        }
    }
}

void startNVS() {
    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
}

//@________________Bellow code will remain as it is.Take it abracadabra of BLE 😀 😀________________

static int gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len, void *dst, uint16_t *len)
{
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len)
    {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0)
    {
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}

void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op)
    {
        case BLE_GATT_REGISTER_OP_SVC:
            MODLOG_DFLT(DEBUG, "registered service %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                    ctxt->svc.handle);
            break;

        case BLE_GATT_REGISTER_OP_CHR:
            MODLOG_DFLT(DEBUG, "registering characteristic %s with "
                    "def_handle=%d val_handle=%d\n",
                    ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                    ctxt->chr.def_handle,
                    ctxt->chr.val_handle);
            break;

        case BLE_GATT_REGISTER_OP_DSC:
            MODLOG_DFLT(DEBUG, "registering descriptor %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                    ctxt->dsc.handle);
            break;

        default:
            assert(0);
            break;
    }
}

int gatt_svr_init(void)
{
    int rc;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    // Adjusts a host configuration object’s settings to accommodate the specified service definition array. 
    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    // Register Services table
    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}

    static void
bleprph_print_conn_desc(struct ble_gap_conn_desc *desc)
{
    MODLOG_DFLT(INFO, "handle=%d our_ota_addr_type=%d our_ota_addr=",
            desc->conn_handle, desc->our_ota_addr.type);
    print_addr(desc->our_ota_addr.val);
    MODLOG_DFLT(INFO, " our_id_addr_type=%d our_id_addr=",
            desc->our_id_addr.type);
    print_addr(desc->our_id_addr.val);
    MODLOG_DFLT(INFO, " peer_ota_addr_type=%d peer_ota_addr=",
            desc->peer_ota_addr.type);
    print_addr(desc->peer_ota_addr.val);
    MODLOG_DFLT(INFO, " peer_id_addr_type=%d peer_id_addr=",
            desc->peer_id_addr.type);
    print_addr(desc->peer_id_addr.val);
    MODLOG_DFLT(INFO, " conn_itvl=%d conn_latency=%d supervision_timeout=%d "
            "encrypted=%d authenticated=%d bonded=%d\n",
            desc->conn_itvl, desc->conn_latency,
            desc->supervision_timeout,
            desc->sec_state.encrypted,
            desc->sec_state.authenticated,
            desc->sec_state.bonded);
}

    static void
bleprph_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    const char *name;
    int rc;

    /**
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info).
     *     o Advertising tx power.
     *     o Device name.
     *     o 16-bit service UUIDs (alert notifications).
     */

    memset(&fields, 0, sizeof fields);

    /* Advertise two flags:
     *     o Discoverability in forthcoming advertisement (general)
     *     o BLE-only (BR/EDR unsupported).
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN |
        BLE_HS_ADV_F_BREDR_UNSUP;

    /* Indicate that the TX power level field should be included; have the
     * stack fill this value automatically.  This is done by assigning the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

//     fields.uuids128 = &gatt_svr_svc_uuid;
//     fields.num_uuids128 = 1;
//     fields.uuids128_is_complete = 1;
//     fields.uuids16 = (ble_uuid16_t[]){BLE_UUID16_INIT(GATT_SVR_SVC_ALERT_UUID)};
    ble_uuid16_t uuid_array[1] = {gatt_svr_chr_uuid16};
    fields.uuids16 = uuid_array;
//     fields.uuids16 = (ble_uuid16_t[]){gatt_svr_chr_uuid16};
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0)
    {
        MODLOG_DFLT(ERROR, "error setting advertisement data; rc=%d\n", rc);
        return;
    }

    /* Begin advertising. */
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
            &adv_params, bleprph_gap_event, NULL);
    if (rc != 0)
    {
        MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);
        return;
    }
}

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * bleprph uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
 * @param ctxt                  Various information pertaining to the event.
 * @param arg                   Application-specified argument; unused by
 *                                  bleprph.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
    static int
bleprph_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type)
    {
        case BLE_GAP_EVENT_CONNECT:
            /* A new connection was established or a connection attempt failed. */
            MODLOG_DFLT(INFO, "connection %s; status=%d ",
                    event->connect.status == 0 ? "established" : "failed",
                    event->connect.status);
            if (event->connect.status == 0)
            {
                rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
                assert(rc == 0);
                bleprph_print_conn_desc(&desc);
            }
            MODLOG_DFLT(INFO, "\n");

            if (event->connect.status != 0)
            {
                /* Connection failed; resume advertising. */
                bleprph_advertise();
            }
            conn_handle = event->connect.conn_handle;
            return 0;

        case BLE_GAP_EVENT_DISCONNECT:
            MODLOG_DFLT(INFO, "disconnect; reason=%d ", event->disconnect.reason);
            bleprph_print_conn_desc(&event->disconnect.conn);
            MODLOG_DFLT(INFO, "\n");

            /* Connection terminated; resume advertising. */
            bleprph_advertise();
            return 0;

        case BLE_GAP_EVENT_CONN_UPDATE:
            /* The central has updated the connection parameters. */
            MODLOG_DFLT(INFO, "connection updated; status=%d ",
                    event->conn_update.status);
            rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
            assert(rc == 0);
            bleprph_print_conn_desc(&desc);
            MODLOG_DFLT(INFO, "\n");
            return 0;

        case BLE_GAP_EVENT_ADV_COMPLETE:
            MODLOG_DFLT(INFO, "advertise complete; reason=%d",
                    event->adv_complete.reason);
            bleprph_advertise();
            return 0;

        case BLE_GAP_EVENT_SUBSCRIBE:

            MODLOG_DFLT(INFO, "subscribe event; cur_notify=%d\n value handle; "
                    "val_handle=%d\n"
                    "conn_handle=%d attr_handle=%d "
                    "reason=%d prevn=%d curn=%d previ=%d curi=%d\n",
                    event->subscribe.conn_handle,
                    event->subscribe.attr_handle,
                    event->subscribe.reason,
                    event->subscribe.prev_notify,
                    event->subscribe.cur_notify,
                    event->subscribe.cur_notify, notification_handle, //!! Client Subscribed to notification_handle
                    event->subscribe.prev_indicate,
                    event->subscribe.cur_indicate);

            if (event->subscribe.attr_handle == notification_handle)
            {
                printf("\nSubscribed with notification_handle =%d\n", event->subscribe.attr_handle);
                notify_enable = event->subscribe.cur_notify; //!! As the client is now subscribed to notifications, the value is set to 1
                printf("notify_enable=%d\n", notify_enable);
            }
            
            return 0;

        case BLE_GAP_EVENT_MTU:
            MODLOG_DFLT(INFO, "mtu update event; conn_handle=%d cid=%d mtu=%d\n",
                    event->mtu.conn_handle,
                    event->mtu.channel_id,
                    event->mtu.value);
            return 0;
    }

    return 0;
}

    static void
bleprph_on_reset(int reason) {
    MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}

    static void
bleprph_on_sync(void) {
    int rc;

    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }

    /* Printing ADDR */
    uint8_t addr_val[6] = {0};
    rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

    MODLOG_DFLT(INFO, "Device Address: ");
    print_addr(addr_val);
    MODLOG_DFLT(INFO, "\n");
    /* Begin advertising. */
    bleprph_advertise();
}

void bleprph_host_task(void *param)
{
    ESP_LOGI(tag, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}
