# blockchain
A mainly educational blockchain project in C

# Main Dependencies
- Ulfius HTTP Framework (https://github.com/babelouest/ulfius)
- Jansson library for encoding, decoding and manipulating JSON data (https://github.com/akheron/jansson)
- Rhonabwy JWK, JWKS, JWS, JWE and JWT library (https://github.com/babelouest/rhonabwy)
- Ubuntu or WSL Environment (version 20.04 LTS and higher)

# System-Specific Dependencies
- **GUI Wallet**: X11, OpenGL and GLUT libraries installed
- **OpenMP miner**: OpenMP Library installed
- **Cuda miner**: Cuda-Capable Nvidia Graphics Card and Cuda Toolkit installed
- **Explorer Website**: Apache HTTP Server to run Explorer from (XAMPP, LAMP etc.)

# Systems
- DNS Server for initial node connection and node retrieval (make dns_server)
- Basic Client with all blockchain capabilites (make client)
- Console Wallet that extends the client with wallet creation/recovery using Seed Phrase from wordlist (https://github.com/bitcoin/bips/blob/master/bip-0039/english.txt),   transaction and balance calculation features (make wallet)
- GUI Wallet using Nuklear UI Library (https://github.com/Immediate-Mode-UI/Nuklear) (make wallet_gui)
- Basic Single-Core/Threaded miner (make miner)
- Multi-Threaded OpenMP miner (make omp_miner)
- CUDA-accelerated miner (make cuda_miner)
- Console-based Blockchain Explorer (make explorer)
- Website Blockchain Explorer (explorer folder)

# Screenshots

## DNS Server
![dns server](https://i.ibb.co/GQYzpcZ/dns-server.png)

## Client
![client](https://i.ibb.co/86mprwJ/client.png)

## Miner
![miner](https://i.ibb.co/MRnB8Bk/miner.png)

## OpenMP Miner
![openmp miner](https://i.ibb.co/4JppYrR/omp-miner.png)

## Cuda Miner
![cuda miner](https://i.ibb.co/BstZNqy/cuda-miner.png)

## Console Wallet
![console wallet](https://i.ibb.co/ZzX5JXp/wallet.gif)

## GUI Wallet
![gui wallet](https://i.ibb.co/0jH8NKF/wallet-gui.gif)

## Console Explorer
![console explorer](https://i.ibb.co/59VRgVD/explorer-console.gif)

## Website Explorer
![website explorer](https://i.ibb.co/SmQ2yz9/explorer-web.gif)

