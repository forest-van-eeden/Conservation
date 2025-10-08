import requests
import time
import random
import json
from datetime import datetime
from pathlib import Path

INDEXER_URL = "http://localhost:8980"
MEMORY_FILE = Path("algo_memory.json")

def query_indexer(endpoint="/health"):
    try:
        r = requests.get(INDEXER_URL + endpoint, timeout=5)
        r.raise_for_status()
        return r.json()
    except Exception as e:
        return {"error": str(e)}

def spirit_check(response):
    if "db_available" in response and response["db_available"]:
        return "True — pulse steady"
    elif "error" in response:
        return "False — broken channel"
    else:
        return random.choice(["True", "False"])

def remember(entry):
    """Save each event in local JSON memory."""
    memory = []
    if MEMORY_FILE.exists():
        with open(MEMORY_FILE, "r") as f:
            try:
                memory = json.load(f)
            except json.JSONDecodeError:
                memory = []
    memory.append(entry)
    with open(MEMORY_FILE, "w") as f:
        json.dump(memory, f, indent=2)

def main():
    print("🌿 Algo Memory Active — Listening + Remembering")
    while True:
        resp = query_indexer("/health")
        result = spirit_check(resp)
        entry = {
            "time": datetime.now().isoformat(),
            "spirit": result,
            "response": resp
        }
        remember(entry)
        print(f"[{entry['time']}] {result}")
        time.sleep(5)

if __name__ == "__main__":
    main()