// programs/zk-storage/src/lib.rs

use anchor_lang::prelude::*;
use sha2::{Sha256, Digest};

declare_id!("3dQsx7p1Fcxzr69vaewmYdb56FWtmjGzTXkxpLxo8Qfu");

#[program]
pub mod zk_storage {
    use super::*;

    /// Initialize a new ZK storage account
    pub fn initialize_storage(
        ctx: Context<InitializeStorage>,
        seed_name: String,
    ) -> Result<()> {
        require!(
            seed_name.len() <= 50,
            ErrorCode::SeedNameTooLong
        );

        let storage = &mut ctx.accounts.storage;
        storage.authority = ctx.accounts.authority.key();
        storage.seed_name = seed_name.clone();
        storage.commitment = [0u8; 32];
        storage.metadata = String::new();
        storage.timestamp = 0;
        storage.bump = ctx.bumps.storage;
        storage.is_initialized = true;
        
        msg!("✅ ZK Storage initialized with seed: {}", seed_name);
        msg!("   Authority: {}", storage.authority);
        msg!("   Bump: {}", storage.bump);
        Ok(())
    }

    /// Store a ZK commitment
    pub fn store_commitment(
        ctx: Context<StoreCommitment>,
        commitment: [u8; 32],
        metadata: String,
        timestamp: u64,
    ) -> Result<()> {
        require!(
            metadata.len() <= 500,
            ErrorCode::MetadataTooLong
        );

        require!(
            ctx.accounts.storage.is_initialized,
            ErrorCode::StorageNotInitialized
        );

        let storage = &mut ctx.accounts.storage;
        storage.commitment = commitment;
        storage.metadata = metadata.clone();
        storage.timestamp = timestamp;
        
        msg!("✅ Commitment stored");
        msg!("   Timestamp: {}", timestamp);
        Ok(())
    }

    /// Update existing commitment
    pub fn update_commitment(
        ctx: Context<UpdateCommitment>,
        new_commitment: [u8; 32],
        metadata: String,
        timestamp: u64,
    ) -> Result<()> {
        require!(
            metadata.len() <= 500,
            ErrorCode::MetadataTooLong
        );

        require!(
            ctx.accounts.storage.is_initialized,
            ErrorCode::StorageNotInitialized
        );

        let storage = &mut ctx.accounts.storage;
        storage.commitment = new_commitment;
        storage.metadata = metadata.clone();
        storage.timestamp = timestamp;
        
        msg!("✅ Commitment updated");
        Ok(())
    }

    /// Verify a commitment on-chain
    pub fn verify_commitment(
        ctx: Context<VerifyCommitment>,
        data: String,
        secret: String,
        nonce: String,
    ) -> Result<()> {
        let storage = &ctx.accounts.storage;
        
        require!(
            storage.is_initialized,
            ErrorCode::StorageNotInitialized
        );

        // Reconstruct commitment
        let input = format!("{}{}{}{}", data, secret, nonce, storage.timestamp);
        let hash = Sha256::digest(input.as_bytes());
        
        require!(
            hash.as_slice() == storage.commitment,
            ErrorCode::InvalidCommitment
        );
        
        msg!("✅ Commitment verified on-chain!");
        Ok(())
    }

    /// Store Merkle root
    pub fn store_merkle_root(
        ctx: Context<StoreMerkleRoot>,
        merkle_root: [u8; 32],
        leaf_count: u32,
        metadata: String,
    ) -> Result<()> {
        require!(
            metadata.len() <= 500,
            ErrorCode::MetadataTooLong
        );

        require!(
            ctx.accounts.storage.is_initialized,
            ErrorCode::StorageNotInitialized
        );

        let storage = &mut ctx.accounts.storage;
        storage.commitment = merkle_root;
        storage.metadata = format!("merkle_leaves:{},{}", leaf_count, metadata);
        storage.timestamp = Clock::get()?.unix_timestamp as u64;
        
        msg!("✅ Merkle root stored with {} leaves", leaf_count);
        Ok(())
    }

    /// Store batch commitments
    pub fn store_batch(
        ctx: Context<StoreBatch>,
        batch_root: [u8; 32],
        batch_size: u32,
        metadata: String,
    ) -> Result<()> {
        require!(
            metadata.len() <= 200,
            ErrorCode::MetadataTooLong
        );

        let batch_storage = &mut ctx.accounts.batch_storage;
        batch_storage.authority = ctx.accounts.authority.key();
        batch_storage.batch_root = batch_root;
        batch_storage.batch_size = batch_size;
        batch_storage.metadata = metadata.clone();
        batch_storage.timestamp = Clock::get()?.unix_timestamp as u64;
        batch_storage.bump = ctx.bumps.batch_storage;
        
        msg!("✅ Batch stored: {} commitments", batch_size);
        Ok(())
    }
}

// ============================================================================
// ACCOUNTS
// ============================================================================

#[derive(Accounts)]
#[instruction(seed_name: String)]
pub struct InitializeStorage<'info> {
    #[account(
        init,
        payer = authority,
        space = 8 + 32 + 4 + 50 + 32 + 4 + 500 + 8 + 1 + 1,
        seeds = [seed_name.as_bytes(), authority.key().as_ref()],
        bump
    )]
    pub storage: Account<'info, ZKStorage>,
    
    #[account(mut)]
    pub authority: Signer<'info>,
    
    pub system_program: Program<'info, System>,
}

#[derive(Accounts)]
pub struct StoreCommitment<'info> {
    #[account(
        mut,
        seeds = [storage.seed_name.as_bytes(), authority.key().as_ref()],
        bump = storage.bump,
        constraint = storage.authority == authority.key() @ ErrorCode::Unauthorized
    )]
    pub storage: Account<'info, ZKStorage>,
    
    pub authority: Signer<'info>,
}

#[derive(Accounts)]
pub struct UpdateCommitment<'info> {
    #[account(
        mut,
        seeds = [storage.seed_name.as_bytes(), authority.key().as_ref()],
        bump = storage.bump,
        constraint = storage.authority == authority.key() @ ErrorCode::Unauthorized
    )]
    pub storage: Account<'info, ZKStorage>,
    
    pub authority: Signer<'info>,
}

#[derive(Accounts)]
pub struct VerifyCommitment<'info> {
    #[account(
        seeds = [storage.seed_name.as_bytes(), storage.authority.as_ref()],
        bump = storage.bump
    )]
    pub storage: Account<'info, ZKStorage>,
    
    pub authority: Signer<'info>,
}

#[derive(Accounts)]
pub struct StoreMerkleRoot<'info> {
    #[account(
        mut,
        seeds = [storage.seed_name.as_bytes(), authority.key().as_ref()],
        bump = storage.bump,
        constraint = storage.authority == authority.key() @ ErrorCode::Unauthorized
    )]
    pub storage: Account<'info, ZKStorage>,
    
    pub authority: Signer<'info>,
}

#[derive(Accounts)]
#[instruction(batch_root: [u8; 32], batch_size: u32, metadata: String)]
pub struct StoreBatch<'info> {
    #[account(
        init,
        payer = authority,
        space = 8 + 32 + 32 + 4 + 4 + 200 + 8 + 1,
        seeds = [b"batch", batch_root.as_ref(), authority.key().as_ref()],
        bump
    )]
    pub batch_storage: Account<'info, BatchStorage>,
    
    #[account(mut)]
    pub authority: Signer<'info>,
    
    pub system_program: Program<'info, System>,
}

// ============================================================================
// DATA STRUCTURES
// ============================================================================

#[account]
pub struct ZKStorage {
    pub authority: Pubkey,              // 32
    pub seed_name: String,              // 4 + 50
    pub commitment: [u8; 32],           // 32
    pub metadata: String,               // 4 + 500
    pub timestamp: u64,                 // 8
    pub bump: u8,                       // 1
    pub is_initialized: bool,           // 1
}

#[account]
pub struct BatchStorage {
    pub authority: Pubkey,              // 32
    pub batch_root: [u8; 32],          // 32
    pub batch_size: u32,               // 4
    pub metadata: String,               // 4 + 200
    pub timestamp: u64,                // 8
    pub bump: u8,                      // 1
}

// ============================================================================
// ERRORS
// ============================================================================

#[error_code]
pub enum ErrorCode {
    #[msg("Invalid commitment - verification failed")]
    InvalidCommitment,
    
    #[msg("Metadata exceeds maximum length")]
    MetadataTooLong,
    
    #[msg("Unauthorized access - not the account authority")]
    Unauthorized,

    #[msg("Storage account not initialized")]
    StorageNotInitialized,

    #[msg("Seed name too long (max 50 characters)")]
    SeedNameTooLong,
}