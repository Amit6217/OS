#include <iostream>
#include <iomanip>
#include <vector>
using namespace std;

int main() {
    int logicalSize, physicalSize, pageSize;

    cout << "Enter logical memory size (bytes): ";
    cin >> logicalSize;
    cout << "Enter physical memory size (bytes): ";
    cin >> physicalSize;
    cout << "Enter page size (bytes): ";
    cin >> pageSize;

    int pages = logicalSize / pageSize;
    int frames = physicalSize / pageSize;

    cout << "\nNumber of pages = " << pages << "\n";
    cout << "Number of frames = " << frames << "\n\n";

    vector<int> frameOf(pages, -1);
    vector<int> present(pages, 0);
    vector<int> pageIn(frames, -1);

    for (int i = 0; i < pages; i++) {
        int f;
        cout << "Frame number for page " << i << " (-1 if not in memory): ";
        cin >> f;
        while (f < -1 || f >= frames || (f >= 0 && pageIn[f] != -1)) {
            cout << "Invalid or already used frame, enter again: ";
            cin >> f;
        }
        if (f >= 0) {
            frameOf[i] = f;
            present[i] = 1;
            pageIn[f] = i;
        }
    }

    cout << "\nPAGE TABLE\n";
    cout << left << setw(8) << "Page" << setw(15) << "Page Address"
         << setw(14) << "Frame Number" << "Presence Bit\n";
    for (int i = 0; i < pages; i++) {
        cout << left << setw(8) << i << setw(15) << i * pageSize;
        if (present[i]) cout << setw(14) << frameOf[i];
        else cout << setw(14) << "-";
        cout << present[i] << "\n";
    }

    cout << "\nFRAME TABLE\n";
    cout << left << setw(8) << "Frame" << setw(16) << "Frame Address" << "Page Number\n";
    for (int i = 0; i < frames; i++) {
        cout << left << setw(8) << i << setw(16) << i * pageSize;
        if (pageIn[i] >= 0) cout << pageIn[i] << "\n";
        else cout << "-\n";
    }

    int base;
    cout << "\nEnter base address: ";
    cin >> base;

    while (true) {
        int la;
        cout << "\nEnter logical address (-1 to stop): ";
        cin >> la;
        if (la == -1) break;

        if (la < 0 || la >= logicalSize) {
            cout << "Invalid logical address\n";
            continue;
        }

        int page = la / pageSize;
        int offset = la % pageSize;
        cout << "Page number = " << page << ", Offset = " << offset << "\n";

        if (!present[page]) {
            cout << "Page " << page << " is not in memory: Page Fault\n";
            continue;
        }

        int pa = base + frameOf[page] * pageSize + offset;
        cout << "Frame number = " << frameOf[page] << "\n";
        cout << "Physical address = " << base << " + " << frameOf[page] << " * "
             << pageSize << " + " << offset << " = " << pa << "\n";
    }

    return 0;
}
