# Obtain Current SDK Version
. ${PSScriptRoot}\SdkVersion-Common.ps1

$match = $SdkVersionFile -match $VersionStringRegEx
$SdkVersion = $Matches[2]
return $SdkVersion
