# 4-5 minute demonstration

1. Open `ui/index.html`. Explain the three columns: batches, warehouses, hospitals.
2. Recall (BFS): choose B101, press Step several times. Point at the queue, the red BFS tree, and the final lists (W1, W2; H1, H2, H3).
3. Traceback: pick H5. Only B103 can have reached it.
4. Delivery path: B101 to H3 gives B101 -> W2 -> H3.
5. Clusters (DSU): press Next union repeatedly, then Show all. Four clusters.
6. Edit network: add B102 -> W1 and re-run clusters. Two clusters merge into nine nodes.
7. Benchmark: choose the synthetic network and run. Explain why Union-Find is faster and why it is not a like-for-like comparison.
8. Show the console program: `./recall_analyzer`, option 1 with `b101`, then option 8.
