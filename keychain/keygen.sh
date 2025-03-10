#!/bin/bash

ROOT_SUBJECT="/C=US/ST=Unset/L=Unset/O=User Relay Protocol/OU=Tests/CN=RootCA"
SERVER_SUBJECT="/C=US/ST=Unset/L=Unset/O=User Relay Protocol/OU=Tests/CN=localhost"
ROOT_PASSWORD="my-little-urp"

echo "Generating certificate..."

openssl genpkey -algorithm RSA:2048 -out root-key.pem -aes256 -pass pass:"$ROOT_PASSWORD" 2>/dev/null
echo " * Generated Root CA private key 'root-key.pem'"

openssl req -x509 -new -key root-key.pem -sha256 -days 1825 -out root-crt.pem -subj "$ROOT_SUBJECT" -passin pass:"$ROOT_PASSWORD" 2>/dev/null
echo " * Generated Root CA self-signed certificate 'root-key.pem'"

openssl genpkey -algorithm RSA:2048 -out server-key.pem 2>/dev/null
echo " * Generated server private key 'server-key.pem'"

openssl req -new -key server-key.pem -out server.csr -subj "$SERVER_SUBJECT" 2>/dev/null
echo " * Generated server certificate signing request 'server.csr'"

# Delete certificate.conf if it exists
rm -f certificate.conf

# Create a configuration file for the server certificate
cat > certificate.conf <<EOL
[req]
distinguished_name = req_distinguished_name
req_extensions = req_ext
prompt = no

[req_distinguished_name]
C = US
ST = Unset
L = Unset
O = User Relay Protocol
OU = Tests
CN = localhost

[req_ext]
subjectAltName = @alt_names

[alt_names]
DNS.1 = localhost
EOL

openssl x509 -req -in server.csr -CA root-crt.pem -CAkey root-key.pem -CAcreateserial -out server-crt.pem -days 99 -sha256 -extfile certificate.conf -extensions req_ext -passin pass:"$ROOT_PASSWORD" 2>/dev/null
echo " * Generated server Root CA signed certificate 'root-crt.pem'"
echo

echo "URP server should use:"
echo " * ssl-private-key 'server-key.pem'"
echo " * ssl-certificate 'server-crt.pem'"
echo

# Instructions to import the Root CA into Firefox
echo "To trust this Root certificate in Firefox:"
echo " * Go to Settings > Privacy & Security > Certificates > View Certificates."
echo " * In the Authorities tab, click Import."
echo " * Select the root-crt.pem file and import it."
echo " * Trust the certificate to identify websites."
echo

echo "You can then check if it works like this:"
echo " * run 'openssl s_server -key server-key.pem -cert server-crt.pem -accept 4433 -www'"
echo " * open https://localhost:4433 in the browser"
echo " * you should see a simple textual webpage"
