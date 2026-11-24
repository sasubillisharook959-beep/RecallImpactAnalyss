// Recall Impact Analysis for Contaminated Batches  (DAA project)
// BFS (forward recall + traceback) and Union-Find (connected clusters)
#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <random>
#include <memory>
using namespace std;

static string trim(string s) {
    const char* ws = " \t\r\n";
    size_t a = s.find_first_not_of(ws);
    if (a == string::npos) return "";
    return s.substr(a, s.find_last_not_of(ws) - a + 1);
}
static string up(string s) { for (auto& c : s) c = toupper((unsigned char)c); return s; }
static string inferType(const string& n) {
    if (n.empty()) return "Unknown";
    switch (n[0]) { case 'H': return "Hospital"; case 'W': return "Warehouse"; case 'B': return "Batch"; }
    return "Unknown";
}
static string jstr(const string& s) { return "\"" + s + "\""; }

// ---------- Disjoint Set Union: path compression + union by rank ----------
class UnionFind {
    vector<int> parent, rankv;
public:
    explicit UnionFind(int n) : parent(n), rankv(n, 0) { for (int i = 0; i < n; i++) parent[i] = i; }
    int find(int x) { return parent[x] == x ? x : parent[x] = find(parent[x]); }
    void unite(int a, int b) {
        a = find(a); b = find(b); if (a == b) return;
        if (rankv[a] < rankv[b]) swap(a, b);
        parent[b] = a; if (rankv[a] == rankv[b]) rankv[a]++;
    }
};

class SupplyNetwork {
    unordered_map<string, int> id;
    vector<string> name, type;
    vector<vector<int>> adj, radj;      // forward and reverse adjacency
    unique_ptr<UnionFind> dsu;

    int getId(const string& s) {
        auto it = id.find(s);
        if (it != id.end()) return it->second;
        int x = name.size(); id[s] = x; name.push_back(s); type.push_back(inferType(s));
        adj.emplace_back(); radj.emplace_back(); return x;
    }
    // BFS from s; fills depth (-1 = unreached) and parent; returns visit order
    vector<int> bfs(int s, vector<int>& depth, vector<int>& par) const {
        depth.assign(name.size(), -1); par.assign(name.size(), -1);
        vector<int> order; queue<int> q; q.push(s); depth[s] = 0;
        while (!q.empty()) {
            int u = q.front(); q.pop(); order.push_back(u);
            for (int v : adj[u]) if (depth[v] < 0) { depth[v] = depth[u] + 1; par[v] = u; q.push(v); }
        }
        return order;
    }
public:
    int nodes() const { return name.size(); }
    int edges() const { int e = 0; for (auto& a : adj) e += a.size(); return e; }

    void addNode(const string& n, const string& t) { type[getId(n)] = t; }
    bool addShipment(const string& a, const string& b) {
        int u = getId(a), v = getId(b);
        if (find(adj[u].begin(), adj[u].end(), v) != adj[u].end()) return false;  // ignore duplicates
        adj[u].push_back(v); radj[v].push_back(u); dsu.reset(); return true;
    }
    bool has(const string& n) const { return id.count(n) > 0; }

    void buildDSU() {
        dsu = make_unique<UnionFind>(name.size());
        for (int u = 0; u < (int)adj.size(); u++) for (int v : adj[u]) dsu->unite(u, v);
    }
    // ---- forward recall: all nodes reachable from a batch, with BFS depth ----
    vector<pair<string, int>> bfsRecall(const string& batch) {
        vector<pair<string, int>> out;
        if (!has(batch)) return out;
        vector<int> d, p;
        for (int u : bfs(id[batch], d, p)) out.push_back({name[u], d[u]});
        return out;
    }
    // ---- shortest delivery path batch -> target ----
    vector<string> path(const string& from, const string& to) {
        vector<string> out;
        if (!has(from) || !has(to)) return out;
        vector<int> d, p; bfs(id[from], d, p);
        if (d[id[to]] < 0) return out;
        for (int v = id[to]; v != -1; v = p[v]) out.push_back(name[v]);
        reverse(out.begin(), out.end()); return out;
    }
    // ---- traceback: every batch that can reach `node` (BFS on reversed edges) ----
    vector<string> traceback(const string& node) const {
        vector<string> out; auto it = id.find(node);
        if (it == id.end()) return out;
        vector<bool> vis(name.size(), false); queue<int> q; q.push(it->second); vis[it->second] = true;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            if (type[u] == "Batch") out.push_back(name[u]);
            for (int v : radj[u]) if (!vis[v]) { vis[v] = true; q.push(v); }
        }
        sort(out.begin(), out.end()); return out;
    }
    vector<vector<string>> clusters() {
        buildDSU();
        unordered_map<int, vector<string>> mp;
        for (int i = 0; i < (int)name.size(); i++) mp[dsu->find(i)].push_back(name[i]);
        vector<vector<string>> ans;
        for (auto& p : mp) { sort(p.second.begin(), p.second.end()); ans.push_back(p.second); }
        sort(ans.begin(), ans.end(), [](auto& a, auto& b) { return a[0] < b[0]; });
        return ans;
    }
    bool sameCluster(const string& a, const string& b) {
        if (!has(a) || !has(b)) return false;
        if (!dsu) buildDSU();
        return dsu->find(id[a]) == dsu->find(id[b]);
    }
    vector<string> batchNames() const { vector<string> b; for (int i = 0; i < nodes(); i++) if (type[i] == "Batch") b.push_back(name[i]); return b; }

    // ---------------- reports ----------------
    void recallReport(const string& batch) {
        auto r = bfsRecall(batch);
        cout << "\n========== RECALL IMPACT ANALYSIS ==========\n";
        if (r.empty()) { cout << "Batch '" << batch << "' not found. Known batches:"; for (auto& b : batchNames()) cout << " " << b; cout << "\n"; return; }
        cout << "Recalled Batch: " << batch << "\n\n";
        int maxd = 0; for (auto& x : r) maxd = max(maxd, x.second);
        for (const char* t : {"Warehouse", "Hospital"}) {
            cout << "Affected " << t << "s:\n"; bool any = false;
            for (auto& x : r) if (type[id[x.first]] == t) { cout << "  " << x.first << "  (hop " << x.second << ")\n"; any = true; }
            if (!any) cout << "  None\n";
        }
        cout << "Total reachable nodes (incl. batch): " << r.size() << "\nMaximum hops from batch: " << maxd << "\n";
    }
    void printClusters() {
        auto c = clusters();
        cout << "\n========== CONNECTED CLUSTERS ==========\n";
        for (size_t i = 0; i < c.size(); i++) {
            cout << "Cluster " << i + 1 << " (" << c[i].size() << " nodes): ";
            for (size_t j = 0; j < c[i].size(); j++) cout << (j ? ", " : "") << c[i][j];
            cout << "\n";
        }
    }
    void displayNetwork() {
        cout << "\n========== SUPPLY NETWORK (" << nodes() << " nodes, " << edges() << " shipments) ==========\n";
        for (int u = 0; u < (int)adj.size(); u++) {
            cout << name[u] << " (" << type[u] << ") -> ";
            for (size_t j = 0; j < adj[u].size(); j++) cout << (j ? ", " : "") << name[adj[u][j]];
            cout << "\n";
        }
    }
    void performance(int queries = 5000) {
        auto batches = batchNames();
        if (batches.empty()) { cout << "No batches in the network.\n"; return; }
        auto t1 = chrono::high_resolution_clock::now();
        size_t c1 = 0;
        for (int q = 0; q < queries; q++) c1 += bfsRecall(batches[q % batches.size()]).size();
        auto t2 = chrono::high_resolution_clock::now();
        buildDSU();
        size_t c2 = 0;
        for (int q = 0; q < queries; q++) c2 += dsu->find(id[batches[q % batches.size()]]) >= 0;
        auto t3 = chrono::high_resolution_clock::now();
        double bfsms = chrono::duration<double, milli>(t2 - t1).count();
        double dsums = chrono::duration<double, milli>(t3 - t2).count();
        cout << "\n========== PERFORMANCE COMPARISON ==========\n"
             << "Network: " << nodes() << " nodes, " << edges() << " edges | Queries: " << queries << "\n" << fixed << setprecision(3)
             << "Repeated BFS time:          " << bfsms << " ms  (O(V+E) per query)\n"
             << "Union-Find build + queries: " << dsums << " ms  (O(E a(V)) build, O(a(V)) per query)\n"
             << "BFS checksum: " << c1 << " | DSU checksum: " << c2 << "\n";
        if (dsums > 0) cout << "Measured speed-up (BFS/DSU): " << bfsms / dsums << "x\n";
        cout << "Note: BFS answers directed reachability; DSU answers undirected clustering.\n";
    }
    // ---------------- I/O ----------------
    bool loadCSV(const string& path) {
        ifstream f(path); if (!f) return false;
        string line; getline(f, line);
        while (getline(f, line)) {
            stringstream ss(line); string a, b, c;
            getline(ss, a, ','); getline(ss, b, ','); getline(ss, c, ',');
            a = up(trim(a)); b = trim(b); c = up(trim(c));
            if (a.empty() || c.empty()) continue;
            addNode(a, b.empty() ? inferType(a) : b); addNode(c, inferType(c));
            addShipment(a, c);
        }
        return true;
    }
    // Synthetic network for the scale benchmark: batches -> warehouses -> hospitals
    static SupplyNetwork synthetic(int nb, int nw, int nh, unsigned seed = 42) {
        SupplyNetwork n; mt19937 rng(seed);
        for (int b = 0; b < nb; b++) for (int k = 0; k < 3; k++) n.addShipment("B" + to_string(b), "W" + to_string(rng() % nw));
        for (int w = 0; w < nw; w++) for (int k = 0; k < 2; k++) n.addShipment("W" + to_string(w), "H" + to_string(rng() % nh));
        return n;
    }
    // Machine-readable dump (used to cross-check the web UI against this engine)
    string toJSON() {
        ostringstream o; o << "{\"nodes\":[";
        for (int i = 0; i < nodes(); i++) o << (i ? "," : "") << "{\"id\":" << jstr(name[i]) << ",\"type\":" << jstr(type[i]) << "}";
        o << "],\"edges\":[";
        bool first = true;
        for (int u = 0; u < nodes(); u++) for (int v : adj[u]) { o << (first ? "" : ",") << "[" << jstr(name[u]) << "," << jstr(name[v]) << "]"; first = false; }
        o << "],\"recalls\":{"; auto bs = batchNames();
        for (size_t i = 0; i < bs.size(); i++) {
            auto r = bfsRecall(bs[i]); o << (i ? "," : "") << jstr(bs[i]) << ":[";
            for (size_t j = 0; j < r.size(); j++) o << (j ? "," : "") << "[" << jstr(r[j].first) << "," << r[j].second << "]";
            o << "]";
        }
        o << "},\"clusters\":["; auto c = clusters();
        for (size_t i = 0; i < c.size(); i++) { o << (i ? "," : "") << "["; for (size_t j = 0; j < c[i].size(); j++) o << (j ? "," : "") << jstr(c[i][j]); o << "]"; }
        o << "]}"; return o.str();
    }
};

static string ask(const string& prompt) { string s; cout << prompt; cin >> s; return up(trim(s)); }

int main(int argc, char** argv) {
    string csv = "data/network.csv"; bool json = false;
    for (int i = 1; i < argc; i++) { string a = argv[i]; if (a == "--json") json = true; else csv = a; }
    SupplyNetwork net;
    if (!net.loadCSV(csv)) { cerr << "Could not open " << csv << "\n"; return 1; }
    if (json) { cout << net.toJSON() << "\n"; return 0; }

    while (true) {
        cout << "\n============================================\n   RECALL IMPACT ANALYSIS SYSTEM\n============================================\n"
             << "1. Recall a batch (BFS)\n2. Find connected clusters (Union-Find)\n3. Display supply network\n"
             << "4. Compare BFS vs Union-Find\n5. Trace delivery path (batch -> node)\n6. Traceback: which batches reached a node?\n"
             << "7. Add a shipment\n8. Scale benchmark (synthetic network)\n9. Exit\nEnter choice: ";
        int ch;
        if (!(cin >> ch)) { if (cin.eof()) break; cin.clear(); cin.ignore(10000, '\n'); cout << "Please enter a number from 1 to 9.\n"; continue; }
        if (ch == 1) net.recallReport(ask("Enter recalled batch: "));
        else if (ch == 2) net.printClusters();
        else if (ch == 3) net.displayNetwork();
        else if (ch == 4) net.performance();
        else if (ch == 5) {
            string b = ask("From batch: "), t = ask("To node: "); auto p = net.path(b, t);
            if (p.empty()) cout << "No delivery path from " << b << " to " << t << ".\n";
            else { cout << "Path (" << p.size() - 1 << " hops): "; for (size_t i = 0; i < p.size(); i++) cout << (i ? " -> " : "") << p[i]; cout << "\n"; }
        } else if (ch == 6) {
            string n = ask("Node (e.g. hospital): ");
            if (!net.has(n)) { cout << "Node not found.\n"; continue; }
            auto r = net.traceback(n); cout << "Batches that reached " << n << ": ";
            if (r.empty()) cout << "None";
            for (size_t i = 0; i < r.size(); i++) cout << (i ? ", " : "") << r[i];
            cout << "\n";
        } else if (ch == 7) {
            string a = ask("From: "), b = ask("To: ");
            cout << (net.addShipment(a, b) ? "Shipment added.\n" : "Shipment already exists.\n");
        } else if (ch == 8) {
            auto big = SupplyNetwork::synthetic(2000, 8000, 20000);
            cout << "\n(Synthetic network: 2000 batches, 8000 warehouses, 20000 hospitals)";
            big.performance(2000);
        } else if (ch == 9) { cout << "Exiting...\n"; break; }
        else cout << "Invalid choice.\n";
    }
    return 0;
}
