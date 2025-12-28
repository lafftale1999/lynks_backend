$body = @{
  users = @{
    username = "testuser"
    password = "test123"
  }
} | ConvertTo-Json -Depth 5

Invoke-RestMethod `
  -Uri "http://127.0.0.1:60000/login" `
  -Method POST `
  -ContentType "application/json" `
  -Body $body