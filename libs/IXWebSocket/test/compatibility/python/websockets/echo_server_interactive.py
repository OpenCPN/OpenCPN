#!/usr/bin/env python

# WS server example

import asyncio
import websockets

async def hello(websocket, path):
    await websocket.send(f"> Welcome !")

    name = await websocket.recv()
    print(f"< {name}")

    greeting = f"Hello {name}!"

    await websocket.send(greeting)
    print(f"> {greeting}")

async def echo(websocket, path):
    msg = await websocket.recv()
    print(f'Received {len(msg)} bytes')
    await websocket.send(msg)

print('Serving on localhost:8766')
start_server = websockets.serve(echo, 'localhost', 8766)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
