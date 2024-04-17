#!/usr/bin/env python

# WS server example

import asyncio
import os
import websockets


async def echo(websocket, path):
    while True:
        msg = await websocket.recv()
        print(f'Received {len(msg)} bytes')
        await websocket.send(msg)

host = os.getenv('BIND_HOST', 'localhost')
print(f'Serving on {host}:8766')

start_server = websockets.serve(echo, host, 8766, max_size=2 ** 30)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
