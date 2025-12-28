param(
    [Parameter(Mandatory = $true)]
    [string]$Token
)

$headers = @{
    Authorization = "$Token"
}

Invoke-RestMethod `
  -Uri "http://127.0.0.1:60000/create" `
  -Method POST `
  -ContentType "application/json" `
  -Headers $headers