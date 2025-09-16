#!/usr/bin/env python3
import sys
from algosdk import algod, transaction, account, encoding

ALGOD_ADDRESS = "https://testnet-algorand.api.purestake.io/ps2"
ALGOD_TOKEN = "YOUR_API_KEY_HERE"
SENDER = "YOUR_ALGORAND_ADDRESS"
SENDER_KEY = "YOUR_PRIVATE_KEY"

if len(sys.argv) < 2:
    print("Usage: push_hash.py <hash>")
    sys.exit(1)

entry_hash = sys.argv[1]

# Connect to Algorand client
algod_client = algod.AlgodClient(ALGOD_TOKEN, ALGOD_ADDRESS)

# Prepare microtransaction (0.001 Algo) with hash in note
params = algod_client.suggested_params()
txn = transaction.PaymentTxn(
    sender=SENDER,
    sp=params,
    receiver=SENDER,
    amt=1000,  # 0.001 Algo in microAlgos
    note=entry_hash.encode()
)
signed_txn = txn.sign(SENDER_KEY)
txid = algod_client.send_transaction(signed_txn)
print(f"Pushed hash {entry_hash} to Algorand, txid: {txid}")