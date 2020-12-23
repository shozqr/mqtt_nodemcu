'use strict'

const aedes = require('aedes')();
const server = require('net').createServer(aedes.handle);
const httpServer = require('http').createServer()
const ws = require('websocket-stream')
const port = 1883;
const wsPort = 8888;

const sqlite3 = require('sqlite3');
const open = require('sqlite').open;
let db;

async function createTable() {
    try {
        await db.run(`CREATE TABLE IF NOT EXISTS mqtt_messages(id integer primary key autoincrement, 
            topic text not null, message text not null, created_at timestamp
             default (datetime('now', 'localtime')));`);

    } catch (err) {
        throw err;
    }
}

server.listen(port, async function () {
    console.log('server listening on port', port);
    try {
        db = await open({
            filename: 'mqtt.db',
            driver: sqlite3.Database
        })
        await createTable();
    } catch (err) {
        console.log(`error: ${err}`);
    }
});

ws.createServer({
    server: httpServer
}, aedes.handle)

httpServer.listen(wsPort, function () {
    console.log('websocket server listening on port', wsPort)
})

aedes.on('clientError', function (client, err) {
    console.log('client error', client.id, err.message, err.stack);
});

aedes.on('connectionError', function (client, err) {
    console.log('client error', client, err.message, err.stack);
});

aedes.on('publish', async function (packet, client) {
    if (client) {
        console.log(`message from client: ${client.id}`);
        console.log(`topic: ${packet.topic}`);
        console.log(`payload: ${packet.payload.toString()}`);
        try {
            await db.run(
                'INSERT INTO mqtt_messages (topic, message) VALUES (?, ?)',
                packet.topic, packet.payload.toString()
            );
        } catch (err) {
            console.log(`error: ${err}`);
        }
    }
});

aedes.on('subscribe', function (subscriptions, client) {
    if (client) {
        console.log('subscribe from client', subscriptions, client.id);
    }
});

aedes.on('client', function (client) {
    console.log('new client', client.id);
});