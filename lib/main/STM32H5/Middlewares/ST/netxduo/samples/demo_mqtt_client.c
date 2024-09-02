/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

/* 

   This is a small demonstration of the high-performance NetX TCP/IP stack 
   MQTT client.

   This demo program establishes a connection to the Mosquitto server at IP address
   10.0.10.1.  It subscribes to a topic, send a message to the same topic.
   The application shall be able to receive the same message from the broker. 

   This demo assumes that the NetX Duo TCP/IP stack has been properly configured,
   with TCP module enabled.   The entry function is "thread_mqtt_entry".  Caller
   needs to pass in the IP instance and an instance of a valid packet pool. 
*/

#include "nx_api.h"
#include "nxd_mqtt_client.h"

/* MQTT Demo defines */

/* IP Address of the local server. */
#define  LOCAL_SERVER_ADDRESS (IP_ADDRESS(192, 168, 1, 1))


/*****************************************************************************************/
/* MQTT Local Server IoT Client example.                                                 */
/*****************************************************************************************/

#define  DEMO_STACK_SIZE            2048
#define  CLIENT_ID_STRING           "mytestclient"
#define  MQTT_CLIENT_STACK_SIZE     4096

#define  STRLEN(p)                  (sizeof(p) - 1)


/* Declare the MQTT thread stack space. */
static ULONG                        mqtt_client_stack[MQTT_CLIENT_STACK_SIZE / sizeof(ULONG)];

/* Declare the MQTT client control block. */
static NXD_MQTT_CLIENT              mqtt_client;

/* Define the symbol for signaling a received message. */


/* Define the test threads.  */
#define TOPIC_NAME                  "topic"
#define MESSAGE_STRING              "This is a message. "

/* Define the priority of the MQTT internal thread. */
#define MQTT_THREAD_PRIORTY         2

/* Define the MQTT keep alive timer for 5 minutes */
#define MQTT_KEEP_ALIVE_TIMER       300

#define QOS0                        0
#define QOS1                        1

/* Declare event flag, which is used in this demo. */
TX_EVENT_FLAGS_GROUP                mqtt_app_flag;
#define DEMO_MESSAGE_EVENT          1
#define DEMO_ALL_EVENTS             3

/* Declare buffers to hold message and topic. */
static UCHAR message_buffer[NXD_MQTT_MAX_MESSAGE_LENGTH];
static UCHAR topic_buffer[NXD_MQTT_MAX_TOPIC_NAME_LENGTH];

/* Declare the disconnect notify function. */
static VOID my_disconnect_func(NXD_MQTT_CLIENT *client_ptr)
{
    NX_PARAMETER_NOT_USED(client_ptr);
    printf("client disconnected from server\n");
}


static VOID my_notify_func(NXD_MQTT_CLIENT* client_ptr, UINT number_of_messages)
{
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(number_of_messages);
    tx_event_flags_set(&mqtt_app_flag, DEMO_MESSAGE_EVENT, TX_OR);
    return;
  
}

void thread_mqtt_entry(NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr);

static ULONG    error_counter;
void thread_mqtt_entry(NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr)
{
UINT status;
NXD_ADDRESS server_ip;
ULONG events;
UINT topic_length, message_length;

    /* Create MQTT client instance. */
    status = nxd_mqtt_client_create(&mqtt_client, "my_client", CLIENT_ID_STRING, STRLEN(CLIENT_ID_STRING),
                                    ip_ptr, pool_ptr, (VOID*)mqtt_client_stack, sizeof(mqtt_client_stack), 
                                    MQTT_THREAD_PRIORTY, NX_NULL, 0);
    
    if (status)
    {
        printf("Error in creating MQTT client: 0x%02x\n", status);
        error_counter++;
    }

#ifdef NXD_MQTT_OVER_WEBSOCKET
    status = nxd_mqtt_client_websocket_set(&mqtt_client, (UCHAR *)"test.mosquitto.org", sizeof("test.mosquitto.org") - 1,
                                           (UCHAR *)"/mqtt", sizeof("/mqtt") - 1);
    if (status)
    {
        printf("Error in setting MQTT over WebSocket: 0x%02x\r\n", status);
        error_counter++;
    }
#endif /* NXD_MQTT_OVER_WEBSOCKET */

    /* Register the disconnect notification function. */
    nxd_mqtt_client_disconnect_notify_set(&mqtt_client, my_disconnect_func);
    
    /* Create an event flag for this demo. */
    status = tx_event_flags_create(&mqtt_app_flag, "my app event");
    if(status)
        error_counter++;
    

    server_ip.nxd_ip_version = 4;
    server_ip.nxd_ip_address.v4 = LOCAL_SERVER_ADDRESS;


    /* Start the connection to the server. */
    status = nxd_mqtt_client_connect(&mqtt_client, &server_ip, NXD_MQTT_PORT, 
                                     MQTT_KEEP_ALIVE_TIMER, 0, NX_WAIT_FOREVER);
    if(status)
        error_counter++;

    /* Subscribe to the topic with QoS level 0. */
    status = nxd_mqtt_client_subscribe(&mqtt_client, TOPIC_NAME, STRLEN(TOPIC_NAME), QOS0);
    if(status)
        error_counter++;
    
    /* Set the receive notify function. */
    status = nxd_mqtt_client_receive_notify_set(&mqtt_client, my_notify_func);
    if(status)
        error_counter++;
    
    /* Publish a message with QoS Level 1. */
    status = nxd_mqtt_client_publish(&mqtt_client, TOPIC_NAME, STRLEN(TOPIC_NAME),
                                     (CHAR*)MESSAGE_STRING, STRLEN(MESSAGE_STRING), 0, QOS1, NX_WAIT_FOREVER);


    /* Now wait for the broker to publish the message. */

    tx_event_flags_get(&mqtt_app_flag, DEMO_ALL_EVENTS, TX_OR_CLEAR, &events, TX_WAIT_FOREVER);
    if(events & DEMO_MESSAGE_EVENT)
    {
        status = nxd_mqtt_client_message_get(&mqtt_client, topic_buffer, sizeof(topic_buffer), &topic_length, 
                                             message_buffer, sizeof(message_buffer), &message_length);
        if(status == NXD_MQTT_SUCCESS)
        {
            topic_buffer[topic_length] = 0;
            message_buffer[message_length] = 0;
            printf("topic = %s, message = %s\n", topic_buffer, message_buffer);
        }
    }

    /* Now unsubscribe the topic. */
    nxd_mqtt_client_unsubscribe(&mqtt_client, TOPIC_NAME, STRLEN(TOPIC_NAME));

    /* Disconnect from the broker. */
    nxd_mqtt_client_disconnect(&mqtt_client);

    /* Delete the client instance, release all the resources. */
    nxd_mqtt_client_delete(&mqtt_client);
    
    return;

}


