# Wireless Network Simulation with ns-3

This repository contains an ns-3 simulation script for a wireless network scenario with multiple mobile nodes and a monitoring node.
It has added support for caching mechanisms.

The simulation demonstrates the following features:

- Random Walk Mobility Model for nodes
- Wi-Fi 802.11n network with configurable channel and physical layer settings
- Point-to-point links between nodes and a monitoring node
- UDP communication between nodes (client-server model)
- Packet capture and tracing (PCAP and ASCII)
- Network animation using NetAnim

The simulation can be used to study wireless network behavior, mobility models, and network monitoring techniques in ns-3.

## Usage

1. Install ns-3 on your system (I used ns-3.36)
2. Clone this repository: `git clone https://github.com/Hemantha-krishna/Network-Simulation-with-Cache`
3. Navigate to the repository directory
4. Build the simulation script: `./waf --run scratch/wireless-simulation` or `./ns3 run scratch/wireless-simulation`
5. We can find the PCAP files in the scratch folder which can be analyzed using tools like Wireshark
6. After the simulation is completed, you can view the animation using NetAnim

Feel free to modify the simulation parameters (e.g., number of nodes, mobility area, simulation time) in the `main` function to explore different scenarios.
