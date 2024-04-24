import asyncio
import websockets

async def echo(websocket):
  async for message in websocket:
    print(f"Received message: {message}")
    await websocket.send(message)

async def main():
  async with websockets.serve(echo, "127.0.0.1", 8002):
    await asyncio.Future()  # run the server forever

if __name__ == "__main__":
  asyncio.run(main())
