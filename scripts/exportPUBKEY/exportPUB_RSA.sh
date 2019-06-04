#!/bin/sh

echo "Exporting RSA PUBKEY"
openssl rsa -in priv.pem -pubout -out pub.pem
