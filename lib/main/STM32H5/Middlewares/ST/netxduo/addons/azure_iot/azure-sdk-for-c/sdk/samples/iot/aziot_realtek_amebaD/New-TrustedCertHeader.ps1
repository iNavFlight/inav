# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

function New-XxdHeader
{
    param($InFile, $OutFile, $BytesPerColumn = 12)

    $Hex = Format-Hex -Path $InFile

    Out-File -Force -Encoding ascii -NoNewline -FilePath $OutFile -InputObject "unsigned char ca_pem[] = {"
    
    $Index = 0;
    while ($Index -LT ($Hex.Bytes.Count - 1))
    {
        if (($Index % ($BytesPerColumn)) -EQ 0)
        {
            Out-File -Append -Encoding ascii -NoNewline -FilePath $OutFile -InputObject "`n "
        }

        Out-File -Append -Encoding ascii -FilePath $OutFile -NoNewline -InputObject $(" 0x{00:x2}," -f $Hex.Bytes[$Index])

        $Index++;
    }

    if (($Index % ($BytesPerColumn)) -EQ 0)
    {
        Out-File -Append -Encoding ascii -NoNewline -FilePath $OutFile -InputObject "`n "
    }

    Out-File -Append -Encoding ascii -FilePath $OutFile -InputObject $(" 0x{00:x2}" -f $Hex.Bytes[$Index])
    
    Out-File -Append -Encoding ascii -FilePath $OutFile -InputObject "};"

    Out-File -Append -Encoding ascii -FilePath $OutFile -InputObject "unsigned int ca_pem_len = $($Hex.Bytes.Count);"
}

echo "It will take a few seconds, please wait."

Remove-Item -Force -Confirm:$false ".\ca.pem" -erroraction SilentlyContinue

Invoke-WebRequest -Uri https://cacerts.digicert.com/BaltimoreCyberTrustRoot.crt.pem -OutFile ca1.pem
Invoke-WebRequest -Uri https://cacerts.digicert.com/DigiCertGlobalRootG2.crt.pem -OutFile ca2.pem

Get-Content .\ca1.pem | Out-File -Encoding ascii .\ca.pem
Get-Content .\ca2.pem | Out-File -Append -Encoding ascii .\ca.pem

Out-File -Append -NoNewline -Encoding ascii -FilePath .\ca.pem -InputObject "`0"
(Get-Content .\ca.pem -Raw).Replace("`r`n", "`n") | Set-Content .\ca.pem -Force -NoNewline

New-XxdHeader -InFile ".\ca.pem" -OutFile ".\ca.h"

Remove-Item -Force -Confirm:$false ".\ca1.pem"
Remove-Item -Force -Confirm:$false ".\ca2.pem"
