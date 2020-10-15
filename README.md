# blockchain
A mainly educational blockchain project in C

# Main Dependencies
- Ulfius HTTP Framework (https://github.com/babelouest/ulfius)
- Jansson library for encoding, decoding and manipulating JSON data (https://github.com/akheron/jansson)

# Specific Dependencies
- **GUI Wallet**: X11, OpenGL and GLUT libraries installed
- **OpenMP miner**: OpenMP Library installed
- **Cuda miner**: Cuda-Capable Nvidia Graphics Card and Nvidia Toolkit installed

# Features
- DNS Server for initial node connection and node retrieval (make dns_server)
- Basic Client with all blockchain capabilites (make client)
- Console Wallet that extends the client with wallet creation/recovery using Seed Phrase from wordlist (https://github.com/bitcoin/bips/blob/master/bip-0039/english.txt),   transaction and balance calculation features (make wallet)
- GUI Wallet using Nuklear UI Library (https://github.com/Immediate-Mode-UI/Nuklear) (make wallet_gui)
- Basic Single-Core/Threaded miner (make miner)
- Multi-Threaded OpenMP miner (make omp_miner)
- CUDA-accelerated miner (make cuda_miner)
