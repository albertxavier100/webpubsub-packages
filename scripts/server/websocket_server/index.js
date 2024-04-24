import { WebSocketServer } from 'ws';

const wss = new WebSocketServer({ port: 8002 });

wss.on('connection', function connection(ws) {
    console.log('new connection');

    ws.on('error', console.error);

    ws.on('message', function message(data) {
        console.log('received: %s', data);
    });

    ws.send('something');
});

wss.on('close', function close() {
    console.log('connection close')
})