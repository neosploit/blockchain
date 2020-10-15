# blockchain
A mainly educational blockchain project in C

# Main Dependencies
- Ulfius HTTP Framework (https://github.com/babelouest/ulfius)
- Jansson library for encoding, decoding and manipulating JSON data (https://github.com/akheron/jansson)
- Ubuntu or WSL Environment

# Feature-Specific Dependencies
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

# Screenshots
![alt text](https://i.ibb.co/B2WnmwC/118805540-642610140021463-2051768474811686891-n.png)
![alt text](https://i.ibb.co/0FKMd0d/120554367-3407797569336584-6852632357135082824-n.png)
![alt text](https://i.ibb.co/S5qhBpc/120575897-348862949505949-1875888404735949257-n.png)
![alt text](https://i.ibb.co/R7CpTHF/120765160-669273930454228-730927498842608372-n.png)
![alt text](https://i.ibb.co/DV0YQtD/120770607-3205423722917670-7611413954801783402-n.png)
![alt text](https://i.ibb.co/dcmQPVZ/120772258-3431084170307717-9044705286711693232-n.png)
