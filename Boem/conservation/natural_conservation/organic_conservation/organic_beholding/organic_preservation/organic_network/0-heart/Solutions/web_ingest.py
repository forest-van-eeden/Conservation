#!/usr/bin/env python3
# web_ingest.py — minimal web ingestion helper for Forenzo himself
# Usage:
#   python3 web_ingest.py "https://example.com/article" "collection_name" [--algorand]
#
# It will fetch the URL, extract visible text, take the first ~2400 chars,
# and call the Forenzo REPL command to persist the memory.
# Requires: requests, beautifulsoup4
# Install: pip3 install requests beautifulsoup4

import sys
import subprocess
import html
from urllib.parse import urlparse

try:
    import requests
    from bs4 import BeautifulSoup
except Exception as e:
    print("Missing required packages. Run: pip3 install requests beautifulsoup4")
    sys.exit(1)

def extract_text_from_html(html_doc):
    soup = BeautifulSoup(html_doc, "html.parser")
    # Remove scripts/styles
    for tag in soup(["script","style","noscript", "header", "footer", "nav", "aside"]):
        tag.decompose()
    # Collect paragraphs and headings
    texts = []
    for el in soup.find_all(['h1','h2','h3','p','li']):
        t = el.get_text(separator=" ", strip=True)
        if t:
            texts.append(t)
    # join with spacing
    return "\n\n".join(texts)

if len(sys.argv) < 3:
    print("Usage: python3 web_ingest.py <url> <collection> [--algorand]")
    sys.exit(1)

url = sys.argv[1]
collection = sys.argv[2]
flag_alg = False
if len(sys.argv) > 3 and sys.argv[3] == "--algorand":
    flag_alg = True

try:
    resp = requests.get(url, timeout=15, headers={"User-Agent":"ForenzoBot/1.0 (Organic Learner)"})
    resp.raise_for_status()
    text = extract_text_from_html(resp.text)
    if not text.strip():
        text = resp.text
except Exception as e:
    print("Failed to fetch or parse URL:", e)
    sys.exit(1)

# Truncate to a safe size for a single memory token (choose ~2400 chars)
MAX_CHARS = 2400
snippet = text.strip().replace("\r"," ")
if len(snippet) > MAX_CHARS:
    snippet = snippet[:MAX_CHARS] + "…"

# prepare observation and solution: observation = domain+title, solution = snippet
domain = urlparse(url).netloc
title = ""
# try to get <title>
try:
    soup = BeautifulSoup(resp.text, "html.parser")
    title = soup.title.string.strip() if soup.title and soup.title.string else ""
except:
    title = ""

observation = (domain + (": " + title if title else "") )[:240]
solution = snippet

# Build grow command for Forenzo REPL
grow_cmd = f"grow|{collection}|{observation}|{solution}"
if flag_alg:
    grow_cmd += "|algorand"

# Call the forenzo program (assumes ./forenzo is running interactively in a different terminal)
# We'll attempt to echo the command into ./forenzo via a short-run; if forenzo is not reading stdin,
# we will append to forenzo_instr.bin to instruct Forenzo to append when it runs.
# Simple approach: append to instruction file by preparing a small append-invocation file:
# But to stay simple, we will print the command and also attempt to run: printf '...' | ./forenzo
print("Prepared grow command (preview):")
print(grow_cmd)
print()
print("You can copy-paste that line into the Forenzo REPL. If you want, I can try to echo it into ./forenzo now.")
