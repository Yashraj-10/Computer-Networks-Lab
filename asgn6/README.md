# Assignment 6: PingNetInfo

This program takes a site address (either a name like cse.iitkgp.ac.in or an IP address like 10.3.45.6), the number of times a probe will be sent per link (n), and the time difference between any two probes (T) as input and finds the route and estimates the latency and bandwidth of each link in the path.

## Prerequisites

- A Unix-based system
- GCC compiler
- Root access

## Usage

Compile the program using the following command:
```bash
gcc pingNetInfo.c -o pingnetinfo
```
Run the program with the following command:
```bash
sudo ./pingnetinfo <site_address> <n> <T>
```

Example usage:

```bash
sudo ./pingnetinfo cse.iitkgp.ac.in 5 1
```


## Output
The program estimates the route taken to reach the target site and calculates the latency and bandwidth of each intermediate link. The output displays the route and the corresponding latency and bandwidth for each hop.It also prints out the header fields of every ICMP packet sent and received in a nice format.

