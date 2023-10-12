import { start_websocket, Manager, MessageData } from '../pkg';

function myCallback(msg) {
    console.log({msg});
}

console.log("hello");

const manager = new Manager();
manager.add_callback(myCallback);

start_websocket(manager, "ws://tom-desktop:9001");
