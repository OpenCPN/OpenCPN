#!/usr/bin/env python3

# websocket send client

import argparse
import asyncio
import websockets


async def send(url, path):
    async with websockets.connect(url, ping_timeout=None, ping_interval=None) as ws:
        with open(path, 'rb') as f:
            message = f.read()

        print('Sending message...')
        await ws.send(message)
        print('Message sent.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='websocket proxy.')
    parser.add_argument('--path', help='Path to the file to send.',
                        default='small_file')
    parser.add_argument('--url', help='Remote websocket url',
                        default='wss://echo.websocket.org')
    args = parser.parse_args()

    asyncio.get_event_loop().run_until_complete(send(args.url, args.path))
