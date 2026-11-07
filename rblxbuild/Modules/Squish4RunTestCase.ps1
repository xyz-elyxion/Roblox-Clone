# Runs one Squish 4 test case: tell the server which application is under test,
# start it in the background, run the case, stop it. This is the Windows
# counterpart of Squish4RunTestCase.sh, called from SquishTestScript; it used to
# be a .bat file.
param(
    [string]$Server = '',
    [string]$Runner = '',
    [string]$TestSuite = '',
    [string]$TestCase = '',
    [string]$Aut = '',
    [string]$AutDir = ''
)

& $Server --stop

Write-Host "Adding AUT... $Server --config addAUT $Aut $AutDir"
& $Server --config addAUT $Aut $AutDir

Write-Host "Starting the squish server... $Server"
Start-Process -FilePath $Server -WindowStyle Hidden | Out-Null

Write-Host "Running the test case... $Runner --testsuite $TestSuite --testcase $TestCase"
& $Runner --testsuite $TestSuite --testcase $TestCase
$result = $LASTEXITCODE

Write-Host "Stopping the squish server... $Server --stop"
& $Server --stop

if ($null -eq $result) { $result = 1 }
exit $result
