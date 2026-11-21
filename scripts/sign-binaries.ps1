<#
    Signs the executables and DLLs in a folder with the Linear code-signing certificate.

    Signing embeds the certificate and a signature in each .exe and .dll, so Windows can tell who
    published it and that nobody edited it afterwards. It is also what an antivirus checks
    before it decides whether a program is a repackaged copy of something it already knows.

    The certificate itself:
      -Folder       the packaged folder (default dist/linear-windows-release)
      -Pfx          the private key, certs/linear-codesign.pfx
      -Password     its password; read from certs/linear-codesign.password when omitted
      -Thumbprint   use a certificate already in Cert:\CurrentUser\My instead of a .pfx

    Everything is signed with SHA-256 and a timestamp, so a signature stays valid after the
    certificate expires. Pass -NoTimestamp on a machine with no internet.

    In CI, keep the .pfx and password in repository secrets and pass them in; a release built
    without them is unsigned and the packager says so.

    Example:
      ./scripts/sign-binaries.ps1
      ./scripts/sign-binaries.ps1 -Folder dist/linear-windows-release -NoTimestamp
#>
[CmdletBinding()]
param(
    [string]$Folder = '',
    [string]$Pfx = '',
    [string]$Password = '',
    [string]$Thumbprint = '',
    [string]$TimestampUrl = 'http://timestamp.digicert.com',
    [switch]$NoTimestamp
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot

if ([string]::IsNullOrWhiteSpace($Folder)) { $Folder = Join-Path $root 'dist/linear-windows-release' }
if (-not (Test-Path -LiteralPath $Folder)) { throw "No such folder: $Folder" }

# Resolve the certificate: a .pfx (with its password) or one already installed.
$certificate = $null
if (-not [string]::IsNullOrWhiteSpace($Thumbprint)) {
    $certificate = Get-ChildItem -Path "Cert:\CurrentUser\My\$Thumbprint" -ErrorAction SilentlyContinue
    if ($null -eq $certificate) { throw "No certificate $Thumbprint in Cert:\CurrentUser\My" }
} else {
    if ([string]::IsNullOrWhiteSpace($Pfx)) { $Pfx = Join-Path $root 'certs/linear-codesign.pfx' }
    if (-not (Test-Path -LiteralPath $Pfx)) {
        throw "No signing key. Run ./scripts/make-certificate.ps1 first, or pass -Pfx / -Thumbprint."
    }
    if ($Pfx -like '*.cer') {
        # A .cer is the public half: it can verify but never sign, so say that instead of
        # failing later with a signature of zero bytes.
        throw "$Pfx is the public certificate. Signing needs the .pfx from make-certificate.ps1."
    }
    if ([string]::IsNullOrWhiteSpace($Password)) {
        $passwordPath = Join-Path $root 'certs/linear-codesign.password'
        if (Test-Path -LiteralPath $passwordPath) { $Password = (Get-Content -LiteralPath $passwordPath -Raw).Trim() }
    }
    $certificate = New-Object System.Security.Cryptography.X509Certificates.X509Certificate2(
        (Resolve-Path -LiteralPath $Pfx).Path, $Password,
        [System.Security.Cryptography.X509Certificates.X509KeyStorageFlags]::EphemeralKeySet)
}

if (-not $certificate.HasPrivateKey) {
    throw "That certificate has no private key, so it cannot sign."
}

# The package ships certs/linear-codesign.cer so LinearCert.exe has something to install. A
# signature made with a different key would be trusted nowhere, so compare before signing
# rather than after uploading.
$publicPath = Join-Path $root 'certs/linear-codesign.cer'
if (Test-Path -LiteralPath $publicPath) {
    $shipped = New-Object System.Security.Cryptography.X509Certificates.X509Certificate2(
        (Resolve-Path -LiteralPath $publicPath).Path)
    if ($shipped.Thumbprint -ne $certificate.Thumbprint) {
        throw "This key ($($certificate.Thumbprint)) does not match the shipped certificate $publicPath ($($shipped.Thumbprint)). Replace both, or sign with the key that matches."
    }
}

Write-Host "signing with $($certificate.Subject) ($($certificate.Thumbprint))"

# LinearGame.dll is loaded into Client.exe, Server.exe, Studio.exe and AiTrainer.exe.
# An unsigned DLL in a signed process is what an antivirus and SmartScreen report, so
# the library is signed with the same certificate and timestamp as the executables.
$files = @(Get-ChildItem -LiteralPath $Folder -File |
    Where-Object { $_.Extension -in @('.exe', '.dll') })
if ($files.Count -eq 0) { throw "No .exe or .dll files in $Folder" }

$failed = @()
foreach ($file in $files) {
    $parameters = @{
        FilePath        = $file.FullName
        Certificate     = $certificate
        HashAlgorithm   = 'SHA256'
    }
    if (-not $NoTimestamp) { $parameters.TimestampServer = $TimestampUrl }

    try {
        $signature = Set-AuthenticodeSignature @parameters
    } catch {
        # A timestamp server that cannot be reached must not leave the file unsigned, so
        # retry once without one rather than failing the whole folder.
        Write-Warning "$($file.Name): timestamp failed ($($_.Exception.Message)); signing without one"
        $parameters.Remove('TimestampServer')
        $signature = Set-AuthenticodeSignature @parameters
    }

    if ($signature.Status -ne 'Valid') {
        $failed += "$($file.Name): $($signature.StatusMessage)"
        Write-Warning "$($file.Name): $($signature.Status)"
    } else {
        Write-Host "  signed $($file.Name)"
    }
}

if ($failed.Count -gt 0) {
    # A throw rather than exit: package-build.ps1 calls this and turns the failure into a
    # warning, while a CI step that runs it directly still fails the job.
    throw ("Signed $($files.Count - $failed.Count) of $($files.Count) file(s):`n  " + ($failed -join "`n  "))
}

Write-Host "signed $($files.Count) file(s) in $Folder"
Write-Host 'Run LinearCert.exe install from that folder so Windows trusts the publisher.'
