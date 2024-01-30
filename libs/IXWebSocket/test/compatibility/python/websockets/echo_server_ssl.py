#!/usr/bin/env python

# WS server example

import asyncio
import os
import pathlib
import ssl
import websockets


async def echo(websocket, path):
    msg = await websocket.recv()
    print(f'Received {len(msg)} bytes')
    await websocket.send(msg)

ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
ssl_context.load_cert_chain('trusted-server-crt.pem',
                            'trusted-server-key.pem')

host = os.getenv('BIND_HOST', 'localhost')
print(f'Serving on {host}:8766')

start_server = websockets.serve(echo, host, 8766, max_size=2 ** 30, ssl=ssl_context)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
