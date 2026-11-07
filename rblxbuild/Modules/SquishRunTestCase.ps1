# Runs one Squish test case: start the server, run the case, stop the server
# again. This is the Windows counterpart of SquishRunTestCase.sh, called from
# SquishTestScript; it used to be a .bat file.
param(
    [string]$Server = '',
    [string]$Runner = '',
    [string]$TestCase = '',
    [string]$Wrapper = '',
    [string]$Aut = ''
)

Write-Host 'Starting the squish server...'
Start-Process -FilePath $Server | Out-Null

Write-Host 'Running the test case...'
$arguments = @('--testcase', $TestCase)
if ($Wrapper) { $arguments += @('--wrapper', $Wrapper) }
if ($Aut) { $arguments += @('--aut', $Aut) }
& $Runner @arguments
$result = $LASTEXITCODE

Write-Host 'Stopping the squish server...'
& $Server --stop

if ($null -eq $result) { $result = 1 }
exit $result
