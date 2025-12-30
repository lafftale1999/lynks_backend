#!/bin/bash
mysql -u root -p <<EOF
SOURCE scripts/01_create_db.sql;
SOURCE scripts/02_create_admin.sql;
SOURCE scripts/03_create_tables.sql;
SOURCE scripts/04_seed.sql;
EOF