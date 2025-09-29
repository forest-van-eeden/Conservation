#!/usr/bin/env python3
# push_hash.py — push a sys_hash as a note on Algorand (TestNet)
import sys
try:
    from algosdk import algod, transaction
except Exception as e:
    print("algosdk not available. Install with: pip3 install py-algorand-sdk")
    raise

ALGOD_ADDRESS = "https://testnet-algorand.api.purestake.io/ps2"  # example provider
ALGOD_TOKEN   = "YOUR_API_KEY"  # set your PureStake or node API key
HEADERS = {"X-API-Key": ALGOD_TOKEN}

SENDER = "YOUR_ADDRESS"
SENDER_SK = "YOUR_PRIVATE_KEY"  # mnemonic-derived or raw private key; be safe!

if len(sys.argv) < 2:
    print("Usage: push_hash.py <64-hex-hash>")
    sys.exit(1)

h = sys.argv[1]
if len(h) != 64:
    print("Warning: hash length not 64 chars. Proceeding anyway.")

client = algod.AlgodClient(ALGOD_TOKEN, ALGOD_ADDRESS, headers=HEADERS)
params = client.suggested_params()
# dummy receiver = sender (self-note)
receiver = SENDER
amt = 1000  # microAlgos = 0.001 Algo
txn = transaction.PaymentTxn(sender=SENDER, sp=params, receiver=receiver, amt=amt, note=h.encode())
signed = txn.sign(SENDER_SK)
txid = client.send_transaction(signed)
print(f"Pushed hash {h} as note, txid={txid}")
