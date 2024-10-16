# NOTE: Update-SdkVersion relies on these variables
$RepoRoot = "${PSScriptRoot}/../.."
$SdkVersionPath = Join-Path $RepoRoot "sdk\inc\azure\core\az_version.h"
$ChangelogPath = Join-Path $RepoRoot "CHANGELOG.md"
$SdkVersionFile = Get-Content -Path $SdkVersionPath -Raw
$VersionStringRegEx = '(#define AZ_SDK_VERSION_STRING )"(([0-9]+)\.([0-9]+)\.([0-9]+)(\-[^\"\-]+)?)"';
$VersionMajorRegEx = '(#define AZ_SDK_VERSION_MAJOR )([0-9]+)';
$VersionMinorRegEx = '(#define AZ_SDK_VERSION_MINOR )([0-9]+)';
$VersionPatchRegEx = '(#define AZ_SDK_VERSION_PATCH )([0-9]+)';
$VersionPrereleaseRegEx = '(#define AZ_SDK_VERSION_PRERELEASE ?("[a-z0-9.]*")?)((\r\n|\r|\n)#undef AZ_SDK_VERSION_PRERELEASE)?';
