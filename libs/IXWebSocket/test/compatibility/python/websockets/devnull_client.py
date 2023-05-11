#!/usr/bin/env python3

# websocket send client

import argparse
import asyncio
import websockets

try:
    import uvloop
    uvloop.install()
except ImportError:
    print('uvloop not available')
    pass

msgCount = 0

async def timer():
    global msgCount

    while True:
        print(f'Received messages: {msgCount}')
        msgCount = 0

        await asyncio.sleep(1)


async def client(url):
    global msgCount

    asyncio.ensure_future(timer())

    async with websockets.connect(url) as ws:
        async for message in ws:
            msgCount += 1


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='websocket proxy.')
    parser.add_argument('--url', help='Remote websocket url',
                        default='wss://echo.websocket.org')
    args = parser.parse_args()

    asyncio.get_event_loop().run_until_complete(client(args.url))
