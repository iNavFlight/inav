Set-StrictMode -Version 3

AfterAll {
    ."$PSScriptRoot/Update-SdkVersion.constants.ps1"
    CheckoutChangedFiles
}

Describe 'Update-SdkVersion ExistingBeta' {
    BeforeEach {
        ."$PSScriptRoot/Update-SdkVersion.constants.ps1"
        Set-Content -Path $AzVersionLocation -Value $AzVersionContentBeta
    }

    It "Increments beta version when no parameters are applied" {
        & $PSScriptRoot/../Update-SdkVersion.ps1

        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_STRING "1\.0\.0-beta\.2"$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_MAJOR 1$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_MINOR 0$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_PATCH 0$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_PRERELEASE "beta\.2"$'
        $AzVersionLocation | Should -Not -FileContentMatchExactly '^#undef AZ_SDK_VERSION_PRERELEASE$'
    }

    It "Sets version when using -NewVersionString with GA version" {
        & $PSScriptRoot/../Update-SdkVersion.ps1 -NewVersionString 1.2.3

        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_STRING "1\.2\.3"$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_MAJOR 1$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_MINOR 2$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_PATCH 3$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_PRERELEASE$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#undef AZ_SDK_VERSION_PRERELEASE$'
    }

    It "Sets version when using -NewVersionString with Beta version" {
        & $PSScriptRoot/../Update-SdkVersion.ps1 -NewVersionString 1.2.3-beta.1

        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_STRING "1\.2\.3-beta\.1"$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_MAJOR 1$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_MINOR 2$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_PATCH 3$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_PRERELEASE "beta\.1"$'
        $AzVersionLocation | Should -Not -FileContentMatchExactly '^#undef AZ_SDK_VERSION_PRERELEASE$'
    }

    AfterAll {
        ."$PSScriptRoot/Update-SdkVersion.constants.ps1"
        CheckoutChangedFiles
    }
}


Describe 'Update-SdkVersion ExistingGA' {
    BeforeEach {
        ."$PSScriptRoot/Update-SdkVersion.constants.ps1"
        Set-Content -Path $AzVersionLocation -Value $AzVersionContentGa
    }

    It "Increments beta version when no parameters are applied" {
        & $PSScriptRoot/../Update-SdkVersion.ps1

        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_STRING "1\.1\.0-beta\.1"$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_MAJOR 1$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_MINOR 1$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_PATCH 0$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_PRERELEASE "beta\.1"$'
        $AzVersionLocation | Should -Not -FileContentMatchExactly '^#undef AZ_SDK_VERSION_PRERELEASE$'
    }

    It "Sets version when using -NewVersionString with GA version" {
        & $PSScriptRoot/../Update-SdkVersion.ps1 -NewVersionString 1.2.3

        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_STRING "1\.2\.3"$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_MAJOR 1$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_MINOR 2$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_PATCH 3$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_PRERELEASE$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#undef AZ_SDK_VERSION_PRERELEASE$'
    }

    It "Sets version when using -NewVersionString with Beta version" {
        & $PSScriptRoot/../Update-SdkVersion.ps1 -NewVersionString 1.2.3-beta.1

        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_STRING "1\.2\.3-beta\.1"$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_MAJOR 1$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_MINOR 2$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_PATCH 3$'
        $AzVersionLocation | Should -FileContentMatchExactly '^#define AZ_SDK_VERSION_PRERELEASE "beta\.1"$'
        $AzVersionLocation | Should -Not -FileContentMatchExactly '^#undef AZ_SDK_VERSION_PRERELEASE$'
    }

    AfterAll {
        ."$PSScriptRoot/Update-SdkVersion.constants.ps1"
        CheckoutChangedFiles
    }
}
