docker compose down -v --rmi all --remove-orphans
docker builder prune -af
docker system prune -af --volumes
docker compose build --no-cache
docker compose up --force-recreate