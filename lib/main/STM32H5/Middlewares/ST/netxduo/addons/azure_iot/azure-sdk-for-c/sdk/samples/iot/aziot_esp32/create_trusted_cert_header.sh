#!/bin/bash
# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

set -x # Set trace on
set -o errexit # Exit if command failed
set -o nounset # Exit if variable not set
set -o pipefail # Exit if pipe failed

command -v xxd >/dev/null 2>&1 || { echo >&2 "Please install xxd."; exit 1; }

wget https://cacerts.digicert.com/BaltimoreCyberTrustRoot.crt.pem -O ca1.pem
wget https://cacerts.digicert.com/DigiCertGlobalRootG2.crt.pem -O ca2.pem

cat ca1.pem > ca.pem
cat ca2.pem >> ca.pem

echo -n -e '\0' >> ca.pem
xxd -i ca.pem ca.h
