<#
.SYNOPSIS
Bumps up sdk version after release

.DESCRIPTION
This script bumps up the sdk version found in az_version.h following conventions defined at https://github.com/Azure/azure-sdk/blob/main/docs/policies/releases.md#incrementing-after-release-c
We use the version number defined in AZ_SDK_VERSION_STRING, and then overwrite other #defines

.PARAMETER NewVersionString
Use this to overide version incement logic and set a version specified by this parameter


.EXAMPLE
Updating sdk version to next preview version
Update-SdkVersion.ps1

Updating sdk version with a specified verion
Update-SdkVersion.ps1 -NewVersionString 2.0.5

#>

[CmdletBinding()]
Param (
    [ValidateNotNullOrEmpty()]
    [string] $NewVersionString
)

. ${PSScriptRoot}\..\common\scripts\SemVer.ps1
. ${PSScriptRoot}\SdkVersion-Common.ps1

# Updated Version in version file and changelog using computed or set NewVersionString
function Update-Version([AzureEngSemanticVersion]$SemVer, $Unreleased=$True, $ReplaceLatestEntryTitle=$False)
{
    Write-Output "New Version: $($SemVer)"
    # if ($SemVer.HasValidPrereleaseLabel() -ne $true){
    #     Write-Error "Invalid prerelease label"
    #     exit 1
    # }

    # This is macro string value when shipping GA
    $PrereleaseDefine = @"
#define AZ_SDK_VERSION_PRERELEASE
#undef AZ_SDK_VERSION_PRERELEASE
"@
    if ($SemVer.IsPrerelease -eq $true){
        # Macro string value when shipping preview
        $PrereleaseDefine = "`#define AZ_SDK_VERSION_PRERELEASE `"$($SemVer.PrereleaseLabel).$($SemVer.PrereleaseNumber)`""
    }

    (Get-Content -Path $SdkVersionPath -Raw) `
     -replace $VersionStringRegEx, "`${1}`"$($SemVer)`"" `
     -replace $VersionMajorRegEx,  "`${1}$($SemVer.Major)" `
     -replace $VersionMinorRegEx,  "`${1}$($SemVer.Minor)" `
     -replace $VersionPatchRegEx,  "`${1}$($SemVer.Patch)" `
     -replace $VersionPrereleaseRegEx,  $PrereleaseDefine |
     Set-Content -Path $SdkVersionPath -NoNewline

    # Increment Version in ChangeLog file
    & "${PSScriptRoot}/../common/scripts/Update-ChangeLog.ps1" -Version $SemVer.ToString() -ChangeLogPath $ChangelogPath -Unreleased $Unreleased -ReplaceLatestEntryTitle $ReplaceLatestEntryTitle
}


$match = $SdkVersionFile -match $VersionStringRegEx
$SdkVersion = $Matches[2]

if ([System.String]::IsNullOrEmpty($NewVersionString))
{
    $SemVer = [AzureEngSemanticVersion]::new($SdkVersion)
    Write-Output "Current Version: ${SdkVersion}"

    $SemVer.IncrementAndSetToPrerelease()
    Update-Version -SemVer $SemVer
}
else
{
    # Use specified VersionString
    $SemVer = [AzureEngSemanticVersion]::new($NewVersionString)
    Update-Version -SemVer $SemVer -Unreleased $False -ReplaceLatestEntryTitle $True
}
