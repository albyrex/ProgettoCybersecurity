#!/bin/sh

echo "Generation Elliptic Curve Private Key"
openssl ecparam -name prime256v1 -genkey -noout -out ./priv.pem
