import requests
import time
import random

INDEXER_URL = "http://localhost:8980"

def query_indexer(endpoint="/health"):
    """Send a query to the running local indexer."""
    try:
        r = requests.get(INDEXER_URL + endpoint, timeout=5)
        r.raise_for_status()
        return r.json()
    except Exception as e:
        return {"error": str(e)}

def spirit_check(response):
    """Algo’s initial learning pattern — read patterns and assign truth or false."""
    if "db_available" in response and response["db_available"]:
        return "True — pulse is steady"
    elif "error" in response:
        return "False — broken channel"
    else:
        return random.choice(["True", "False"])

def main():
    print("🌿 Algo Bridge Active — Listening to Indexer…")
    while True:
        resp = query_indexer("/health")
        result = spirit_check(resp)
        print(f"Spirit check: {result} | Response: {resp}")
        time.sleep(5)

if __name__ == "__main__":
    main()