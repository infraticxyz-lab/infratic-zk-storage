prerequisites:
  title: "Prerequisites"
  intro: "Before you begin, ensure you have the following tools installed on your system:"
  tools:
    - name: "Rust"
      url: "https://www.rust-lang.org/tools/install"
    - name: "Solana CLI"
      url: "https://docs.solana.com/cli/install"
    - name: "Anchor Framework (avm & anchor-cli)"
      url: "https://www.anchor-lang.com/docs/installation"
    - name: "Node.js (LTS)"
      url: "https://nodejs.org/en/"
    - name: "Yarn (or npm)"
      url: "https://yarnpkg.com/getting-started/install"

installation:
  title: "Installation"
  steps:
    - step: 1
      description: "Clone the repository:"
      code: |
        git clone https://github.com/infraticxyz-lab/infratic-zk-storage.git
        cd Solana-contract
    - step: 2
      description: "Install the Node.js dependencies:"
      code: |
        yarn install
        # or
        # npm install

developmentWorkflow:
  title: "Development Workflow"
  sections:
    - title: "1. Build the Program"
      description: "Compile the on-chain Rust program (located in `programs/`) using the Anchor CLI. This will generate the compiled `.so` file and the program's IDL (Interface Definition Language) in the `target/` directory."
      code: "anchor build"
      
    - title: "2. Run Tests"
      description: "Run the test suite (located in `tests/`) to verify the program's functionality. This command will automatically start a local test validator, deploy the program to it, and execute your TypeScript tests."
      code: "anchor test"
      
    - title: "3. Deploy to a Network"
      description: "You can deploy your smart contract to `localhost`, `devnet`, or `mainnet-beta`."
      steps:
        - title: "Step 3.1: Configure Solana CLI"
          description: "Set your Solana CLI to target the desired network."
          options:
            - name: "For Localhost"
              note: "(First, run `solana-test-validator` in a separate terminal)"
              code: "solana config set --url localhost"
            - name: "For Devnet"
              code: "solana config set --url devnet"
            - name: "For Mainnet"
              code: "solana config set --url mainnet-beta"
              
        - title: "Step 3.2: Check Wallet and Balance"
          description: "Deployment costs SOL for network fees and account rent."
          code: |
            # Check your current wallet address
            solana address

            # Check your balance
            solana balance
          note: "If you are on `devnet` and need SOL, you can request an airdrop:"
          note_code: "solana airdrop 4"
          
        - title: "Step 3.3: Deploy"
          description: "Deploy the program using the Anchor CLI."
          code: "anchor deploy"
          post_deploy_note: |
            **After the first deployment:**
            The command will output a new **Program ID** (the on-chain address of your contract). You must copy this Program ID and paste it into two places:
            
            1. `Anchor.toml` (in the `[programs.devnet]` or `[programs.mainnet]` section)
            2. `programs/zk-storage/src/lib.rs` (inside the `declare_id!("...")` macro)
            
            After updating the Program ID, run `anchor build` one more time to embed the new ID in the program's artifacts.