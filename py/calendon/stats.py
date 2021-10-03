from typing import Tuple
import sqlite3
import os

from calendon.context import ProjectContext


def establish_connection(ctx: ProjectContext):
    new_db = not os.path.exists(os.path.join(ctx.calendon_home(), 'stats.db'))
    connection = sqlite3.connect('stats.db')
    if new_db:
        print('Creating a new database.')
        connection.execute("""CREATE TABLE COMMANDS
            (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
            command TEXT,
            args TEXT,
            ms INTEGER)
            """)
    return connection


def command_add_time(ctx: ProjectContext, command, args, elapsed_time_ms: int):
    connection = establish_connection(ctx)
    with connection:
        query = """INSERT INTO COMMANDS (command, args, ms) VALUES (?, ?, ?)"""
        connection.execute(query, (command, args, elapsed_time_ms))


def command_get_times(ctx: ProjectContext, command: str, args: str) -> Tuple[int, int, int]:
    connection = establish_connection(ctx)
    with connection:
        query = """SELECT min(ms), avg(ms), max(ms) FROM COMMANDS where command = ? AND args = ?"""
        cursor = connection.cursor()
        cursor.execute(query, (command, args))
        return cursor.fetchone()
