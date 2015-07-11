#!/usr/bin/env python

import asyncio
import websockets

@asyncio.coroutine
def hello():
    while True:
    	websocket = yield from websockets.connect('ws://localhost:8765/')
    	name = input("What's your name? ")
    	yield from websocket.send(name)
    	print("> {}".format(name))
    	greeting = yield from websocket.recv()
    	print("< {}".format(greeting))
    	cont = input("Want to continue? y/n ")
    	if not cont is 'y':
    		break
    		websocket.close()

asyncio.get_event_loop().run_until_complete(hello())