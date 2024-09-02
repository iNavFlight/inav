// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef PNP_TEMPCONTROLLER_COMPONENT_H
#define PNP_TEMPCONTROLLER_COMPONENT_H

#include "pnp_mqtt_message.h"
#include <azure/az_core.h>

/**
 * @brief Processes a command intended for the temperature controller component.  Parses
 * request, builds response, and responds via MQTT.
 *
 * @param[in] hub_client An
 * #az_iot_hub_client corresponding to this MQTT connection.
 * @param[in] mqtt_client An MQTTClient corresponding to this MQTT connection.
 * @param[in] command_request The #az_iot_hub_client_command_request that contains information about
 * the requested command.
 * @param[in] command_received_payload An #az_span with the payload of the command.
 */
void pnp_temperature_controller_process_command_request(
    az_iot_hub_client const* hub_client,
    MQTTClient mqtt_client,
    az_iot_hub_client_command_request const* command_request,
    az_span command_received_payload);

/**
 * @brief Sends the serial number property.
 *
 * @param[in] hub_client An #az_iot_hub_client
 * corresponding to this MQTT connection.
 * @param[in] mqtt_client An MQTTClient corresponding to this MQTT connection.
 */
void pnp_temperature_controller_send_serial_number(
    az_iot_hub_client const* hub_client,
    MQTTClient mqtt_client);

/**
 * @brief Sends the simulated workingSet via telemetry.
 *
 * @param[in] hub_client An
 * #az_iot_hub_client corresponding to this MQTT connection.
 * @param[in] mqtt_client An MQTTClient corresponding to this MQTT connection.
 */
void pnp_temperature_controller_send_workingset(
    az_iot_hub_client const* hub_client,
    MQTTClient mqtt_client);

#endif // PNP_TEMPCONTROLLER_COMPONENT_H
