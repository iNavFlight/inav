cd /workspaces/azure-sdk-for-c
mkdir .vscode
cp ./.vscode-config/codespaces-launch.json ./.vscode/launch.json
cp ./.vscode-config/codespaces-tasks.json ./.vscode/tasks.json

mkdir cert
cd cert

openssl ecparam -out device_ec_key.pem -name prime256v1 -genkey
openssl req -new -days 30 -sha256 -nodes -x509 -key device_ec_key.pem -out device_ec_cert.pem -extensions client_auth -config ../sdk/samples/iot/x509_config.cfg -subj "/CN=paho-sample-device1"

openssl x509 -noout -text -in device_ec_cert.pem

rm -f device_cert_store.pem
cat device_ec_cert.pem device_ec_key.pem > device_cert_store.pem

openssl x509 -noout -fingerprint -in device_ec_cert.pem | sed 's/://g'| sed 's/\(SHA1 Fingerprint=\)//g' | tee fingerprint.txt
