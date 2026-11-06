#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
using namespace std;

int main() {
    int addrBits, pdiBits, ptiBits, physicalSize;

    cout << "Enter virtual address size (bits): ";
    cin >> addrBits;
    cout << "Enter number of PDI bits: ";
    cin >> pdiBits;
    cout << "Enter number of PTI bits: ";
    cin >> ptiBits;
    cout << "Enter physical memory size (bytes): ";
    cin >> physicalSize;

    int offsetBits = addrBits - pdiBits - ptiBits;
    int pageSize = 1 << offsetBits;
    int pdEntries = 1 << pdiBits;
    int ptEntries = 1 << ptiBits;
    int pages = pdEntries * ptEntries;
    int frames = physicalSize / pageSize;
    int entrySize = 4;

    cout << "\nOffset bits = " << offsetBits << "\n";
    cout << "Page size = " << pageSize << " bytes\n";
    cout << "Number of virtual pages = " << pages << "\n";
    cout << "Number of frames = " << frames << "\n";
    cout << "Page directory = " << pdEntries << " entries, "
         << pdEntries * entrySize << " bytes\n";
    cout << "Each page table = " << ptEntries << " entries, "
         << ptEntries * entrySize << " bytes\n\n";

    vector<vector<int>> table(pdEntries, vector<int>(ptEntries, -1));
    vector<int> used(frames, 0);

    int n;
    cout << "How many pages do you want to map? ";
    cin >> n;

    for (int i = 0; i < n; i++) {
        int pdi, pti, f;
        cout << "Enter PDI, PTI and frame number: ";
        cin >> pdi >> pti >> f;
        if (pdi < 0 || pdi >= pdEntries || pti < 0 || pti >= ptEntries ||
            f < 0 || f >= frames || used[f] || table[pdi][pti] != -1) {
            cout << "Invalid entry, try again\n";
            i--;
            continue;
        }
        table[pdi][pti] = f;
        used[f] = 1;
    }

    vector<int> tablePresent(pdEntries, 0);
    int allocated = 0;
    for (int i = 0; i < pdEntries; i++)
        for (int j = 0; j < ptEntries; j++)
            if (table[i][j] != -1) tablePresent[i] = 1;

    cout << "\nPAGE DIRECTORY\n";
    cout << left << setw(6) << "PDI" << setw(14) << "Page Table" << "Presence Bit\n";
    for (int i = 0; i < pdEntries; i++) {
        cout << left << setw(6) << i;
        if (tablePresent[i]) {
            cout << setw(14) << ("PT" + to_string(i));
            allocated++;
        } else {
            cout << setw(14) << "-";
        }
        cout << tablePresent[i] << "\n";
    }

    for (int i = 0; i < pdEntries; i++) {
        if (!tablePresent[i]) continue;
        cout << "\nPAGE TABLE PT" << i << "\n";
        cout << left << setw(6) << "PTI" << setw(14) << "Frame Number" << "Presence Bit\n";
        for (int j = 0; j < ptEntries; j++) {
            cout << left << setw(6) << j;
            if (table[i][j] != -1) cout << setw(14) << table[i][j] << "1\n";
            else cout << setw(14) << "-" << "0\n";
        }
    }

    cout << "\nPage tables allocated = " << allocated << " of " << pdEntries
         << " (" << allocated * ptEntries * entrySize << " bytes)\n";

    while (true) {
        long la;
        cout << "\nEnter logical address (-1 to stop): ";
        cin >> la;
        if (la == -1) break;

        if (la < 0 || la >= (1L << addrBits)) {
            cout << "Invalid logical address\n";
            continue;
        }

        int offset = la % pageSize;
        int pti = (la >> offsetBits) % ptEntries;
        int pdi = la >> (offsetBits + ptiBits);
        cout << "PDI = " << pdi << ", PTI = " << pti << ", Offset = " << offset << "\n";

        if (!tablePresent[pdi]) {
            cout << "Page directory entry " << pdi << " not present: Page Fault\n";
            continue;
        }

        if (table[pdi][pti] == -1) {
            cout << "Page not present in PT" << pdi << ": Page Fault\n";
            continue;
        }

        int pa = table[pdi][pti] * pageSize + offset;
        cout << "Frame number = " << table[pdi][pti] << "\n";
        cout << "Physical address = " << table[pdi][pti] << " * " << pageSize
             << " + " << offset << " = " << pa << "\n";
    }

    return 0;
}
