#!/bin/bash
# Runs scripted sessions against ./recall_analyzer and prints the output of each test case.
cd "$(dirname "$0")/.." || exit 1
run(){ printf '%s\n' "TEST $1: $2"; printf "$3" | ./recall_analyzer | grep -vE '^(=+|   RECALL IMPACT ANALYSIS SYSTEM|[1-9]\. .*|)$' | sed 's/Enter choice: //g;/^$/d'; echo; }
run 1 "Recall B101 (two warehouses, three hospitals)" '1\nB101\n9\n'
run 2 "Recall B103 (batch split across two warehouses)" '1\nB103\n9\n'
run 3 "Lower-case input b104 is accepted" '1\nb104\n9\n'
run 4 "Unknown batch B999" '1\nB999\n9\n'
run 5 "Delivery path B101 to H3" '5\nB101\nH3\n9\n'
run 6 "No path B102 to H1" '5\nB102\nH1\n9\n'
run 7 "Traceback of hospital H5" '6\nH5\n9\n'
run 8 "New shipment W2->H9 then recall B101" '7\nW2\nH9\n1\nB101\n9\n'
run 9 "Duplicate shipment rejected" '7\nW1\nH1\n9\n'
run 10 "Shared warehouse merges clusters (B102->W1)" '7\nB102\nW1\n2\n9\n'
run 11 "Non-numeric menu input does not crash" 'abc\n9\n'
