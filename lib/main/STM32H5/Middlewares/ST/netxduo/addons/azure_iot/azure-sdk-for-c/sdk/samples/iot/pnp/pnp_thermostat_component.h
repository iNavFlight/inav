// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef PNP_THERMOSTAT_COMPONENT_H
#define PNP_THERMOSTAT_COMPONENT_H

#include <stdbool.h>
#include <stdint.h>

#include <azure/az_core.h>
#include <azure/az_iot.h>

#include "pnp_mqtt_message.h"

// State associated with the current thermostat component.
typedef struct
{
  az_span component_name;
  double average_temperature;
  double current_temperature;
  double maximum_temperature;
  double minimum_temperature;
  double temperature_summation;
  uint32_t temperature_count;
  bool send_maximum_temperature_property;
} pnp_thermostat_component;

/**
 * @brief Initialize a #pnp_thermostat_component which holds device thermostat info.
 *
 * @param[out] out_thermostat_component A pointer to a #out_thermostat_component instance to
 * initialize.
 * @param[in] component_name The name of the component.
 * @param[in] initial_temperature The initial temperature to set all temperature member variables.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK #pnp_thermostat_component is initialied successfully.
 * @retval #AZ_ERROR_ARG The pointer to the #pnp_thermostat_component instance is NULL.
 */
az_result pnp_thermostat_init(
    pnp_thermostat_component* out_thermostat_component,
    az_span component_name,
    double initial_temperature);

/**
 * @brief Sends a telemetry message for this thermostat component.
 *
 * @param[in] thermostat_component A pointer to the themostat on which to invoke the command.
 * @param[in] hub_client An #az_iot_hub_client corresponding to this MQTT connection.
 * @param[in] mqtt_client An MQTTClient corresponding to this MQTT connection.
 */
void pnp_thermostat_send_telemetry_message(
    pnp_thermostat_component* thermostat_component,
    az_iot_hub_client const* hub_client,
    MQTTClient mqtt_client);

/**
 * @brief Sends the maximum temperature since last reboot property.
 *
 * @param[in] hub_client An #az_iot_hub_client corresponding to this MQTT connection.
 * @param[in] mqtt_client An MQTTClient corresponding to this MQTT connection.
 */
void pnp_thermostat_update_maximum_temperature_property(
    pnp_thermostat_component* thermostat_component,
    az_iot_hub_client const* hub_client,
    MQTTClient mqtt_client);

/**
 * @brief Processes a property update intended for thermostat component.  Parses request, build
 * response, and responds via MQTT.
 *
 * @param[in, out] thermostat_component A pointer to the themostat on which to invoke the command.
 * @param[in] hub_client An #az_iot_hub_client corresponding to this MQTT connection.
 * @param[in] mqtt_client An MQTTClient corresponding to this MQTT connection.
 * @param[in, out] property_name_and_value The #az_json_reader pointing to the property name.
 * @param[in] version The version parsed from the received message, and used to prepare the returned
 * reported property message.
 */
void pnp_thermostat_process_property_update(
    pnp_thermostat_component* thermostat_component,
    az_iot_hub_client const* hub_client,
    MQTTClient mqtt_client,
    az_json_reader* property_name_and_value,
    int32_t version);

/**
 * @brief Processes a command intended for the thermostat component.  Parses request, builds
 * response, and responds via MQTT.
 *
 * @param[in] thermostat_component A pointer to the themostat on which to invoke the command.
 * @param[in] hub_client An #az_iot_hub_client corresponding to this MQTT connection.
 * @param[in] mqtt_client An MQTTClient corresponding to this MQTT connection.
 * @param[in] command_request The #az_iot_hub_client_command_request that contains information about
 * the requested command.
 * @param[in] command_received_payload An #az_span with the payload of the command.
 */
void pnp_thermostat_process_command_request(
    pnp_thermostat_component const* thermostat_component,
    az_iot_hub_client const* hub_client,
    MQTTClient mqtt_client,
    az_iot_hub_client_command_request const* command_request,
    az_span command_received_payload);

#endif // PNP_THERMOSTAT_COMPONENT_H
