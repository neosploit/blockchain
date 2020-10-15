# blockchain
A mainly educational blockchain project in C

# Main Dependencies
- Ulfius HTTP Framework (https://github.com/babelouest/ulfius)
- Jansson library for encoding, decoding and manipulating JSON data (https://github.com/akheron/jansson)

# Features
- DNS Server for intial node connection (make dns_server)
- Basic Client with all blockchain capabilites (make client)
- Console Wallet that extends the client with wallet creation/recovery and transaction features (make wallet)
- GUI Wallet using Nuklear UI Library (https://github.com/Immediate-Mode-UI/Nuklear) (make wallet_gui)
- Basic Single-Core/Threaded miner (make miner)
- Multi-Threaded OpenMP miner (make omp_miner)
- CUDA-accelerated miner (make cuda_miner)
