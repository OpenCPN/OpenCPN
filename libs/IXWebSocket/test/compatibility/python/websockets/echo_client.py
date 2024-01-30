#!/usr/bin/env python3

# websocket send client

import argparse
import asyncio
import websockets


async def send(url):
    async with websockets.connect(url) as ws:
        while True:
            message = input('> ')
            print('Sending message...')
            await ws.send(message)
            print('Message sent.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='websocket proxy.')
    parser.add_argument('--url', help='Remote websocket url',
                        default='wss://echo.websocket.org')
    args = parser.parse_args()

    asyncio.get_event_loop().run_until_complete(send(args.url))
