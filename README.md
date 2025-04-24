# Crypto Wallet Telegram Bot  

A bot for tracking cryptocurrency wallets and monitoring crypto exchange rates.  

## 🚀 Features  
- **📌 Wallet Management**  
  - Add cryptocurrency wallets for tracking  
  - View a list of linked wallets  

- **💰 Balance Information**  
  - Check balances for BTC, ETH, TON  
  - Transaction history  

- **📊 Rate Monitoring**  
  - Current cryptocurrency prices  
  - Price change graphs  

- **🔔 Notifications**  
  - Alerts for significant balance changes  
  - Notifications for sudden rate fluctuations  

## 🛠 Technologies  
- **Backend**: C++20  
- **Databases**:  
  - PostgreSQL - stores user and wallet data  
  - Redis - caches rate and balance data  
- **Libraries**:  
  - `tgbot-cpp` - Telegram API integration  
  - `pqxx` - PostgreSQL interaction  
  - `sw::redis++` - Redis interaction  
  - `Boost` - utilities and async operations  
  - `nlohmann::json` - JSON parsing  
  - `libcurl` - network transfers  

## ⚙️ Setup & Launch  

### Requirements  
- C++20 compiler (GCC 10+ or Clang 12+)  
- CMake 3.16+  
- PostgreSQL 12+  
- Redis 6+  

### Build  
```bash  
git clone https://github.com/Koshkovka/crypto-bot.git  
cd crypto-bot  
mkdir build && cd build  
cmake ..  
make -j$(nproc)  
```  

### Configuration  
Create a `config.json` file in the project root:  
```json  
{
  "BOT_TOKEN": "YOUR_BOT_TOKEN",
  "ETHERSCAN_TOKEN": "YOUR_ETHERSCAN_TOKEN",
  "postgres": {
    "host": "localhost",
    "database": "YOUR_DATABASE",
    "port": "5432",
    "username": "USERNAME",
    "password": "PASSWORD"
  },
  "redis": {
    "host": "localhost",
    "port": "6379"
  }
}
```  

### Launch  
```bash  
./build/crypto_bot  
```  

## 📋 Bot Commands  
| Command | Description |  
|---------|-------------|  
| `/start` | Start using the bot |  
| `/link_new_wallet` | Link a new wallet |  
| `/view_wallets` | View linked wallets |  
| `/wallet_info` | Get wallet details |  
| `/target_coins` | Monitor cryptocurrency rates |  
| `/help` | Show command list |  

## 🌐 API Integrations  
The bot integrates with:  
- **Blockchain.com API** – BTC wallet data  
- **Etherscan API** – ETH wallet data  
- **Tonapi** – TON wallet data  
- **CoinGecko API** – Cryptocurrency rates  

## 📈 Future Plans  
- [ ] Support for additional blockchains  
- [ ] Advanced portfolio analytics  
- [ ] Customizable notifications  
- [ ] Web interface for bot management  

## 📜 License  
MIT License. See `LICENSE` for details.  
