const net = require('net');

const PORT = 23;
const HOST = '0.0.0.0';

const server = net.createServer((socket) => {
  console.log('Client connected');

  socket.on('data', (data) => {
    if (data === null) {
      return
    }
    console.log(data.toString());
  });

  socket.on('end', () => {
    console.log('Client disconnected');
  });

  socket.on('error', (err) => {
    console.error('Socket error:', err);
  });
});

server.listen(PORT, HOST, () => {
  console.log(`Server listening on ${HOST}:${PORT}`);
});
