from contextlib import asynccontextmanager
from datetime import datetime
from pathlib import Path

import uvicorn
from fastapi import FastAPI, Query
from fastapi.responses import HTMLResponse
from httpx2 import query

import db

BASE = Path(__file__).resolve().parent
HTML_FILE = BASE.parent / "temple" / "first.html"

@asynccontextmanager
async def lifespan(app : FastAPI) :
    db.init_db()
    yield

app = FastAPI(title="Device Platform" , version="0.2.0" , lifespan=lifespan)

@app.get("/")
def index() :
    return {
        "service": "device-platform",
        "version": app.version,
        "history": "/api/history",
        "docs": "/docs",
    }

@app.post("/api/report")
def report(device : str = Query(... , min_length = 1 , max_length=32 , description="设备名") ,
           value: float | None = Query(None, ge=-100, le=1000, description="测量值（如温度）"),
           ts : str | None = Query(None, max_length=32, description="时间戳，可省略") ,
           ):
    ts_ = ts or datetime.now().isoformat(timespec="seconds")
    row_id = db.insert_report(device , value , ts_)
    return {
        "ok" : True ,
        "id" : row_id ,
        "device" : device ,
        "value" : value ,
        "ts" : ts_
    }

@app.get("/api/status")
def get_status():
    row = db.latest()
    count = db.count_all()
    if row is None:
        return {"device": None, "value": None, "online": False,
                "count": 0, "server_time": None}
    return {
        "device": row["devices"],
        "value": row["value"],
        "online": True,
        "count": count,
        "server_time": row["ts"],
    }

@app.get("/api/history")
def get_history(limit: int = Query(10, ge=1, le=100, description="返回条数 1~100")):
    items = db.recent(limit)
    return {
        "count" : len(items) ,
        "items" : items
    }

@app.get("/panel", response_class=HTMLResponse)
def panel():
    return HTML_FILE.read_text(encoding="utf-8")


if __name__ == "__main__":
    uvicorn.run("main:app", host="0.0.0.0", port=8080, reload=True)