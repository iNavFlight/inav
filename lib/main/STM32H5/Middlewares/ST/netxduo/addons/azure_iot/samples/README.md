# Sample Guide

This sample showcases the usability of AZRTOS API to connect to Azure IoT and start interacting with Azure IoT services like IoTHub and Device Provisioning Service. All the output of sample is redirect to stdout. To start the sample, the user needs to provide user configuration in sample_config.h. Following are the configuration example to setup the sample.

## Prerequisites
To run this sample, user should create an IoTHub instance in Azure ([Doc](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-create-through-portal#create-an-iot-hub)). If sample is configured to use Device Provisioning service, use [doc](https://docs.microsoft.com/en-us/azure/iot-dps/quick-setup-auto-provision#create-a-new-iot-hub-device-provisioning-service) to create Device Provisioning instance in Azure and link it to the IoTHub ([Link](https://docs.microsoft.com/en-us/azure/iot-dps/quick-setup-auto-provision#link-the-iot-hub-and-your-device-provisioning-service)).

## IoTHub connect using Symmetric key

Sample connects to the device in the IoTHub using Symmetric key. To register new device in IoTHub use the [doc](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-create-through-portal#register-a-new-device-in-the-iot-hub) to create device with Symmetric key authentication. After device's registration is complete, copy the connection string for the device with following format **HostName=<>;DeviceId=<>;SharedAccessKey=<>**. Add following macros to your compiler option.

```
#define HOST_NAME                                   "<Hostname from connection string>"
#define DEVICE_ID                                   "<DeviceId from connection string>"
#define DEVICE_SYMMETRIC_KEY                        "<SharedAccessKey from connection string>"
```
Above configuration by default enables all the features like: Telemetry, Cloud to device message and Direct Methods. To disable anyone, add following macro corresponding to the feature.

```
/* Defined, telemetry is disabled. */
#define DISABLE_TELEMETRY_SAMPLE

/* Defined, C2D is disabled. */
#define DISABLE_C2D_SAMPLE

/* Defined, Direct methods is disabled. */
#define DISABLE_DIRECT_METHOD_SAMPLE

```

## IoTHub connect using X509 cert

Sample connects to the device in the IoTHub using X509 cert. To generate self signed cert use the same steps mention in [IoTHub connect using X509 cert](#iothub-connect-using-x509-cert) section. Once certificate is generated, register new device in IoTHub with this cert using the x509 getting started [document](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-security-x509-get-started). After device registration is complete, copy the connection string for the device with following format **HostName=<>;DeviceId=<>;x509=true**. Add following macros to your compiler option.

```
#define HOST_NAME                                   "<Hostname from connection string>"
#define DEVICE_ID                                   "<DeviceId from connection string>"
#define USE_DEVICE_CERTIFICATE                      1
#define DEVICE_KEY_TYPE                             NX_SECURE_X509_KEY_TYPE_RSA_PKCS1_DER /* NX_SECURE_X509_KEY_TYPE_RSA_PKCS1_DER for RSA, NX_SECURE_X509_KEY_TYPE_EC_DER for ECC */
```

### Steps to create self-signed certs using openssl:

Set x509 configuration file for common name in cert.
```
cat > x509_config.cfg <<EOT
[req]
req_extensions = client_auth
distinguished_name = req_distinguished_name

[req_distinguished_name]

[ client_auth ]
basicConstraints = CA:FALSE
keyUsage = digitalSignature, keyEncipherment
extendedKeyUsage = clientAuth
EOT
```

Create RSA self-signed certs
```
# Generate private key and certificate (public key).
openssl genrsa -out privkey.pem 2048
openssl req -new -days 365 -nodes -x509 -key privkey.pem -out cert.pem -config x509_config.cfg -subj "/CN=<Same as device Id>"

# Convert format from key to der.
openssl rsa -outform der -in privkey.pem -out privkey.der 

# Convert format from cert pem to der.
openssl x509 -outform der -in cert.pem -out cert.der
```

Or create EC self-signed certs
```
# Generate private key and certificate (public key). Note: The comman name must be device id.
openssl req -new -x509 -nodes -days 365 -newkey ec:<(openssl ecparam -name secp384r1) -keyout privkey.pem -out cert.pem -config x509_config.cfg -subj "/CN=<Same as device Id>"

# Convert format from key to der.
openssl ec -outform der -in privkey.pem -out privkey.der

# Convert format from cert pem to der.
openssl x509 -outform der -in cert.pem -out cert.der
```

Convert der to hex array (ubuntu) and set them in sample_device_identity.c.
```
echo "#include \"nx_api.h\"
/**
device cert (`openssl x509 -in cert.pem -fingerprint -noout | sed 's/://g' `) :
`cat cert.pem`

device private key :
`cat privkey.pem`
*/
" > sample_device_identity.c

xxd -i cert.der | sed -E "s/(unsigned char) (\w+)/\1 sample_device_cert_ptr/g; s/(unsigned int) (\w+)_len/\1 sample_device_cert_len/g" >> sample_device_identity.c
xxd -i privkey.der | sed -E "s/(unsigned char) (\w+)/\1 sample_device_private_key_ptr/g; s/(unsigned int) (\w+)_len/\1 sample_device_private_key_len/g" >> sample_device_identity.c
```


## Use Device Provisioning Service with Symmetric key

Sample uses Device Provisioning service to get IoTHub device details and then connects to assigned IoTHub device. To start, user creates individual enrollment using [doc](https://docs.microsoft.com/en-us/azure/iot-dps/quick-create-simulated-device-symm-key#create-a-device-enrollment-entry-in-the-portal)  with Symmetric key. After completion, update following macros to your compiler option. 

Note: To get the values, use the [doc](https://docs.microsoft.com/en-us/azure/iot-dps/concepts-device) to understand the concepts of Device provisioning service.

```
#define ENDPOINT                                    "<Service Endpoint or Global device endpoint from Provisioning service overview page>"
#define ID_SCOPE                                    "<ID Scope value from Provisioning service overview page>"
#define REGISTRATION_ID                             "<RegistrationId provide when doing Individual registration>"
#define DEVICE_SYMMETRIC_KEY                        "<Symmetric key from Individual registration detail page>"

/* Enable DPS */
#define ENABLE_DPS_SAMPLE
```

## Use Device Provisioning Service with X509

Sample uses Device Provisioning service to get IoTHub details and then connects to assigned IoTHub device. To start, user creates individual enrollment using [doc](https://docs.microsoft.com/en-us/azure/iot-dps/quick-create-simulated-device-x509#create-a-device-enrollment-entry-in-the-portal) with X509 cert. After completion, update following macros to your compiler option. 

Note: To get the values, use the [doc](https://docs.microsoft.com/en-us/azure/iot-dps/concepts-device) to understand the concepts of Device provisioning service.

```
#define ENDPOINT                                    "<Service Endpoint or Global device endpoint from Provisioning service overview page>"
#define ID_SCOPE                                    "<ID Scope value from Provisioning service overview page>"
#define REGISTRATION_ID                             "<RegistrationId provide when doing Individual registration>"
#define USE_DEVICE_CERTIFICATE                      1
#define DEVICE_KEY_TYPE                             NX_SECURE_X509_KEY_TYPE_RSA_PKCS1_DER /* Right now only RSA certs are supported*/

/* Enable DPS */
#define ENABLE_DPS_SAMPLE
```
To generate self signed cert use the same steps mention in [IoTHub connect using X509 cert](#iothub-connect-using-x509-cert) section.

### [IoT Plug and Play (Certificates)][pnp_sample]
Connect a IoT Plug and Play enabled device with the Digital Twin Model ID (DTMI) detailed [here](https://github.com/Azure/opendigitaltwins-dtdl/blob/master/DTDL/v2/samples/Thermostat.json).
In short, the capabilities are listed here:
- **Methods**: Invoke a IoT Plug and Play command called `getMaxMinReport` with JSON payload value `"since"` with an [ISO8601](https://en.wikipedia.org/wiki/ISO_8601) value for start time for the report. The method sends a response containing the following JSON payload:  
```json
{
  "maxTemp": 20,
  "minTemp": 20,
  "avgTemp": 20,
  "startTime": "<ISO8601 time>",
  "endTime": "<ISO8601 time>"
}
```
with correct values substituted for each field.
- **Telemetry**: Device sends a JSON message with the field name `temperature` and the `double` value of the temperature.
- **Twin**: Desired property with the field name `targetTemperature` and the `double` value for the desired temperature. Reported property with the field name `maxTempSinceLastReboot` and the `double` value for the highest temperature.Note that part of the IoT Plug and Play spec is a response to a desired property update from the service. The device will send back a reported property with a similarly named property and a set of "ack" values: `ac` for the HTTP-like ack code, `av` for ack version of the property, and an optional `ad` for an ack description.