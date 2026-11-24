# Recall Impact Analysis for Contaminated Batches

DAA project, SRM University AP. Submitted to Prof. Nihar Ranjan.

Team: Sharook Sasubilli (AP25110010783), Goutham Rajana (AP25110010476), Avinash Reddy (AP25110010982), Gowtham Krishna (AP25110010997).

## Problem
When a batch is declared unsafe, find every warehouse and hospital that received it directly or indirectly, find which batches could have reached a given hospital, and group the supply network into connected clusters.

## Algorithms
- **BFS** on the directed shipment graph: recall (with hop depth), shortest delivery path, traceback on the reversed graph. O(V + E) per query.
- **Union-Find** (path compression + union by rank): connected clusters. O(alpha(V)) amortised per operation.

## Project layout
```
src/main.cpp        C++17 engine and console menu
ui/index.html       Web UI (open in any browser, no server needed)
data/network.csv    Sample network (source,type,target)
tests/run_tests.sh  Eleven scripted test cases
docs/               Report (DOCX/PDF), presentation (PPTX), sample and test output, demo script
```

## Console program
```bash
g++ -std=c++17 -O2 src/main.cpp -o recall_analyzer
./recall_analyzer                 # run from the project root
./recall_analyzer other.csv       # use another network file
./recall_analyzer --json          # dump network, recalls and clusters as JSON
```
Windows (PowerShell): `g++ -std=c++17 -O2 src/main.cpp -o recall_analyzer.exe` then `.\recall_analyzer.exe`

Menu: 1 Recall (BFS), 2 Clusters (Union-Find), 3 Display network, 4 Compare BFS vs Union-Find, 5 Delivery path, 6 Traceback, 7 Add shipment, 8 Scale benchmark (21,000 nodes), 9 Exit.

## Web UI
Double-click `ui/index.html`. Views: Recall (BFS) with Step/Play/Show all and a live queue, Traceback, Delivery path, Clusters (DSU) union by union, Benchmark. You can add shipments, load your own CSV and download the edited network.

## Tests
```bash
bash tests/run_tests.sh
```
Expected: sample recall of B101 gives warehouses W1, W2 and hospitals H1, H2, H3. The web UI was checked against `./recall_analyzer --json` and matched exactly.

## DAA note
BFS answers directed reachability. Union-Find treats every shipment as an undirected link, so it answers connectivity, not downstream impact. The benchmark therefore compares a harder question (BFS) with an easier one (Union-Find); use Union-Find for network structure and BFS for the exact recall list.

## CSV format
```
source,type,target
B101,Batch,W1
```
Names are case-insensitive. Types are inferred from the first letter (B, W, H) when blank.
