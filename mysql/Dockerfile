FROM mysql:8.0

# Kopiera init-sql till MySQLs auto-init-mapp
# Allt som ligger här körs i alfabetisk ordning vid första start (när datavolymen är tom).
COPY ./scripts/ /docker-entrypoint-initdb.d/

# (valfritt) säkerställ läsbart
RUN chmod -R 644 /docker-entrypoint-initdb.d/*.sql