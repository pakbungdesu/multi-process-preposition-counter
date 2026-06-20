#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <ctime>
#include <cctype>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <vector>
using namespace std;

#define N_CHILD 3   // number of children

struct Prep {
    string word;
    int count;
};

vector<string> prep;   // store prepositions dynamically

// Load prepositions from file
int loadPrepositions(const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening prepositions file: " << filename << endl;
        return -1;
    }
    string word;
    while (file >> word) {
        prep.push_back(word);
    }
    file.close();
    return prep.size();
}

string toLower(const string &s) {
    string out = s;
    for (size_t i = 0; i < out.size(); i++) {
        out[i] = tolower(out[i]);
    }
    return out;
}

string removePunct(const string &s) {
    string out;
    for (char c : s) {
        if (isalpha(c) || isspace(c)) {
            out += c;
        }
    }
    return out;
}

void readAndCount(const string &filename, Prep arr[], int size) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        line = toLower(line);
        line = removePunct(line);

        stringstream ss(line);
        string word;
        while (ss >> word) {
            for (int i = 0; i < size; i++) {
                if (word == prep[i]) {
                    arr[i].count++;
                }
            }
        }
    }
    file.close();
}

string getOutputFilename() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char buffer[64];
    snprintf(buffer, sizeof(buffer),
             "output_%04d_%02d_%02d_%02d_%02d_%02d.csv",
             1900 + ltm->tm_year,
             1 + ltm->tm_mon,
             ltm->tm_mday,
             ltm->tm_hour,
             ltm->tm_min,
             ltm->tm_sec);
    return string(buffer);
}

int main() {
    // Load prepositions dynamically
    int SIZE = loadPrepositions("prepositions.txt");
    if (SIZE <= 0) {
        return 1; // cannot continue without prepositions
    }

    int shmid = shmget((key_t)2345, sizeof(Prep) * SIZE * N_CHILD, 0666 | IPC_CREAT);
    if (shmid < 0) {
        perror("shmget failed");
        return 1;
    }
    Prep *shared_memory = (Prep*) shmat(shmid, NULL, 0);

    pid_t pid;
    for (int i = 0; i < N_CHILD; i++) {
        pid = fork();
        if (pid == 0) {
            // Child
            Prep arr[SIZE];
            for (int j = 0; j < SIZE; j++) {
                arr[j].word = prep[j];
                arr[j].count = 0;
            }

            string filename = "lotr" + to_string(i+1) + ".txt";
            readAndCount(filename, arr, SIZE);

            // Write results into dedicated block
            for (int j = 0; j < SIZE; j++) {
                shared_memory[i*SIZE + j] = arr[j];
            }
            shmdt(shared_memory);
            exit(0);
        }
    }

    // Parent waits
    for (int i = 0; i < N_CHILD; i++) {
        wait(NULL);
    }

    // Merge results
    Prep finalArr[SIZE];
    for (int j = 0; j < SIZE; j++) {
        finalArr[j].word = prep[j];
        finalArr[j].count = 0;
    }

    for (int c = 0; c < N_CHILD; c++) {
        for (int j = 0; j < SIZE; j++) {
            finalArr[j].count += shared_memory[c*SIZE + j].count;
        }
    }

    // Print results
    cout << "\nFinal counts across all files:\n";
    for (int j = 0; j < SIZE; j++) {
        cout << finalArr[j].word << " | " << finalArr[j].count << endl;
    }

    // Save CSV
    string filename = getOutputFilename();
    ofstream fout(filename);
    fout << "Preposition|Count\n";
    for (int j = 0; j < SIZE; j++) {
        fout << finalArr[j].word << "|" << finalArr[j].count << "\n";
    }
    fout.close();

    cout << "\nResults saved to " << filename << endl;

    shmdt(shared_memory);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
