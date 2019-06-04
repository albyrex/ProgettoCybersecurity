#!/bin/sh

echo "Generation RSA Private Key"
openssl genrsa -out ./priv.pem 2048
