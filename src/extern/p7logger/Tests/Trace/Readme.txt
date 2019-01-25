This project is used to test:
- P7 engine (client/server)
- P7.Trace engine
- Baical server

Console arguments examples:

1. Creating and destroing 3000 times P7 object and P7.Trace object. During every iteration
   test send 5000 messages to server 127.0.0.1

<Test executable> /P7.Addr=127.0.0.1 /P7.PSize=1472 /P7.Pool=2048 /P7.Verb=1 /Test=1 /Count=3000


2. Sending 3 trace streams simultaneously, with approximately speed 10 thousands 
   traces per seconds to server 127.0.0.1

<Test executable> /P7.Addr=127.0.0.1 /P7.PSize=1472 /P7.Pool=2048 /P7.Verb=1 /Test=2 /Speed=10 /Count=3 



