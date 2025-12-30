param(
    [string]$BaseUri = "http://127.0.0.1:60000",
    [string]$Username = "testuser",
    [string]$Password = "test123"
)

$ErrorActionPreference = "Stop"

function Print-Response {
    param(
        [Parameter(Mandatory=$true)] $Response,
        [string]$Title = "RESPONSE"
    )

    Write-Host ""
    Write-Host "----- $Title -----"
    if ($null -eq $Response) {
        Write-Host "(empty response)"
        return
    }

    # Invoke-RestMethod returnerar ofta redan ett objekt
    # Visa tabell om det ser "flat" ut, annars lista allt
    try {
        $props = $Response.PSObject.Properties.Name
        if ($props.Count -le 8 -and ($props -notcontains "0")) {
            $Response | Format-Table -AutoSize | Out-String | Write-Host
        } else {
            $Response | Format-List * | Out-String | Write-Host
        }
    } catch {
        $Response | Out-String | Write-Host
    }
}

function Invoke-Api {
    param(
        [Parameter(Mandatory=$true)][string]$Method,
        [Parameter(Mandatory=$true)][string]$Uri,
        [hashtable]$Headers,
        [string]$Body,
        [string]$Title = "HTTP CALL"
    )

    Write-Host ""
    Write-Host "=============================="
    Write-Host $Title
    Write-Host "=============================="
    Write-Host "$Method $Uri"

    try {
        $resp = Invoke-RestMethod -Uri $Uri -Method $Method -ContentType "application/json" -Headers $Headers -Body $Body
        Print-Response -Response $resp -Title "RESULT"
        return $resp
    }
    catch {
        Write-Host ""
        Write-Host "----- ERROR -----"
        Write-Host $_.Exception.Message

        # Försök läsa ut body från HTTP-felet (om det finns)
        $ex = $_.Exception
        if ($ex.Response -and $ex.Response.GetResponseStream) {
            try {
                $reader = New-Object System.IO.StreamReader($ex.Response.GetResponseStream())
                $errBody = $reader.ReadToEnd()
                Write-Host ""
                Write-Host "----- ERROR BODY -----"
                Write-Host $errBody
            } catch { }
        }

        throw
    }
}

# 1) LOGIN
$loginBody = @{
    users = @{
        username = $Username
        password = $Password
    }
} | ConvertTo-Json -Depth 5

$loginResponse = Invoke-Api `
    -Method "POST" `
    -Uri "$BaseUri/login" `
    -Headers @{} `
    -Body $loginBody `
    -Title "1) LOGIN USER"

if (-not $loginResponse -or -not $loginResponse.PSObject.Properties.Match('token')) {
    throw "Login failed - token missing in response"
}

$token = [string]$loginResponse.token
Write-Host "Token extracted: $token"

# 2) CREATE MEETING
$headers = @{ Authorization = $token }

$createResponse = Invoke-Api `
    -Method "POST" `
    -Uri "$BaseUri/create" `
    -Headers $headers `
    -Body $null `
    -Title "2) CREATE MEETING"

if (-not $createResponse -or -not $createResponse.PSObject.Properties.Match('room_id')) {
    throw "Create meeting failed - room_id missing in response"
}

$roomId = $createResponse.room_id
Write-Host "RoomId extracted: $roomId"

# 3) LIST PARTICIPANTS
$listBody = @{
    room_id = $roomId
} | ConvertTo-Json -Depth 5

$listResponse = Invoke-Api `
    -Method "POST" `
    -Uri "$BaseUri/list_participants" `
    -Headers $headers `
    -Body $listBody `
    -Title "3) LIST PARTICIPANTS"

Write-Host ""
Write-Host "=============================="
Write-Host "DONE"
Write-Host "=============================="
