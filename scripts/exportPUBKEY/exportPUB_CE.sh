#!/bin/sh

echo "Exporting EC PUBKEY"
openssl ec -in priv.pem -pubout -out pub.pem
