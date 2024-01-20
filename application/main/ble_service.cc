#include "ble_service.h"

namespace BLE{
namespace nni{
    static uint8_t own_addr_type;
    static uint16_t notification_handle;
    static uint16_t conn_handle;

    const char *tag = "BLE Service";
    static void bleprph_print_conn_desc(struct ble_gap_conn_desc *desc);

    static void print_addr(const void *addr);

    // GATT Server Service,         B2BBC642-46DA-11ED-B878-0242AC120002
    static const ble_uuid128_t gatt_svr_svc_uuid =
    BLE_UUID128_INIT(0x02, 0x00, 0x12, 0xac, 0x42, 0x02, 0x78, 0xb8, 0xed, 0x11, 0xda, 0x46, 0x42, 0xc6, 0xbb, 0xb2);

    // GATT Server Characteristic , C9AF9C76-46DE-11ED-B878-0242AC120002
    static const ble_uuid128_t gatt_svr_chr_uuid =
    BLE_UUID128_INIT(0x02, 0x00, 0x12, 0xac, 0x42, 0x02, 0x78, 0xb8, 0xed, 0x11, 0xde, 0x46, 0x76, 0x9c, 0xaf, 0xc9);

    static void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
}
}

/***********************************************/
/*             BLE Service                     */
/***********************************************/
bool BLE_Service::notify_enable_ = false;
const struct ble_gatt_svc_def BLE_Service::gatt_svr_svcs_[] = {
    {
        /*** Service: Control and config */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &(BLE::nni::gatt_svr_svc_uuid.u),
        .characteristics = 
            (struct ble_gatt_chr_def[]) { 
                {
                    /*** Characteristic:  */
                    .uuid = &(BLE::nni::gatt_svr_chr_uuid.u),
                    .access_cb = gatt_svr_chr_access,
                    .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY,
                    .val_handle = &(BLE::nni::notification_handle),
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

BLE_Service::BLE_Service() {
    ESP_LOGI(BLE::nni::tag, "BLE Service Constructor");
    notify_enable_ = false;
}

BLE_Service::~BLE_Service() {
    ESP_LOGI(BLE::nni::tag, "BLE Service Destructor");
    Stop();
}

bool BLE_Service::AreThereSubscribers() {
    return notify_enable_;
}

// MBFU: In its simplest form, an mbuf is a memory block with some space reserved for internal information 
// and a pointer which is used to “chain” memory blocks together in order to create a “packet”. 
// This is a very important aspect of the mbuf: 
//              the ability to chain mbufs together to create larger “packets” (chains of mbufs)
bool BLE_Service::SendNotification() {
    int rc;
    struct os_mbuf *om;
    const char *notification = "Hello There";
    om = ble_hs_mbuf_from_flat(notification, strlen(notification));
    rc = ble_gatts_notify_custom(BLE::nni::conn_handle, BLE::nni::notification_handle, om);

    if (rc != 0) {
        ESP_LOGI(BLE::nni::tag, "error notifying; rc=%d\n", rc);
        return false;
    }
    return true;
}

esp_err_t BLE_Service::Start(){
    //! Below is the sequence of APIs to be called to init/enable NimBLE host and ESP controller:
    ESP_LOGI(BLE::nni::tag, "Starting BLE Service");
    notify_enable_ = false;
    esp_err_t ret;

    ret = startNVS();
    assert(ret == ESP_OK);
    
    nimble_port_init();
    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = bleprph_on_reset;
    ble_hs_cfg.sync_cb = bleprph_on_sync;
    ble_hs_cfg.gatts_register_cb = BLE::nni::gatt_svr_register_cb;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_NO_IO; // No I/O capabilities
    ble_hs_cfg.sm_mitm = 1; // Man in the middle protection
    ble_hs_cfg.sm_sc = 1; // Secure connections

    ret = gatt_svr_init();
    assert(ret == 0);

    /* Set the default device name. */
    ret = ble_svc_gap_device_name_set("nimble-ble"); //!! Set the name of this device
    assert(ret == 0);

    nimble_port_freertos_init(bleprph_host_task);

    return ret;
}

esp_err_t BLE_Service::Stop() {
    //! Below is the sequence of APIs to be called to disable/deinit NimBLE host and ESP controller:
    ESP_LOGI(BLE::nni::tag, "Stopping BLE Service");
    esp_err_t ret = nimble_port_stop();
    if (ret == ESP_OK) {
        ret = nimble_port_deinit();
        if (ret != ESP_OK) {
            ESP_LOGE(BLE::nni::tag, "NimBLE host deinitialization failed\n");
        }
    }
    return ret;
}

/***********************************************/
/*             BLE Service Support             */
/***********************************************/
int 
BLE_Service::gatt_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt,
        void *arg) {
    int rc;

    // NOTE: It's not possible to set a name to a characteristic/service
    switch (ctxt->op) {
        case BLE_GATT_ACCESS_OP_READ_CHR: 
            {
                const char *characteristic_value = "I am characteristic value";
                rc = os_mbuf_append(ctxt->om, characteristic_value, strlen(characteristic_value));
            }
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

        case BLE_GATT_ACCESS_OP_WRITE_CHR: 
            {
                char characteristic_received_value[50];
                // Minimum and maximum length the client can write to a characterstic
                uint16_t min_length = 1;
                uint16_t max_length = 700;

                rc = gatt_svr_chr_write(ctxt->om, 
                        min_length, max_length, 
                        &characteristic_received_value, NULL);
                ESP_LOGI(BLE::nni::tag, "Received=%s\n", characteristic_received_value);  // Print the received value
            }
            return rc;
        default:
            assert(0);
            return BLE_ATT_ERR_UNLIKELY;
    }
}

esp_err_t 
BLE_Service::startNVS() {
    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

int 
BLE_Service::gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len, void *dst, uint16_t *len)
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

esp_err_t
BLE_Service::gatt_svr_init(void) {
    int rc;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    // Adjusts a host configuration object’s settings to accommodate the specified service definition array. 
    rc = ble_gatts_count_cfg(gatt_svr_svcs_);
    if (rc != 0) {
        return ESP_FAIL;
    }

    // Register Services table
    rc = ble_gatts_add_svcs(gatt_svr_svcs_);
    if (rc != 0) {
        return ESP_FAIL;
    }

    return ESP_OK;
}


void
BLE_Service::bleprph_advertise(void) {
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
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

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

    // TODO: for some reason, this doesn't work
//     ble_uuid128_t uuid128_array[1] = {BLE::nni::gatt_svr_svc_uuid};
//     fields.uuids128 = uuid128_array;
//     fields.num_uuids128 = 1;
//     fields.uuids128_is_complete = 0;
//     
    const ble_uuid16_t gatt_svr_chr_uuid16 = BLE_UUID16_INIT(0x0001);
    ble_uuid16_t uuid_array[1] = {gatt_svr_chr_uuid16};
    fields.uuids16 = uuid_array;
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0)
    {
        ESP_LOGE(BLE::nni::tag, "error setting advertisement data; rc=%d\n", rc);
        return;
    }

    /* Begin advertising. */
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(BLE::nni::own_addr_type, NULL, BLE_HS_FOREVER,
            &adv_params, bleprph_gap_event, NULL);
    if (rc != 0)
    {
        ESP_LOGE(BLE::nni::tag, "error enabling advertisement; rc=%d\n", rc);
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
int
BLE_Service::bleprph_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type)
    {
        case BLE_GAP_EVENT_CONNECT:
            /* A new connection was established or a connection attempt failed. */
            ESP_LOGI(BLE::nni::tag, "connection %s; status=%d ",
                    event->connect.status == 0 ? "established" : "failed",
                    event->connect.status);
            if (event->connect.status == 0)
            {
                rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
                assert(rc == 0);
                BLE::nni::bleprph_print_conn_desc(&desc);
            }
            ESP_LOGI(BLE::nni::tag, "\n");

            if (event->connect.status != 0)
            {
                /* Connection failed; resume advertising. */
                bleprph_advertise();
            }
            BLE::nni::conn_handle = event->connect.conn_handle;
            return 0;

        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(BLE::nni::tag, "disconnect; reason=%d ", event->disconnect.reason);
            BLE::nni::bleprph_print_conn_desc(&event->disconnect.conn);
            ESP_LOGI(BLE::nni::tag, "\n");

            /* Connection terminated; resume advertising. */
            bleprph_advertise();
            return 0;

        case BLE_GAP_EVENT_CONN_UPDATE:
            /* The central has updated the connection parameters. */
            ESP_LOGI(BLE::nni::tag, "connection updated; status=%d ",
                    event->conn_update.status);
            rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
            assert(rc == 0);
            BLE::nni::bleprph_print_conn_desc(&desc);
            ESP_LOGI(BLE::nni::tag, "\n");
            return 0;

        case BLE_GAP_EVENT_ADV_COMPLETE:
            ESP_LOGI(BLE::nni::tag, "advertise complete; reason=%d",
                    event->adv_complete.reason);
            bleprph_advertise();
            return 0;

        case BLE_GAP_EVENT_SUBSCRIBE:

            ESP_LOGI(BLE::nni::tag, "subscribe event; cur_notify=%d\n value handle; "
                    "val_handle=%d\n"
                    "conn_handle=%d attr_handle=%d "
                    "reason=%d prevn=%d curn=%d previ=%d curi=%d\n",
                    event->subscribe.conn_handle,
                    event->subscribe.attr_handle,
                    event->subscribe.reason,
                    event->subscribe.prev_notify,
                    event->subscribe.cur_notify,
                    event->subscribe.cur_notify, 
                    BLE::nni::notification_handle, //!! Client Subscribed to notification_handle
                    event->subscribe.prev_indicate,
                    event->subscribe.cur_indicate);

            if (event->subscribe.attr_handle == BLE::nni::notification_handle)
            {
                printf("\nSubscribed with notification_handle =%d\n", event->subscribe.attr_handle);
                notify_enable_ = event->subscribe.cur_notify; //!! As the client is now subscribed to notifications, the value is set to 1
                printf("notify_enable=%d\n", notify_enable_);
            }
            
            return 0;

        case BLE_GAP_EVENT_MTU:
            ESP_LOGI(BLE::nni::tag, "mtu update event; conn_handle=%d cid=%d mtu=%d\n",
                    event->mtu.conn_handle,
                    event->mtu.channel_id,
                    event->mtu.value);
            return 0;
    }

    return 0;
}

void 
BLE_Service::bleprph_on_reset(int reason) {
    ESP_LOGE(BLE::nni::tag, "Resetting state; reason=%d\n", reason);
}

void
BLE_Service::bleprph_on_sync(void) {
    int rc;
    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &BLE::nni::own_addr_type);
    if (rc != 0) {
        ESP_LOGE(BLE::nni::tag, "error determining address type; rc=%d\n", rc);
        return;
    }

    /* Printing ADDR */
    uint8_t addr_val[6] = {0};
    rc = ble_hs_id_copy_addr(BLE::nni::own_addr_type, addr_val, NULL);
    rc = ble_hs_id_copy_addr(BLE::nni::own_addr_type, addr_val, NULL);
    ESP_LOGI(BLE::nni::tag, "Device Address: ");
    BLE::nni::print_addr(addr_val);
    ESP_LOGI(BLE::nni::tag, "\n");
    /* Begin advertising. */
    bleprph_advertise();
}

void 
BLE_Service::bleprph_host_task(void *param) {
    ESP_LOGI(BLE::nni::tag, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();
    nimble_port_freertos_deinit();
}



static void
BLE::nni::bleprph_print_conn_desc(struct ble_gap_conn_desc *desc)
{
    ESP_LOGI(BLE::nni::tag, "handle=%d our_ota_addr_type=%d our_ota_addr=",
            desc->conn_handle, desc->our_ota_addr.type);
    BLE::nni::print_addr(desc->our_ota_addr.val);
    ESP_LOGI(BLE::nni::tag, " our_id_addr_type=%d our_id_addr=",
            desc->our_id_addr.type);
    BLE::nni::print_addr(desc->our_id_addr.val);
    ESP_LOGI(BLE::nni::tag, " peer_ota_addr_type=%d peer_ota_addr=",
            desc->peer_ota_addr.type);
    BLE::nni::print_addr(desc->peer_ota_addr.val);
    ESP_LOGI(BLE::nni::tag, " peer_id_addr_type=%d peer_id_addr=",
            desc->peer_id_addr.type);
    BLE::nni::print_addr(desc->peer_id_addr.val);
    ESP_LOGI(BLE::nni::tag, " conn_itvl=%d conn_latency=%d supervision_timeout=%d "
            "encrypted=%d authenticated=%d bonded=%d\n",
            desc->conn_itvl, desc->conn_latency,
            desc->supervision_timeout,
            desc->sec_state.encrypted,
            desc->sec_state.authenticated,
            desc->sec_state.bonded);
}


void 
BLE::nni::gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg) {
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op)
    {
        case BLE_GATT_REGISTER_OP_SVC:
            ESP_LOGD(BLE::nni::tag, "registered service %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                    ctxt->svc.handle);
            break;

        case BLE_GATT_REGISTER_OP_CHR:
            ESP_LOGD(BLE::nni::tag, "registering characteristic %s with "
                    "def_handle=%d val_handle=%d\n",
                    ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                    ctxt->chr.def_handle,
                    ctxt->chr.val_handle);
            break;

        case BLE_GATT_REGISTER_OP_DSC:
            ESP_LOGD(BLE::nni::tag, "registering descriptor %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                    ctxt->dsc.handle);
            break;

        default:
            assert(0);
            break;
    }
}

void 
BLE::nni::print_addr(const void *addr){
    const uint8_t *u8p;
    u8p = (const uint8_t*)addr;
    ESP_LOGI(BLE::nni::tag, "%02x:%02x:%02x:%02x:%02x:%02x",u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);
}

