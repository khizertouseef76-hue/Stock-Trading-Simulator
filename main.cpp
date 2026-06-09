#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <ctime>

using namespace std;

//  Utility 
float randomFloat(float a, float b) {
    float r = (float)rand() / RAND_MAX;
    return a + r * (b - a);
}
 
//  Trade & Pending Order
struct Trade {
    string ticker;
    int qty;
    float price;
    bool isBuy;
};

struct PendingOrder {
    Trade t;
    float timePlaced;
};

// Stock Base Class 
class Stock {
protected:
    string name, ticker, sector, overview, ceo;
    long long marketCap = 0;
    int foundedYear = 0;

    float price, vol;
    vector<float> history;

    void loadMeta() {
        ifstream f("data/metadata/" + ticker + "_info.txt");
        if (!f.is_open()) return;
        string line;
        while (getline(f, line)) {
            auto p = line.find(':');
            if (p == string::npos) continue;
            string k = line.substr(0, p);
            string v = line.substr(p + 1);
            if (!v.empty() && v[0] == ' ') v.erase(0, 1);

            if (k == "Name") name = v;
            else if (k == "Ticker") ticker = v;
            else if (k == "Sector") sector = v;
            else if (k == "Overview") overview = v;
            else if (k == "CEO") ceo = v;
            else if (k == "MarketCap") marketCap = stoll(v);
            else if (k == "Founded") foundedYear = stoi(v);
        }
    }

    void loadHistory() {
        ifstream f("data/history/" + ticker + ".txt");
        float p;
        while (f >> p) history.push_back(p);
        if (history.empty()) history.push_back(price);
        price = history.back(); 
    }

    void savePrice() {
        ofstream f("data/history/" + ticker + ".txt", ios::app);
        f << price << "\n";
    }

public:
    Stock(string t, float base, float v) : ticker(t), price(base), vol(v) {
        loadMeta();
        loadHistory();
    }
    virtual void updatePrice() = 0;

    void step() {
        updatePrice();
        history.push_back(price);
        if (history.size() > 300) history.erase(history.begin());
        savePrice();
    }

    string getTicker() const { return ticker; }
    string getName() const { return name; }
    string getSector() const { return sector; }
    string getOverview() const { return overview; }
    string getCEO() const { return ceo; }
    float getPrice() const { return price; }
    long long getMarketCap() const { return marketCap; }
    int getFoundedYear() const { return foundedYear; }
    const vector<float>& getHistory() const { return history; }
};

//  Derived Stocks 
class Banking : public Stock {
public:
    Banking(string t, float p) : Stock(t, p, 0.8f) {}
    void updatePrice() override { price = max(10.f, price + randomFloat(-1, 1) * vol); }
};
class Energy : public Stock {
public:
    Energy(string t, float p) : Stock(t, p, 1.2f) {}
    void updatePrice() override { price = max(5.f, price + randomFloat(-1.5f, 1.5f) * vol); }
};
class Industrial : public Stock {
public:
    Industrial(string t, float p) : Stock(t, p, 1.0f) {}
    void updatePrice() override { price = max(15.f, price + randomFloat(-1, 1.5f)); }
};
class Utility : public Stock {
public:
    Utility(string t, float p) : Stock(t, p, 0.6f) {}
    void updatePrice() override { price = max(5.f, price + randomFloat(-0.7f, 0.7f)); }
};
class Textile : public Stock {
public:
    Textile(string t, float p) : Stock(t, p, 1.3f) {}
    void updatePrice() override { price = max(5.f, price + randomFloat(-1.8f, 1.8f)); }
};

//  Portfolio (Holdings)
struct Holding { Stock* s; int qty; };

class Portfolio {
    float cash;
    vector<Holding> hold;
public:
    Portfolio(float c = 200000) : cash(c) {}

    float getCash() const { return cash; }
    void addCash(float c) { cash += c; }

    bool buy(Stock* s, int q, float price) {
        float cost = q * price;
        if (cost > cash) return false;
        cash -= cost;

        for (auto& h : hold) {
            if (h.s == s) { h.qty += q; return true; }
        }
        hold.push_back({ s, q });
        return true;
    }

    bool sell(Stock* s, int q, float price) {
        for (auto& h : hold) {
            if (h.s == s && h.qty >= q) {
                h.qty -= q;
                cash += q * price;
                return true;
            }
        } 
        return false;
    }

    int qty(Stock* s) const {
        for (auto& h : hold)
            if (h.s == s) 
        return h.qty;
        return 0;
    }

    const vector<Holding>& getHoldings() const { return hold; }
};

//  Stock Market 
class Market {
public:
    vector<Stock*> stocks;

 vector<PendingOrder> pendingBuy; 
    vector<Trade> undoStack;         
    string lastAction = "";

    Portfolio& pf;
    Market(Portfolio& P) : pf(P) {}

    ~Market() { for (auto s : stocks) delete s; }

    Stock* find(string t) {
        for (auto s : stocks) if (s->getTicker() == t) return s;
        return nullptr;
    }

    void placeBuy(string t, int q) {
        Stock* s = find(t);
    if (!s) return;
        PendingOrder p{ {t, q, s->getPrice(), true}, clock() / 1000.f };
        pendingBuy.push_back(p);
        lastAction = "Order placed (10 sec to fill)";
    }

    void cancelBuy() {
        if (pendingBuy.empty()) {
            lastAction = "No pending orders";
            return;
        } 
        pendingBuy.erase(pendingBuy.begin());
        lastAction = "Order cancelled";
    }

    void placeSell(string t, int q) {
        Stock* s = find(t);
        if (!s) return;
        if (pf.sell(s, q, s->getPrice())) {
            undoStack.push_back({ t, q, s->getPrice(), false });
            lastAction = "Sold " + t;
        }
    }

    void undoSell() {
        if (undoStack.empty()) { lastAction = "Nothing to undo"; return; }
        Trade t = undoStack.back(); undoStack.pop_back();
        Stock* s = find(t.ticker);
        if (s) pf.buy(s, t.qty, t.price);
        lastAction = "Undo successful";
    }

    void step() {
        float now = clock() / 1000.f;

   if (!pendingBuy.empty()) {
            PendingOrder& p = pendingBuy.front();
            if (now - p.timePlaced >= 10.f) {
                Stock* s = find(p.t.ticker);
                if (s && pf.buy(s, p.t.qty, p.t.price))
                    lastAction = "Order executed";
                pendingBuy.erase(pendingBuy.begin());
            }
        }

        for (auto s : stocks) s->step();
    }
};

// ---------------- UI ----------------
class UI {
    sf::Font font;
    sf::Vector2u size;
public:
    UI(sf::Vector2u s) : size(s) {}
    void loadFont(string p) { font.loadFromFile(p);
     }

    void bg(sf::RenderWindow& w) {
        sf::RectangleShape r(sf::Vector2f(size.x, size.y));
        r.setFillColor(sf::Color(15, 15, 30)); w.draw(r);
    }

    void top(sf::RenderWindow& w) {
        sf::RectangleShape r(sf::Vector2f(size.x, 60));
        r.setFillColor(sf::Color(20, 20, 35)); w.draw(r);

        sf::Text t("Stock Market Trading Simulator", font, 22);
        t.setFillColor(sf::Color::White);
        t.setPosition(20, 15);
        w.draw(t);
    }

    void leftPanel(sf::RenderWindow& w) {
        sf::RectangleShape r(sf::Vector2f(250, size.y));
        r.setFillColor(sf::Color(25, 25, 45)); w.draw(r);
    }

    void stockList(sf::RenderWindow& w, const vector<Stock*>& st, int sel) {
        float y = 80;
        for (int i = 0; i < st.size(); i++) {
            sf::RectangleShape r(sf::Vector2f(250, 35));
            r.setPosition(0, y);
            r.setFillColor(i == sel ? sf::Color(50, 80, 120) : sf::Color(30, 30, 55));
            w.draw(r);

            sf::Text t(st[i]->getTicker() + "  " + st[i]->getName(), font, 14);
            t.setFillColor(sf::Color::White);
            t.setPosition(10, y + 8);
            w.draw(t);
            y += 40;
        }
    }

void portfolioList(sf::RenderWindow& w, const Portfolio& pf) {
    float y = 80;

    sf::Text title("My Portfolio", font, 18);
    title.setFillColor(sf::Color::White);
    title.setPosition(10, y - 40);
    w.draw(title);

    float totalValue = 0;
    float totalPL = 0;

    for (auto& h : pf.getHoldings()) {
        sf::RectangleShape r(sf::Vector2f(250, 45));
        r.setPosition(0, y);
        r.setFillColor(sf::Color(30, 30, 55));
        w.draw(r);

        float price = h.s->getPrice();
        float value = price * h.qty;
        float initialPrice = h.s->getHistory().front(); // reference price
        float pl = (price - initialPrice) * h.qty;

        totalValue += value;
        totalPL += pl;

        stringstream ss;
        ss << h.s->getTicker() 
           << " | Qty: " << h.qty
           << " | P/L: " << (pl >= 0 ? "+" : "") << pl;

        sf::Text t(ss.str(), font, 14);
        t.setFillColor(pl >= 0 ? sf::Color::Green : sf::Color::Red);
        t.setPosition(10, y + 8);
        w.draw(t);

        y += 50;
    }

    // Totals panel
    sf::RectangleShape tot(sf::Vector2f(250, 70));
    tot.setPosition(0, y + 10);
    tot.setFillColor(sf::Color(20, 20, 40));
    w.draw(tot);

    sf::Text t1("Portfolio Value: " + to_string((int)totalValue), font, 14);
    t1.setFillColor(sf::Color::White);
    t1.setPosition(10, y + 20);
    w.draw(t1);

    sf::Text t2("Cash: " + to_string((int)pf.getCash()), font, 14);
    t2.setFillColor(sf::Color::White);
    t2.setPosition(10, y + 40);
    w.draw(t2);

    sf::Text t3("Total P/L: " + string(totalPL >= 0 ? "+" : "") + to_string((int)totalPL), font, 14);
    t3.setFillColor(totalPL >= 0 ? sf::Color::Green : sf::Color::Red);
    t3.setPosition(10, y + 60);
    w.draw(t3);
}

    void details(sf::RenderWindow& w, Stock* s, const Portfolio& pf) {
        if (!s) return;
        float x = 270, y = 80;

        sf::RectangleShape r(sf::Vector2f(size.x - x - 20, 200));
        r.setPosition(x, y);
        r.setFillColor(sf::Color(25, 25, 50)); w.draw(r);

        sf::Text t(s->getTicker() + " - " + s->getName(), font, 20);
        t.setFillColor(sf::Color::White); t.setPosition(x + 15, y + 10);
        w.draw(t);

        sf::Text info("", font, 14);
        info.setFillColor(sf::Color::White);
        float yy = y + 50;

        info.setString("Sector: " + s->getSector()); info.setPosition(x+15, yy); w.draw(info); yy+=20;
        info.setString("CEO: " + s->getCEO()); info.setPosition(x+15, yy); w.draw(info); yy+=20;
        info.setString("Founded: " + to_string(s->getFoundedYear())); info.setPosition(x+15, yy); w.draw(info); yy+=20;

        info.setString("Price: PKR " + to_string(s->getPrice())); info.setPosition(x+15, yy); w.draw(info); yy+=20;
        info.setString("You Own: " + to_string(pf.qty(s)) + " shares");
        info.setPosition(x+15, yy); w.draw(info);
    }

    void graph(sf::RenderWindow& w, Stock* s) {
        if (!s) return;
        const auto& h = s->getHistory();
        if (h.size() < 2) return;

        float x = 270, y = 300, wth = size.x - x - 20, hgt = size.y - y - 20;
        sf::RectangleShape bg(sf::Vector2f(wth, hgt));
        bg.setPosition(x, y); bg.setFillColor(sf::Color(20,20,40));
        w.draw(bg);

        float mx = *max_element(h.begin(), h.end());
        float mn = *min_element(h.begin(), h.end());
        if (mx - mn < 0.1f) mx += 1;

        sf::VertexArray line(sf::LineStrip, h.size());
        for (int i = 0; i < h.size(); i++) {
            float xx = x + 10 + (float)i / (h.size() - 1) * (wth - 20);
            float yy = y + hgt - 10 - ((h[i] - mn) / (mx - mn)) * (hgt - 20);
            line[i].position = sf::Vector2f(xx, yy);
            line[i].color = sf::Color::Green;
        }
        w.draw(line);
    }

    void account(sf::RenderWindow& w, const Portfolio& pf) {
        float x = 270, y = 250;
        sf::RectangleShape r(sf::Vector2f(size.x - x - 20, 40));
        r.setFillColor(sf::Color(25,25,45));
        r.setPosition(x, y);
        w.draw(r);

        stringstream ss;
        ss << "Cash: PKR " << pf.getCash();
        sf::Text t(ss.str(), font, 16);
        t.setFillColor(sf::Color::White);
        t.setPosition(x + 15, y + 8);
        w.draw(t);
    }

    void help(sf::RenderWindow& w) {
        sf::Text t("[UP/DOWN] Select   [B] Buy  [S] Sell  [U] Undo  [C] Cancel Buy  [P] Portfolio  [T] Trade", font, 14);
        t.setFillColor(sf::Color(180,180,200));
        t.setPosition(270, 60);
        w.draw(t);
    }

    void pendingStatus(sf::RenderWindow& w, int count, string last) {
        sf::Text t("Pending Orders: " + to_string(count) + "   Last Action: " + last, font, 14);
        t.setFillColor(sf::Color::Yellow);
        t.setPosition(270, size.y - 30);
        w.draw(t);
    }
};

// ---------------- Main ----------------
int main() {
    srand(time(nullptr));

    sf::RenderWindow win(sf::VideoMode(1200,720), "Stock Market Trading Simulator");
    win.setFramerateLimit(60);

    Portfolio pf;
    Market mk(pf);
    UI ui(win.getSize());
    ui.loadFont("D:/Maze Game/assets/fonts/FRADMCN.TTF");

    //Pakistani Stocks
    mk.stocks.push_back(new Banking("HBL",130));
    mk.stocks.push_back(new Banking("MCB",170));
    mk.stocks.push_back(new Banking("UBL",125));
    mk.stocks.push_back(new Industrial("ENGRO",220));
    mk.stocks.push_back(new Energy("OGDC",100));
    mk.stocks.push_back(new Energy("PSO",135));
    mk.stocks.push_back(new Utility("SNGP",55));
    mk.stocks.push_back(new Energy("PPL",80));
    mk.stocks.push_back(new Industrial("LUCK",650));
    mk.stocks.push_back(new Energy("HUBC",75));
    mk.stocks.push_back(new Textile("NML",50));

    int sel = 0;
    bool portfolioView = false;

    sf::Clock clk;

    while (win.isOpen()) {
        sf::Event e;
        while (win.pollEvent(e)) {
            if (e.type == sf::Event::Closed) win.close();

            if (e.type == sf::Event::KeyPressed) {
                auto& s = mk.stocks;

                if (e.key.code == sf::Keyboard::Escape) win.close();
                else if (e.key.code == sf::Keyboard::P) portfolioView = true;
                else if (e.key.code == sf::Keyboard::T) portfolioView = false;

                if (!portfolioView) {
                    if (e.key.code == sf::Keyboard::Up) sel = (sel - 1 + s.size()) % s.size();
                    if (e.key.code == sf::Keyboard::Down) sel = (sel + 1) % s.size();
                    if (e.key.code == sf::Keyboard::B) mk.placeBuy(s[sel]->getTicker(), 10);
                    if (e.key.code == sf::Keyboard::C) mk.cancelBuy();
                    if (e.key.code == sf::Keyboard::S) mk.placeSell(s[sel]->getTicker(), 10);
                    if (e.key.code == sf::Keyboard::U) mk.undoSell();
                }
            }
        }

        if (clk.getElapsedTime().asSeconds() >= 1.5) {
            mk.step();
            clk.restart();
        }

        win.clear();
        ui.bg(win);
        ui.top(win);
        ui.leftPanel(win);
        ui.help(win);

        if (portfolioView) {
            ui.portfolioList(win, pf);
        } else {
            ui.stockList(win, mk.stocks, sel);
            ui.details(win, mk.stocks[sel], pf);
            ui.graph(win, mk.stocks[sel]);
            ui.account(win, pf);
        }

        ui.pendingStatus(win, mk.pendingBuy.size(), mk.lastAction);
        win.display();
    }
}
