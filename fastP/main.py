from datetime import datetime
import uvicorn
from pathlib import Path
from fastapi import FastAPI
import sqlite3
from fastapi.responses import HTMLResponse

BASE = Path(__file__).resolve().parent
app = FastAPI(title="Device Plastform")

DB = BASE / "data.db"
HTML_FILE = BASE.parent / "temple" / "first.html"

def init_db():
    conn = sqlite3.connect(DB)
    conn.execute("""
    CREATE TABLE IF NOT EXISTS status_log(
     id INTEGER PRIMARY KEY AUTOINCREMENT ,
     devices TEXT NOT NULL ,
     ts TEXT NOT NULL 
    )
    """)
    conn.commit()
    conn.close()

init_db()

@app.get("/")
def index():
    return {
        "serives" : "device-platform" ,
        "history" : "/api/history" ,
        "docs" : "/docs"
    }

@app.get("/api/status")
def get_status():
         conn = sqlite3.connect(DB)
         row = conn.execute(
             "select devices , ts from  status_log order by id **desc** limit 1"
         ).fetchone()

         count = conn.execute(
             "select count(*) from status_log"
                              ).fetchone()[0]
         conn.close()
         if row is None :
             return {"device" : "None" , "online" : False , "count" : 0 , "server_time" : None}
         return {"device" : row[0] , "online" : True , 	"count" : count , "server_time" : row[1]}

@app.post("/api/report")
def report(device : str , ts : str |  None = None) :
    ts = ts or datetime.now().isoformat(timespec="seconds")
    conn = sqlite3.connect(DB)
    conn.execute("insert into status_log (devices , ts) values (?,?)" , (device ,ts))
    conn.commit()
    conn.close()
    return {
        "ok" : True,
        "device" : device ,
        "ts" : ts
    }
@app.get("/api/history")
def get_history(limit : int = 10):
    conn = sqlite3.connect(DB)
    conn.row_factory = sqlite3.Row
    rows = conn.execute(
        "select id , devices , ts from status_log order by id desc limit ?" ,(limit,)
    ).fetchall()
    conn.close()
    return {
        "count" : len(rows) ,
        "items" : [dict(r) for r in rows]
    }

@app.get("/panel" , response_class=HTMLResponse)
def panel():
    return HTML_FILE.read_text(encoding="utf-8")

if __name__ == "__main__":
    uvicorn.run("main:app", host="127.0.0.1", port=8080, reload=True)



