
#
# Main component of TF Micro project 'application'.
#

idf_component_register(
    SRCS
        "image_provider.cc"
        "main.cc"
        "main_functions.cc"
        "model_settings.cc"
        "model_utils.cc"
		"model.cc"
#         "person_detect_model_data.cc"
        "app_camera_esp.cc"
        "ble_service.cc"

    PRIV_REQUIRES console static_images spi_flash nvs_flash bt
    INCLUDE_DIRS "")
