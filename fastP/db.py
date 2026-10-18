#数据库的管理
import sqlite3
from fileinput import close
from pathlib import Path

DB_PATH = Path(__file__).resolve().parent / "data.db"

def connect() -> sqlite3.Connection: #-> mean is what return
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn

def init_db() -> None:
    """建表"""
    conn = connect()
    conn.execute("""
    CREATE TABLE IF NOT EXISTS status_log(
    id INTEGER PRIMARY KEY AUTOINCREMENT ,
    devices TEXT NOT NULL,
    value REAL ,
    ts TEXT  NOT NULL
    )
    """)
    cols = [r[1] for r in conn.execute("PRAGMA table_info(status_log)")]
    if "value" not in cols :
        conn.execute("ALTER TABLE status_log ADD COLUM value REAL")
    conn.commit()
    conn.close()

def insert_report(device :str , value , ts : str) -> int :
    conn = connect()
    cur = conn.execute(
        "insert into status_log (devices , value , ts) values (?,?,?)" , (device , value , ts) ,
    )
    new_id = cur.lastrowid
    conn.commit()
    conn.close()
    return new_id

def latest():
    conn = connect()
    row = conn.execute(
        "SELECT devices , value , ts FROM status_log ORDER BY id DESC LIMIT 1"
    ).fetchone()
    conn.close()
    return dict(row) if row else None

def count_all() -> int:
    conn = connect()
    n = conn.execute("select count(*) from status_log").fetchone()[0]
    conn.close()
    return n

def recent(limit: int) :
    conn = connect()
    rows = conn.execute(
        "select id , devices , values , ts from status_log order by id desc limit ?",(limit,),
    ).fetchall()
    conn.close()
    return [dict(r) for r in rows]

