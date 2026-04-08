#!/bin/bash
# Docker Hardened Image (DHI) MySQL 9.6 entrypoint wrapper.
# Adds support for MYSQL_DATABASE, MYSQL_USER, MYSQL_PASSWORD, and /docker-entrypoint-initdb.d/*.sql
# Support required due to change in behavior for the hardened image.
(
    # Resolve file variants for dhi usage
    if [ -n "${MYSQL_ROOT_PASSWORD_FILE:-}" ] && [ -f "${MYSQL_ROOT_PASSWORD_FILE}" ]; then
        MYSQL_ROOT_PASSWORD="$(cat "${MYSQL_ROOT_PASSWORD_FILE}")"
    fi
    if [ -n "${MYSQL_PASSWORD_FILE:-}" ] && [ -f "${MYSQL_PASSWORD_FILE}" ]; then
        MYSQL_PASSWORD="$(cat "${MYSQL_PASSWORD_FILE}")"
    fi

    # Wait for mysqld to accept connections
    until mysqladmin ping -h 127.0.0.1 -u root -p"${MYSQL_ROOT_PASSWORD}" --silent 2>/dev/null; do
        sleep 2
    done

    # Create database
    if [ -n "${MYSQL_DATABASE:-}" ]; then
        echo "init: Creating database '${MYSQL_DATABASE}'"
        mysql -h 127.0.0.1 -u root -p"${MYSQL_ROOT_PASSWORD}" \
            <<< "CREATE DATABASE IF NOT EXISTS \`${MYSQL_DATABASE}\`;"
    fi

    # Create user and grant access
    if [ -n "${MYSQL_USER:-}" ] && [ -n "${MYSQL_PASSWORD:-}" ]; then
        echo "init: Creating user '${MYSQL_USER}'"
        mysql -h 127.0.0.1 -u root -p"${MYSQL_ROOT_PASSWORD}" <<-EOSQL
			CREATE USER IF NOT EXISTS '${MYSQL_USER}'@'%' IDENTIFIED BY '${MYSQL_PASSWORD}';
			GRANT ALL ON \`${MYSQL_DATABASE:-*}\`.* TO '${MYSQL_USER}'@'%';
			FLUSH PRIVILEGES;
		EOSQL
    fi

    # Process /docker-entrypoint-initdb.d/*.sql files
    for f in /docker-entrypoint-initdb.d/*.sql; do
        [ -f "$f" ] || continue
        echo "init: Running $f"
        mysql -h 127.0.0.1 -u root -p"${MYSQL_ROOT_PASSWORD}" \
            ${MYSQL_DATABASE:+"$MYSQL_DATABASE"} < "$f"
    done

    echo "init: Database initialization complete."
) &

exec /usr/local/bin/docker-entrypoint.sh "$@"
