CREATE USER IF NOT EXISTS 'lynks_admin'@'localhost' IDENTIFIED BY 'admin123';
GRANT ALL PRIVILEGES ON lynks_db.* TO 'lynks_admin'@'localhost';
FLUSH PRIVILEGES;