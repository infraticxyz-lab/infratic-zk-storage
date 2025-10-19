# Infratic ZK Storage

Zero-Knowledge Proofs on Solana Blockchain

An open-source project that securely stores secret data from ESP32 IoT devices to the Solana blockchain using Zero-Knowledge Proof (ZK) technology.

**Version:** 1.0.0 | **License:** MIT | **Repository:** [infraticxyz-lab/infratic-zk-storage](https://github.com/infraticxyz-lab/infratic-zk-storage)

---

## Overview

Enable privacy-preserving data storage on Solana using ZK proofs

**Target Users:**
- IoT developers
- Blockchain engineers
- Privacy-focused applications

**Key Benefits:**
- Data confidentiality maintained
- Blockchain-verifiable proofs
- Cost-efficient batch operations
- Open-source and extensible

---

## Features

### Data Commitment
Cryptographic proof of data existence without revelation
- **Use Case:** Sensor data privacy
- **Example:** Temperature readings hidden, existence proven

### Merkle Tree Proof
Privacy-preserving verification for multi-sensor networks
- **Use Case:** Network sensor validation
- **Example:** Prove single sensor reading in dataset without revealing others

### Range Proof
Validate value within range without disclosing exact value
- **Use Case:** Compliance verification
- **Example:** Prove temperature in range [20-30]C without revealing exact value

### Timestamped Proof
Blockchain-anchored temporal proofs
- **Use Case:** Immutable audit trails
- **Example:** Prove asset location at specific blockchain timestamp

### Batch Commitments
Cost-efficient batch publishing with Merkle trees
- **Use Case:** Bulk data operations
- **Example:** Store 5 readings in 1 transaction (80% cost savings)

---

## Technology Stack

### Hardware
- **ESP32:** 240MHz dual-core processor, WiFi enabled
- **WiFi Connectivity:** IEEE 802.11 b/g/n

### Blockchain
- **Solana:** Devnet (https://api.devnet.solana.com)
- **Anchor Framework:** v0.28.0+
- **ZK Storage Program:** `3dQsx7p1Fcxzr69vaewmYdb56FWtmjGzTXkxpLxo8Qfu` (Rust)

### Cryptography
- SHA256 - Data commitment hashing
- Ed25519 - Transaction signing
- Merkle Trees - Multi-data verification
- Base58 Encoding - Address encoding

### Libraries
- **Infratic-lib.h** - Solana integration library (C++ Header)
- **Infratic-lib.cpp** - Core functionality implementation (C++)
- **ArduinoJson** v6.21.5 - JSON parsing
---

## Project Structure

```
infratic-zk-storage/
├── Infratic-pio-esp32-zk-test-client/
│   ├── src/
│   │   └── main.cpp
│   ├── include/
│   ├── lib/
│   ├── test/
│   └── platformio.ini
├── Solana-contract/
│   ├── app/
│   ├── migrations/
│   ├── programs/
│   │   └── zk-storage/
│   │       └── src/
│   │           └── lib.rs
│   ├── tests/
│   └── Cargo.toml
└── README.md
```

---

## Installation

### Requirements
- **PlatformIO or Arduino IDE** - ESP32 development ([https://platformio.org](https://platformio.org))
- **Rust** - Anchor program compilation ([https://rustup.rs](https://rustup.rs))
- **Anchor CLI** - Solana program deployment
  ```bash
  cargo install --git https://github.com/coral-xyz/anchor --tag v0.28.0 anchor-cli --locked
  ```
- **Solana CLI** - Blockchain interaction
  ```bash
  sh -c "$(curl -sSfL https://release.solana.com/v1.16.15/install)"
  ```
- **ESP32 Development Board** - ESP32-WROOM-32 or compatible

### Setup Steps

#### Step 1: Deploy Rust Program to Devnet

```bash
cd programs/zk-storage
anchor build
anchor deploy --provider.cluster devnet
```

**Notes:**
- Ensure Solana CLI is configured for devnet
- Account must have SOL for deployment fees
- Program ID will be displayed after deployment

#### Step 2: Configure ESP32 Code

Edit `src/main.ino` and update:

```cpp
const char *WIFI_SSID = "YOUR_SSID";
const char *WIFI_PASSWORD = "YOUR_PASSWORD";
const String PRIVATE_KEY_BASE58 = "epnp45SwerY234...56edrctvb";
const String PUBLIC_KEY_BASE58 = "FgtXBob7...BHRUKZzQ";
const String ANCHOR_PROGRAM_ID = "3dQsx7p1...GzTXkxpL";
```

#### Step 3: Upload and Test

```bash
platformio run --target upload
platformio device monitor
```

**Expected Output:**
- WiFi connected
- PDA derivation successful
- Commitment created
- Transaction confirmed

---

## Usage

### Basic Example

```cpp
// 1. Create ZK Commitment
String commitment, nonce;
uint64_t timestamp;
solana.createDataCommitment(
  "temp:25.7C",
  "sensor_secret",
  commitment,
  nonce,
  timestamp
);

// 2. Store on Blockchain
String txSig;
solana.storeCommitmentOnChain(
  privateKey,
  publicKey,
  commitment,
  "sensor:ESP32_001",
  txSig
);

// 3. Verify Commitment
bool verified = solana.verifyDataCommitment(
  "temp:25.7C",
  "sensor_secret",
  nonce,
  timestamp,
  commitment
);
```
---

## ZK Proof Types

| Type | Privacy | Efficiency | Use Cases | Cost Benefit |
|------|---------|-----------|-----------|-------------|
| **Data Commitment** | 10/10 | 9/10 | Sensor privacy, Personal info, Logging | 1x baseline |
| **Merkle Tree** | 9/10 | 9/10 | Multi-sensor, Large datasets | Logarithmic |
| **Range Proof** | 8/10 | 8/10 | Compliance, QA, Reporting | Value hidden |
| **Timestamped** | 7/10 | 9/10 | Asset tracking, Audits, Logs | Blockchain-anchored |
| **Batch** | 9/10 | 10/10 | Bulk ops, Cost optimization | 80% savings |

---

## Cost Analysis

| Operation | Cost | Savings | Use Case |
|-----------|------|---------|----------|
| Single Transaction | 1x | — | Single sensor reading |
| Batch (5 readings) | 0.2x | 80% | Multiple sensors |
| Merkle Tree (1000 readings) | 0.01x | 99% | Large networks |

---

## API Reference

### Core Functions

**createDataCommitment**
```cpp
bool createDataCommitment(const String &data, const String &secret,
                          String &outCommitment, String &outNonce,
                          uint64_t &outTimestamp)
```
Create ZK commitment to data

**buildMerkleTree**
```cpp
bool buildMerkleTree(const std::vector<String> &dataList, String &outRoot)
```
Construct Merkle tree from data list

**createMerkleProof**
```cpp
bool createMerkleProof(const std::vector<String> &dataList, size_t dataIndex,
                       MerkleProof &outProof)
```
Generate proof for specific data point

**createRangeProof**
```cpp
bool createRangeProof(int64_t value, int64_t minValue, int64_t maxValue,
                      const String &secret, RangeProof &outProof)
```
Prove value in range without revealing

### Verification Functions

- **verifyDataCommitment** - Verify data against commitment
- **verifyMerkleProof** - Verify Merkle tree membership
- **verifyRangeProof** - Verify range proof validity
- **verifyTimestampedProof** - Verify blockchain-anchored timestamp

---

## Troubleshooting

### Error: 0x7d6 - Initialize Storage Failed

**Cause:** Account already exists or constraint violation

**Solutions:**
- Use different seed name
- Check account balance
- Verify authority matches
- Ensure account not initialized

### Error: 0xbc4 - Store Commitment Failed

**Cause:** PDA seed mismatch or authorization error

**Solutions:**
- Verify PDA derivation
- Check authority pubkey
- Ensure account initialized first
- Validate seed consistency

### WiFi Connection Issue

**Solutions:**
- Check SSID and password
- Verify WiFi 2.4GHz support (not 5GHz)
- Check signal strength
- Restart ESP32

### RPC Connection Issue

**Solutions:**
- Ping devnet: `ping api.devnet.solana.com`
- Check internet connectivity
- Try different RPC endpoint
- Verify network not blocked

---

## Security

### Best Practices

- Never commit private keys
- Use environment variables for secrets
- Verify Anchor program before deployment
- Test on devnet before mainnet
- Validate all user inputs

### Key Management

- Store keys securely
- Use hardware wallets when possible
- Rotate keys periodically
- Use separate keys per environment

---

## Resources

### Documentation

- [Infratic Library](https://github.com/infraticxyz-lab/Infratic-lib) - Solana C++ library
- [Anchor Documentation](https://docs.rs/anchor-lang/latest/anchor_lang/) - Solana Anchor framework
- [Solana Documentation](https://docs.solana.com) - Solana blockchain docs
- [Zero-Knowledge Proofs](https://en.wikipedia.org/wiki/Zero-knowledge_proof) - ZK theory and applications

### Tools

- [Solana Explorer](https://explorer.solana.com?cluster=devnet) - Blockchain explorer (devnet)
- [Anchor IDE](https://www.anchor-lang.com) - Solana program development

---

## Contact

- **Issues:** GitHub Issues - Report bugs and request features
- **Discussions:** GitHub Discussions - Ask questions and share ideas
- **Twitter:** [@infraticxyz](https://twitter.com/infraticxyz)

---

## License

MIT

---

**Built with cryptographic integrity and privacy-first design**

Author: asimbugra | Year: 2025

**Repository:** [https://github.com/infraticxyz-lab/infratic-zk-storage](https://github.com/infraticxyz-lab/infratic-zk-storage)

**Built with:** Infratic Library • Anchor Framework • Solana Blockchain