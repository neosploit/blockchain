# blockchain
A mainly educational blockchain project in C

# Main Dependencies
- Ulfius HTTP Framework (https://github.com/babelouest/ulfius)
- Jansson library for encoding, decoding and manipulating JSON data (https://github.com/akheron/jansson)
- Ubuntu or WSL Environment

# System-Specific Dependencies
- **GUI Wallet**: X11, OpenGL and GLUT libraries installed
- **OpenMP miner**: OpenMP Library installed
- **Cuda miner**: Cuda-Capable Nvidia Graphics Card and Nvidia Toolkit installed

# Systems
- DNS Server for initial node connection and node retrieval (make dns_server)
- Basic Client with all blockchain capabilites (make client)
- Console Wallet that extends the client with wallet creation/recovery using Seed Phrase from wordlist (https://github.com/bitcoin/bips/blob/master/bip-0039/english.txt),   transaction and balance calculation features (make wallet)
- GUI Wallet using Nuklear UI Library (https://github.com/Immediate-Mode-UI/Nuklear) (make wallet_gui)
- Basic Single-Core/Threaded miner (make miner)
- Multi-Threaded OpenMP miner (make omp_miner)
- CUDA-accelerated miner (make cuda_miner)
- Web-Site Explorer (explorer folder)

# Screenshots

## DNS Server
![alt text](https://i.ibb.co/GQYzpcZ/dns-server.png)

## Client
![alt text](https://i.ibb.co/86mprwJ/client.png)

## Miner
![alt text](https://i.ibb.co/MRnB8Bk/miner.png)

## OpenMP Miner
![alt text](https://i.ibb.co/4JppYrR/omp-miner.png)

## Cuda Miner
![alt text](https://i.ibb.co/BstZNqy/cuda-miner.png)

## Console Wallet
![alt text](https://i.ibb.co/ZzX5JXp/wallet.gif)

## GUI Wallet
![alt text](https://i.ibb.co/0jH8NKF/wallet-gui.gif)
