set(COMPONENT_ADD_INCLUDEDIRS .)

file(GLOB component_files "*.c")

set(COMPONENT_SRCS ${component_files})

set(COMPONENT_PRIV_REQUIRES mqtt_connection nvs_flash)

register_component()
