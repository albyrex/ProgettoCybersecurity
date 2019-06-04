#!/bin/sh

echo "Generation Certification Request for Private Key (namefile = priv.pem)"
openssl req -out CSR.csr -key priv.pem -new
