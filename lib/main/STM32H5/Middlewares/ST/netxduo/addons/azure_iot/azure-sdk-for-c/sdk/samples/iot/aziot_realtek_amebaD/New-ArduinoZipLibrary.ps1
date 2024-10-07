# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

git clone https://github.com/Azure/azure-sdk-for-c sdkrepo
mkdir azure-sdk-for-c

cp sdkrepo/sdk/inc/azure/core/* azure-sdk-for-c
cp sdkrepo/sdk/inc/azure/core/internal/* azure-sdk-for-c
cp -ErrorAction SilentlyContinue sdkrepo/sdk/inc/azure/iot/* azure-sdk-for-c
cp sdkrepo/sdk/inc/azure/iot/internal/* azure-sdk-for-c

cp -Recurse sdkrepo/sdk/src/azure/core/* azure-sdk-for-c/
cp -Recurse sdkrepo/sdk/src/azure/iot/* azure-sdk-for-c/
cp sdkrepo/sdk/src/azure/platform/az_noplatform.c azure-sdk-for-c/

rm -Force azure-sdk-for-c/internal

Get-ChildItem -Recurse -Include *.c,*.h -Path .\azure-sdk-for-c\ | %{
	$(Get-Content -Raw $_ ) -replace "<azure`/(iot`/internal|core`/internal|iot|core)`/", "<" | out-file -Encoding ascii -Force $_
}

Compress-Archive -Path 'azure-sdk-for-c/*' -DestinationPath 'azure-sdk-for-c.zip'

rm -Recurse -Force azure-sdk-for-c
rm -Recurse -Force sdkrepo/

