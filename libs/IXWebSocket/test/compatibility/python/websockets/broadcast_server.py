#!/usr/bin/env python

# WS server example

import asyncio
import os
import websockets


clients = set()


async def echo(websocket, path):
    clients.add(websocket)

    try:
        while True:
            msg = await websocket.recv()

            for ws in clients:
                if ws != websocket:
                    print(f'Sending {len(msg)} bytes to {ws}')
                    await ws.send(msg)
    except websockets.exceptions.ConnectionClosedOK:
        print('Client terminating')
        clients.remove(websocket)


host = os.getenv('BIND_HOST', 'localhost')
print(f'Serving on {host}:8766')

start_server = websockets.serve(echo, host, 8766, max_size=2 ** 30)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
