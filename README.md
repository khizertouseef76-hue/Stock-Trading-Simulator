PSX Trading Simulator

A Stock Market Trading Simulator for the Pakistan Stock Exchange (PSX), built using C++ and SFML.
It demonstrates OOP principles and DSA concepts through an interactive, graphical trading experience.

🎮 Features

Simulated PSX Stocks:
HBL, ENGRO, UBL, LUCK, OGDC, PSO, SNGP, PPL, HUBC, NML.

Dynamic Prices: Real-time price updates with sector-based volatility.

Interactive Trading:

Place buy/sell orders

Cancel pending orders (FIFO queue)

Undo last trade (LIFO stack)

Portfolio Management:

Displays holdings per stock

Account balance and total equity

Profit/Loss calculation

Graphical UI:

Dark-themed, modern interface

Live stock graphs with history

Detailed stock overview and account summary

Persistent Data:

Metadata and price history stored locally for each stock

🛠 OOP & DSA Concepts

Classes & Objects: Stock, Portfolio, Market, UIManager

Inheritance & Polymorphism: Derived classes like BankingStock, EnergyStock, override updatePrice()

Encapsulation: Private attributes with getter/setter methods

Data Structures:

Vector: Stocks, holdings, and price history

Queue: Pending buy orders (FIFO)

Stack: Undo last sell (LIFO)

Modular Design: Separates UI, market logic, and portfolio handling

⌨ Controls
Key	Action
UP/DOWN	Navigate stock list
B	Place Buy Order (10 shares)
C	Cancel Pending Buy Order
S	Place Sell Order (10 shares)
U	Undo Last Sell
P	View Portfolio
T	Back to Stock View
ESC	Exit
🚀 Getting Started

Clone the repository:

git clone https://github.com/yourusername/PSX-Trading-Simulator.git
cd PSX-Trading-Simulator


Install SFML (Graphics, Window, System).

Compile the program:

g++ main.cpp -I"path_to_SFML/include" -L"path_to_SFML/lib" -lsfml-graphics -lsfml-window -lsfml-system -o app.exe


Folder structure:

project/
├─ main.cpp
├─ data/
│  ├─ metadata/
│  │  ├─ HBL_info.txt
│  │  ├─ ENGRO_info.txt
│  │  └─ ...etc
│  └─ history/
│     ├─ HBL.txt
│     ├─ ENGRO.txt
│     └─ ...etc
├─ assets/
│  └─ fonts/
│     └─ FRADMCN.TTF


Run:

./app.exe


📚 Libraries Used
Library	Purpose
<iostream>	Input/output
<fstream>	File handling (metadata & price history)
<vector>	Dynamic arrays (stocks, holdings, history)
<string>	String manipulation
<sstream>	Parsing metadata
<algorithm>	max, min, sorting
<cmath>	Mathematical operations
<ctime>	Random seed and timing
SFML	GUI rendering, event handling, and graphics

👤 Author

Khizer Ahmad – FAST University Student / C++ & AI Enthusiast