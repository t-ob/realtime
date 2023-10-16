import { start_websocket, Manager, MessageData } from '../pkg';
import Plotly from 'plotly.js-dist-min';

import { RECEIVER_WEBSOCKET_URL } from './constants';

function myCallback(msg: MessageData) {
  console.log({ msg });
}

let c = 0;
let lastX = 0;
let lastTestLossX = 0;
function myPlotlyCallback(msg: MessageData) {
  const { label, data } = msg;
  const [loss] = data;
  if (label === "train_loss") {
    lastX = c;
    const x = [[c]];
    const y = [[loss]]
    Plotly.extendTraces('myDiv', { x, y }, [0]);
    c += 1;
  } else if (label === "test_loss") {
    const x = [[lastX]];
    const y = [[loss]];
    Plotly.extendTraces('myDiv', { x, y }, [1]);
    lastTestLossX = lastX + 1;
  }
}

const manager = new Manager();
manager.add_callback(myCallback);
manager.add_callback(myPlotlyCallback);

start_websocket(manager, RECEIVER_WEBSOCKET_URL);

Plotly.newPlot('myDiv', [{
  x: [0],
  y: [null],
  mode: 'lines',
  line: { color: '#80CAF6' }
},
{
  x: [0],
  y: [null],
  mode: 'lines',
  line: { color: '#f6ac80' }
}]);
