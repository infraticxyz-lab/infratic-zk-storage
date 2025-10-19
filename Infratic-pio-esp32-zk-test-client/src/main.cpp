
#include <Arduino.h>
#include <WiFi.h>
#include "Infratic-lib.h"

const char *WIFI_SSID = "YOUR_WIFI_SSID";
const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const String SOLANA_RPC_URL = "https://api.devnet.solana.com";

const String PRIVATE_KEY_BASE58 = "YOUR_PRIVATE_KEY_BASE58";
const String PUBLIC_KEY_BASE58 = "YOUR_PUBLIC_KEY_BASE58";

Infratic solana(SOLANA_RPC_URL);

const String ANCHOR_PROGRAM_ID = "3dQsx7p1Fcxzr69vaewmYdb56FWtmjGzTXkxpLxo8Qfu";
const String ZK_SEED_NAME = "zk_test_" + String(random(100000, 999999));

void debug_seed_pda()
{
    // Test Authority
    std::vector<uint8_t> authBytes = base58ToPubkey(PUBLIC_KEY_BASE58);
    Serial.println("Authority Pubkey Decode:");
    Serial.printf("  Size: %d bytes (should be 32)\n", authBytes.size());
    Serial.print("  Hex: ");
    for (int i = 0; i < authBytes.size() && i < 8; i++)
    {
        Serial.printf("%02x", authBytes[i]);
    }
    Serial.println("...");

    // Test Seed Name
    Serial.println("\nSeed Name Bytes:");
    Serial.print("  Text: " + ZK_SEED_NAME + " (");
    Serial.printf("%d bytes)\n", ZK_SEED_NAME.length());
    Serial.print("  Hex: ");
    for (char c : ZK_SEED_NAME)
    {
        Serial.printf("%02x", (uint8_t)c);
    }
    Serial.println();

    // Derive PDA
    std::vector<std::vector<uint8_t>> seeds = {
        std::vector<uint8_t>(ZK_SEED_NAME.begin(), ZK_SEED_NAME.end()),
        authBytes};

    String pda;
    uint8_t bump;
    if (solana.derivePDA(seeds, ANCHOR_PROGRAM_ID, pda, bump))
    {
        Serial.println("\nPDA Derivation Success:");
        Serial.println("  PDA: " + pda);
        Serial.printf("  Bump: %d\n", bump);
    }
    else
    {
        Serial.println("\nPDA Derivation Failed");
    }

    Serial.println("\n════════════════════════════════════════════\n");
}

void example_anchorInitializeAndStore()
{

    // STEP 1: Initialize Storage
    Serial.println("Step 1: Initialize Storage Account");
    Serial.println("─────────────────────────────────────────────");

    String pda, txSig1;
    if (solana.initializeZKStorage(
            PRIVATE_KEY_BASE58,
            PUBLIC_KEY_BASE58,
            ANCHOR_PROGRAM_ID,
            ZK_SEED_NAME,
            pda,
            txSig1))
    {
        Serial.println("✓ Storage initialized!");
        Serial.println("  PDA: " + pda);
        Serial.println("  Tx: " + txSig1);
    }
    else
    {
        Serial.println("⚠ Initialize failed or already exists");
        Serial.println("  (This is OK - will retry with existing account)");
    }

    delay(3000);

    // STEP 2: Create commitment
    Serial.println("\nStep 2: Create ZK Commitment");
    Serial.println("─────────────────────────────────────────────");

    float temperature = 25.7;
    String data = "temp:" + String(temperature);
    String secret = "sensor_secret";

    String commitment, nonce;
    uint64_t timestamp;
    if (solana.createDataCommitment(data, secret, commitment, nonce, timestamp))
    {
        Serial.println("✓ Commitment created");
        Serial.println("  Data: " + data);
        Serial.println("  Commitment: " + commitment.substring(0, 32) + "...");
        Serial.println("  Nonce: " + nonce.substring(0, 16) + "...");
        Serial.printf("  Timestamp: %llu\n", timestamp);

        delay(2000);

        // STEP 3: Store commitment
        Serial.println("\nStep 3: Store Commitment in Anchor");
        Serial.println("─────────────────────────────────────────────");

        String txSig2;
        String metadata = "sensor:ESP32_001,nonce:" + nonce.substring(0, 16) + ",ts:" + String(timestamp);

        if (solana.storeCommitmentInAnchor(
                PRIVATE_KEY_BASE58,
                PUBLIC_KEY_BASE58,
                ANCHOR_PROGRAM_ID,
                ZK_SEED_NAME,
                commitment,
                metadata,
                txSig2))
        {
            Serial.println("✓ Commitment stored in Anchor!");
            Serial.println("  Tx: " + txSig2);
            Serial.println("  Data is now PERMANENT on Solana");
        }
        else
        {
            Serial.println("✗ Store failed");
        }
    }
    else
    {
        Serial.println("✗ Commitment creation failed");
    }

    Serial.println("\n════════════════════════════════════════════\n");
}

void example_anchorUpdateCommitment()
{
    Serial.println("Creating new commitment...");

    float newTemp = 26.5;
    String newData = "temp:" + String(newTemp);
    String secret = "sensor_secret";

    String newCommitment, newNonce;
    uint64_t newTimestamp;
    if (solana.createDataCommitment(newData, secret, newCommitment, newNonce, newTimestamp))
    {
        Serial.println("✓ New commitment created: " + newCommitment.substring(0, 32) + "...");

        String txSig;
        String metadata = "updated,ts:" + String(newTimestamp);

        if (solana.updateCommitmentInAnchor(
                PRIVATE_KEY_BASE58,
                PUBLIC_KEY_BASE58,
                ANCHOR_PROGRAM_ID,
                ZK_SEED_NAME,
                newCommitment,
                metadata,
                txSig))
        {
            Serial.println("✓ Commitment updated!");
            Serial.println("  Tx: " + txSig);
        }
        else
        {
            Serial.println("✗ Update failed");
        }
    }

    Serial.println("\n════════════════════════════════════════════\n");
}

void example_anchorStoreMerkleRoot()
{
    Serial.println("Collecting sensor data...");

    std::vector<String> sensorData = {
        "temp:25.7",
        "humidity:65.2",
        "pressure:1013.25",
        "co2:420",
        "light:750"};

    Serial.printf("  Collected %d data points\n", sensorData.size());

    String merkleRoot;
    if (solana.buildMerkleTree(sensorData, merkleRoot))
    {
        Serial.println("✓ Merkle root: " + merkleRoot.substring(0, 32) + "...");

        String txSig;
        String metadata = "sensor_batch_001";

        if (solana.storeMerkleRootInAnchor(
                PRIVATE_KEY_BASE58,
                PUBLIC_KEY_BASE58,
                ANCHOR_PROGRAM_ID,
                "merkle_data",
                merkleRoot,
                sensorData.size(),
                metadata,
                txSig))
        {
            Serial.println("✓ Merkle root stored!");
            Serial.println("  Tx: " + txSig);
            Serial.printf("  Compressed %d points\n", sensorData.size());
        }
        else
        {
            Serial.println("✗ Store failed");
        }
    }

    Serial.println("\n════════════════════════════════════════════\n");
}

void example_anchorStoreBatch()
{
    std::vector<String> dataList = {
        "reading1:value1",
        "reading2:value2",
        "reading3:value3",
        "reading4:value4",
        "reading5:value5"};

    String secret = "batch_secret";
    std::vector<ZKCommitment> commitments;

    if (solana.createBatchCommitments(dataList, secret, commitments))
    {
        Serial.printf("✓ Created %d commitments\n", commitments.size());

        std::vector<String> commitmentHashes;
        for (const auto &c : commitments)
        {
            commitmentHashes.push_back(c.commitment);
        }

        String batchRoot;
        if (solana.buildMerkleTree(commitmentHashes, batchRoot))
        {
            Serial.println("✓ Batch root: " + batchRoot.substring(0, 32) + "...");

            String txSig;
            String batchId = "batch_" + String(millis());
            String metadata = "device:ESP32_001";

            if (solana.storeBatchInAnchor(
                    PRIVATE_KEY_BASE58,
                    PUBLIC_KEY_BASE58,
                    ANCHOR_PROGRAM_ID,
                    batchId,
                    batchRoot,
                    commitments.size(),
                    metadata,
                    txSig))
            {
                Serial.println("✓ Batch stored!");
                Serial.println("  ID: " + batchId);
                Serial.println("  Tx: " + txSig);
                Serial.printf("  Cost: 1 tx for %d commitments\n", commitments.size());
            }
            else
            {
                Serial.println("✗ Batch store failed");
            }
        }
    }

    Serial.println("\n════════════════════════════════════════════\n");
}

void test_pda_consistency()
{

    String pda1, pda2;
    uint8_t bump1, bump2;

    std::vector<uint8_t> authBytes = base58ToPubkey(PUBLIC_KEY_BASE58);
    std::vector<std::vector<uint8_t>> seeds = {
        std::vector<uint8_t>(ZK_SEED_NAME.begin(), ZK_SEED_NAME.end()),
        authBytes};

    Serial.println("Deriving PDA first time...");
    if (solana.derivePDA(seeds, ANCHOR_PROGRAM_ID, pda1, bump1))
    {
        delay(100);
        Serial.println("Deriving PDA second time...");
        if (solana.derivePDA(seeds, ANCHOR_PROGRAM_ID, pda2, bump2))
        {
            Serial.println("PDA 1: " + pda1);
            Serial.println("PDA 2: " + pda2);
            Serial.println("Match: " + String(pda1 == pda2 ? "YES" : "NO"));
            Serial.printf("Bumps: %d vs %d\n", bump1, bump2);

            if (pda1 == pda2 && bump1 == bump2)
            {
                Serial.println("\n✓ PDA derivation is consistent!");
            }
            else
            {
                Serial.println("\n✗ PDA derivation is INCONSISTENT!");
            }
        }
    }

    Serial.println("\n════════════════════════════════════════════\n");
}

void test_commitment_conversion()
{

    String data = "temp:25.7";
    String secret = "test_secret";
    String commitment, nonce;
    uint64_t timestamp;

    if (solana.createDataCommitment(data, secret, commitment, nonce, timestamp))
    {
        Serial.println("Commitment (hex):");
        Serial.println(commitment);
        Serial.printf("\nLength: %d (should be 64 for 32 bytes)\n", commitment.length());

        std::vector<uint8_t> bytes;
        for (size_t i = 0; i < commitment.length() && i < 64; i += 2)
        {
            String byteStr = commitment.substring(i, i + 2);
            bytes.push_back(strtol(byteStr.c_str(), nullptr, 16));
        }

        Serial.printf("\nConverted bytes: %d (should be 32)\n", bytes.size());
        Serial.print("Hex check: ");
        for (int i = 0; i < bytes.size() && i < 16; i++)
        {
            Serial.printf("%02x", bytes[i]);
        }
        Serial.println("...");

        if (bytes.size() == 32)
        {
            Serial.println("\n✓ Commitment conversion is correct!");
        }
        else
        {
            Serial.println("\n✗ Commitment conversion failed!");
        }
    }

    Serial.println("\n════════════════════════════════════════════\n");
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // WiFi connection
    Serial.println("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int wifiAttempts = 0;
    while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20)
    {
        delay(500);
        Serial.print(".");
        wifiAttempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\n✓ WiFi Connected\n");
    }
    else
    {
        Serial.println("\n✗ WiFi Failed\n");
        return;
    }

    // Print configuration
    Serial.println("Configuration:");
    Serial.println("  Program ID: " + ANCHOR_PROGRAM_ID);
    Serial.println("  Seed Name: " + ZK_SEED_NAME);
    Serial.println("  Authority: " + PUBLIC_KEY_BASE58.substring(0, 16) + "...");
    Serial.println("  RPC: " + SOLANA_RPC_URL);
    Serial.println();

    delay(2000);

    // Run debug checks
    debug_seed_pda();
    delay(2000);

    test_pda_consistency();
    delay(2000);

    test_commitment_conversion();
    delay(2000);

    // Run main example
    example_anchorInitializeAndStore();
}

void loop()
{
    delay(10000);
}