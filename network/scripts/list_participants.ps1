param(
    [Parameter(Mandatory = $true)]
    [string]$Token,

    [Parameter(Mandatory = $true)]
    [int]$Room
)

$headers = @{
    Authorization = "$Token"
}

$body = @{
    room_id = $Room
} | ConvertTo-Json -Depth 5

Invoke-RestMethod `
  -Uri "http://127.0.0.1:60000/list_participants" `
  -Method POST `
  -ContentType "application/json" `
  -Headers $headers
  -Body $body