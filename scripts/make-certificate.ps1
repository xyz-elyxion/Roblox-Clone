#Requires -Version 7
<#
    Makes the Linear code-signing certificate.

    Run this once, on the machine that will sign releases. It writes two files:

      certs/linear-codesign.cer   the public certificate - commit this, ship it in the package
      certs/linear-codesign.pfx   the private key - never commit, never ship

    The .pfx is what signs; the .cer is what LinearCert.exe installs so Windows trusts the
    result. Both are ignored by git except the .cer, so a clone cannot leak the key by accident.

    Anyone holding the .pfx can produce a program that Windows calls "Linear". Treat it like a
    password, and delete it from a machine that no longer signs releases.

    Example:
      ./scripts/make-certificate.ps1
      ./scripts/make-certificate.ps1 -Subject 'CN=Linear QA' -Years 2 -OutDir D:\keys
#>
[CmdletBinding()]
param(
    [string]$Subject = 'CN=Linear, O=Linear, C=US',
    [string]$OutDir = '',
    [int]$Years = 10,
    [string]$PfxPassword = '',
    [switch]$Force
)

$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($OutDir)) {
    $OutDir = Join-Path (Split-Path -Parent $PSScriptRoot) 'certs'
}
if (-not (Test-Path -LiteralPath $OutDir)) {
    New-Item -ItemType Directory -Path $OutDir | Out-Null
    Write-Host "created $OutDir"
}

$cerPath = Join-Path $OutDir 'linear-codesign.cer'
$pfxPath = Join-Path $OutDir 'linear-codesign.pfx'

if ((Test-Path -LiteralPath $pfxPath) -and -not $Force) {
    # Overwriting a key that signed releases means nothing already shipped verifies against
    # the new one, so this is a deliberate act, not something a second run should do quietly.
    throw "$pfxPath already exists. Pass -Force to replace it, and remember that a replacement key does not verify earlier releases."
}

# A code-signing EKU (1.3.6.1.5.5.7.3.3) is what makes the certificate usable for Authenticode
# and nothing else. -Type CodeSigningCert adds it and keeps the certificate out of the CA
# role, so it cannot be used to sign other certificates.
$extensions = @('2.5.29.37={text}1.3.6.1.5.5.7.3.3')

Write-Host "creating a $Years-year code-signing certificate for '$Subject'"
$certificate = New-SelfSignedCertificate `
    -Subject $Subject `
    -Type CodeSigningCert `
    -KeyAlgorithm RSA `
    -KeyLength 3072 `
    -HashAlgorithm SHA256 `
    -KeyUsage DigitalSignature `
    -TextExtension $extensions `
    -NotAfter (Get-Date).AddYears($Years) `
    -CertStoreLocation 'Cert:\CurrentUser\My'

Export-Certificate -Cert $certificate -FilePath $cerPath -Type CERT | Out-Null
Write-Host "wrote $cerPath (commit this one)"

if ([string]::IsNullOrWhiteSpace($PfxPassword)) {
    # The password only protects the file at rest on this machine; it is written beside it so
    # signing can run unattended. Move both somewhere safer if this machine is shared.
    $bytes = New-Object byte[] 24
    [System.Security.Cryptography.RandomNumberGenerator]::Create().GetBytes($bytes)
    $PfxPassword = [Convert]::ToBase64String($bytes)
    $passwordPath = Join-Path $OutDir 'linear-codesign.password'
    Set-Content -LiteralPath $passwordPath -Value $PfxPassword -NoNewline
    Write-Host "wrote $passwordPath (local only, ignored by git)"
}

$secure = ConvertTo-SecureString -String $PfxPassword -Force -AsPlainText
Export-PfxCertificate -Cert $certificate -FilePath $pfxPath -Password $secure | Out-Null
Write-Host "wrote $pfxPath (never commit this one)"

Write-Host ''
Write-Host "thumbprint: $($certificate.Thumbprint)"
Write-Host "expires:    $($certificate.NotAfter.ToString('yyyy-MM-dd'))"
Write-Host ''
Write-Host 'Next steps:'
Write-Host '  1. Sign a folder:  ./scripts/sign-binaries.ps1 -Folder dist/linear-windows-release'
Write-Host '  2. Trust it here:  ./LinearCert.exe install'
Write-Host '  3. In CI, store the .pfx and its password as secrets and pass them to sign-binaries.'
