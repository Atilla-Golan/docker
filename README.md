Embedded Linux Docker project
This project includes C coding practices and embedded system concepts of Docker and build process.
The goal is to create a client C programs that demonstrates the usage of scanning a server's 3 TCP ports and printing the published data.
Another C code is written to control the server with sending data to the server over a UDP port and observing them by printing them with given amplitudes and frequencies.

Preparation
clone the github repo! bash: git clone (https://github.com/Atilla-Golan/docker.git)

Load the Docker image that behaves as server:

docker load < fsw-linux-homework.tar.gz
Run it:

docker run -p 4000:4000/udp -p 4001:4001 -p 4002:4002 -p 4003:4003 fsw-linux-homework
in the second terminal, build and start a client container from dockerfile in this directory:

bash: docker build -t myapp .

run it:

bash: docker run -it --rm myapp ./client1 (client2 for the second program)

you can use below to stop (Ctrl+c), remove running container and images related to our project:
bash: docker container ls // to see the container ID

bash: docker remove -f <container_id>

make clean

Example output fro client1:

{"timestamp": 1709286246830, "out1": "-4.8", "out2": "8.0", "out3": "--"}
{"timestamp": 1709286246930, "out1": "-4.0", "out2": "--",  "out3": "1.0"}
{"timestamp": 1709286247030, "out1": "-2.9", "out2": "1.2", "out3": "--"}
{"timestamp": 1709286247130, "out1": "-1.5", "out2": "--",  "out3": "--"}
...
{"timestamp": 1709286257730, "out1": "5.0",  "out2": "--",  "out3": "2.5"}
{"timestamp": 1709286257830, "out1": "4.8",  "out2": "5.0", "out3": "--"}
{"timestamp": 1709286257930, "out1": "4.0",  "out2": "--",  "out3": "--"}

What are the frequencies, amplitues and shapes you see on the server outputs?

The amplitude varies around -5 to 5. The frequency seems to be related to the repeat times of peak value = 13 lines (every 13 lines of printed value, you can see peak or deep value) which is T=period= 13*100ms =1.3s then f=1/T= 1/1.3 = 0.76Hz.

The data values printed shows the signal seems to be a sinusiod wave.
