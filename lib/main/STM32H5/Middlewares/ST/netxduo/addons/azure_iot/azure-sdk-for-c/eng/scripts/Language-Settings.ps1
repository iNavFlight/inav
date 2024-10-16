$Language = "c"
$PackageRepository = "C"
$packagePattern = "package-info.json"
$MetadataUri = "https://raw.githubusercontent.com/Azure/azure-sdk/main/_data/releases/latest/c-packages.csv"
$BlobStorageUrl = "https://azuresdkdocs.blob.core.windows.net/%24web?restype=container&comp=list&prefix=c%2F&delimiter=%2F"

# Parse out package publishing information given a vcpkg format.
function Get-c-PackageInfoFromPackageFile ($pkg, $workingDirectory)
{
  $packageInfo = Get-Content -Raw -Path $pkg | ConvertFrom-JSON
  $packageArtifactLocation = (Get-ItemProperty $pkg).Directory.FullName
  $releaseNotes = ""
  $readmeContent = ""

  $pkgVersion = $packageInfo.version

  $changeLogLoc = @(Get-ChildItem -Path $packageArtifactLocation -Recurse -Include "CHANGELOG.md")[0]
  if ($changeLogLoc)
  {
    $releaseNotes = Get-ChangeLogEntryAsString -ChangeLogLocation $changeLogLoc -VersionString $pkgVersion
  }

  $readmeContentLoc = @(Get-ChildItem -Path $packageArtifactLocation -Recurse -Include "README.md")[0]
  if ($readmeContentLoc) {
    $readmeContent = Get-Content -Raw $readmeContentLoc
  }

  return New-Object PSObject -Property @{
    PackageId      = ''
    PackageVersion = $pkgVersion
    ReleaseTag     = $pkgVersion
    # Artifact info is always considered deployable for C becasue it is not
    # deployed anywhere. Dealing with duplicate tags happens downstream in
    # CheckArtifactShaAgainstTagsList
    Deployable     = $true
    ReleaseNotes   = $releaseNotes
  }
}

# Stage and Upload Docs to blob Storage
function Publish-c-GithubIODocs ($DocLocation, $PublicArtifactLocation)
{
    # The documentation publishing process for C differs from the other
    # languages in this file because this script is invoked for the whole SDK
    # publishing. It is not, for example, invoked once per service publishing.
    # There is a similar situation for other language publishing steps above...
    # Those loops are left over from previous versions of this script which were
    # used to publish multiple docs packages in a single invocation.
    $pkgInfo = Get-Content $DocLocation/package-info.json | ConvertFrom-Json
    $releaseTag = RetrieveReleaseTag -artifactLocation $PublicArtifactLocation
    Upload-Blobs -DocDir $DocLocation -PkgName 'az_core' -DocVersion $pkgInfo.version -ReleaseTag $releaseTag
    Upload-Blobs -DocDir $DocLocation -PkgName 'az_iot' -DocVersion $pkgInfo.version -ReleaseTag $releaseTag
}

function Get-c-GithubIoDocIndex() {
  # Update the main.js and docfx.json language content
  UpdateDocIndexFiles -appTitleLang $PackageRepository
  # Fetch out all package metadata from csv file.
  $metadata = Get-CSVMetadata -MetadataUri $MetadataUri
  # Leave the track 2 packages if multiple packages fetched out.
  $artifacts =  Get-BlobStorage-Artifacts -blobStorageUrl $BlobStorageUrl -blobDirectoryRegex "^c/(.*)/$" -blobArtifactsReplacement '$1'
  # Build up the artifact to service name mapping for GithubIo toc.
  $tocContent = Get-TocMapping -metadata $metadata -artifacts $artifacts
  # Generate yml/md toc files and build site.
  GenerateDocfxTocContent -tocContent $tocContent -lang $PackageRepository -campaignId "UA-62780441-44"
}
