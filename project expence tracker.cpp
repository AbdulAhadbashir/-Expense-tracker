#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <map>
#include <sstream>
#include <algorithm>

using namespace std;

string currentDate() {
    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);
    char date[20];
    sprintf_s(date, "%04d-%02d-%02d", 1900 + ltm.tm_year, 1 + ltm.tm_mon, ltm.tm_mday);
    return string(date);
}

bool isDateInRange(const string& date, const string& start, const string& end) {
    return date >= start && date <= end;
}

class User {
private:
    string username;
    string password;
public:
    User(string u = "", string p = "") : username(u), password(p) {}

    bool login(string u, string p) const {
        return (username == u && password == p);
    }

    string getUsername() const { return username; }
};

class Expense {
private:
    int id;
    string user;
    string date;
    string category;
    double amount;
    string note;

public:
    Expense(int i = 0, string us = "", string d = "", string c = "", double a = 0.0, string n = "")
        : id(i), user(us), date(d), category(c), amount(a), note(n) {
    }

    void inputExpense(string u) {
        user = u;
        cout << "Enter category: ";
        cin.ignore();
        getline(cin, category);
        cout << "Enter amount: ";
        cin >> amount;
        cin.ignore();
        cout << "Enter note: ";
        getline(cin, note);
        date = currentDate();
    }

    void display() const {
        cout << setw(5) << id << setw(15) << date << setw(15) << category
            << setw(10) << amount << setw(20) << note << endl;
    }

    int getID() const { return id; }
    string getUser() const { return user; }
    string getCategory() const { return category; }
    string getDate() const { return date; }
    double getAmount() const { return amount; }

    void setID(int i) { id = i; }

    void serialize(ofstream& out) const {
        out << id << "," << user << "," << date << "," << category << "," << amount << "," << note << endl;
    }

    static Expense deserialize(const string& line) {
        int id;
        double amount;
        string user, date, category, note;

        stringstream ss(line);
        string token;

        getline(ss, token, ',');
        id = stoi(token);
        getline(ss, user, ',');
        getline(ss, date, ',');
        getline(ss, category, ',');
        getline(ss, token, ',');
        amount = stod(token);
        getline(ss, note);

        return Expense(id, user, date, category, amount, note);
    }

    void serializeCSV(ofstream& out) const {
        out << id << "," << user << "," << date << "," << category << "," << amount << "," << note << "\n";
    }
};

class ExpenseManager {
private:
    vector<Expense> expenses;
    int nextID;
    string currentUser;

public:
    ExpenseManager(string user) {
        nextID = 1;
        currentUser = user;
        loadFromFile();
    }

    void addExpense() {
        Expense e;
        e.setID(nextID++);
        e.inputExpense(currentUser);
        expenses.push_back(e);
        cout << "Expense added successfully!\n";
    }

    void viewExpenses() {
        cout << left << setw(5) << "ID" << setw(15) << "Date"
            << setw(15) << "Category" << setw(10) << "Amount"
            << setw(20) << "Note" << endl;
        for (const auto& e : expenses) {
            if (e.getUser() == currentUser)
                e.display();
        }
    }

    void deleteExpense() {
        int id;
        cout << "Enter ID to delete: ";
        cin >> id;
        for (auto it = expenses.begin(); it != expenses.end(); ++it) {
            if (it->getID() == id && it->getUser() == currentUser) {
                expenses.erase(it);
                cout << "Deleted successfully.\n";
                return;
            }
        }
        cout << "ID not found or unauthorized.\n";
    }

    void updateExpense() {
        int id;
        cout << "Enter ID to update: ";
        cin >> id;
        for (auto& e : expenses) {
            if (e.getID() == id && e.getUser() == currentUser) {
                e.inputExpense(currentUser);
                e.setID(id);
                cout << "Updated successfully.\n";
                return;
            }
        }
        cout << "ID not found or unauthorized.\n";
    }

    void searchByCategory() {
        string category;
        cout << "Enter category: ";
        cin.ignore();
        getline(cin, category);
        for (const auto& e : expenses) {
            if (e.getUser() == currentUser && e.getCategory() == category) {
                e.display();
            }
        }
    }

    void filterByDateRange() {
        string start, end;
        cout << "Enter start date (YYYY-MM-DD): ";
        cin >> start;
        cout << "Enter end date (YYYY-MM-DD): ";
        cin >> end;
        for (const auto& e : expenses) {
            if (e.getUser() == currentUser && isDateInRange(e.getDate(), start, end)) {
                e.display();
            }
        }
    }

    void generateReport() {
        double total = 0;
        map<string, double> categoryTotals;

        cout << "\n=== Expense Report by Category ===\n";
        for (const auto& e : expenses) {
            if (e.getUser() == currentUser) {
                total += e.getAmount();
                categoryTotals[e.getCategory()] += e.getAmount();
            }
        }

        if (categoryTotals.empty()) {
            cout << "No expense data found for user.\n";
        }
        else {
            cout << "Total Expenses: " << total << "\n";
            cout << "----------------------------------\n";
            for (const auto& entry : categoryTotals) {
                cout << "Category: " << setw(15) << left << entry.first
                    << "Amount: " << entry.second << "\n";
            }
        }
    }

    void monthlyReport() {
        map<string, double> monthTotals;
        int count = 0;

        for (const auto& e : expenses) {
            if (e.getUser() == currentUser) {
                string month = e.getDate().substr(0, 7);
                monthTotals[month] += e.getAmount();
                count++;
            }
        }

        cout << "\n=== Monthly Expense Summary ===\n";
        if (monthTotals.empty()) {
            cout << "No records found to generate monthly report.\n";
        }
        else {
            for (const auto& entry : monthTotals) {
                cout << "Month: " << entry.first << " | Total: " << entry.second << "\n";
            }
        }

        cout << "Total Records Processed: " << count << "\n";
    }

    void exportCSV() {
        string filename;
        cout << "Enter filename to export (e.g., expenses_user.csv): ";
        cin >> filename;

        ofstream fout(filename);
        if (!fout.is_open()) {
            cout << "Error: Unable to create/open file.\n";
            return;
        }

        fout << "ID,User,Date,Category,Amount,Note\n";
        int exported = 0;
        for (const auto& e : expenses) {
            if (e.getUser() == currentUser) {
                e.serializeCSV(fout);
                exported++;
            }
        }

        fout.close();
        if (exported > 0)
            cout << "Successfully exported " << exported << " records to " << filename << "\n";
        else
            cout << "No records exported. No data available for current user.\n";
    }

    void saveToFile() {
        ofstream fout("expenses.txt");
        for (const auto& e : expenses) {
            e.serialize(fout);
        }
        fout.close();
    }

    void loadFromFile() {
        ifstream fin("expenses.txt");
        if (!fin.is_open()) {
            cout << "Note: No existing file found. A new one will be created on exit.\n";
            return;
        }

        string line;
        int loaded = 0;
        while (getline(fin, line)) {
            if (!line.empty()) {
                Expense e = Expense::deserialize(line);
                expenses.push_back(e);
                nextID = max(nextID, e.getID() + 1);
                loaded++;
            }
        }

        fin.close();
        cout << "Loaded " << loaded << " expense records from file.\n";
    }
};

void pauseAndContinue() {
    cout << "\nPress Enter to continue...";
    cin.ignore();
    cin.get();
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void showHeader(const string& username) {
    clearScreen();
    cout << "======================================\n";
    cout << "     Expense Tracker - " << username << "\n";
    cout << "======================================\n";
}

void userMenu(ExpenseManager& manager) {
    int choice;
    do {
        showHeader("User");

        cout << "1. Add New Expense\n";
        cout << "2. View All Expenses\n";
        cout << "3. Update Existing Expense\n";
        cout << "4. Delete Expense\n";
        cout << "5. Search Expenses by Category\n";
        cout << "6. Filter Expenses by Date Range\n";
        cout << "7. Generate Category-wise Report\n";
        cout << "8. View Monthly Report\n";
        cout << "9. Export Expenses to CSV\n";
        cout << "10. Help / About\n";
        cout << "0. Logout\n";
        cout << "--------------------------------------\n";
        cout << "Enter your choice: ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input. Please enter a number.\n";
            pauseAndContinue();
            continue;
        }

        switch (choice) {
        case 1: manager.addExpense(); break;
        case 2: manager.viewExpenses(); break;
        case 3: manager.updateExpense(); break;
        case 4: manager.deleteExpense(); break;
        case 5: manager.searchByCategory(); break;
        case 6: manager.filterByDateRange(); break;
        case 7: manager.generateReport(); break;
        case 8: manager.monthlyReport(); break;
        case 9: manager.exportCSV(); break;
        case 10:
            cout << "\n--- About This Application ---\n";
            cout << "C++ Based Personal Expense Tracker\n";
            cout << "Developed for BSSE F24 - IIUI OOP Project\n";
            cout << "Features: File I/O, Reports, Menu Interface\n";
            break;
        case 0:
            cout << "Logging out and saving data...\n";
            manager.saveToFile();
            break;
        default:
            cout << "Invalid option. Please try again.\n";
        }

        if (choice != 0)
            pauseAndContinue();

    } while (choice != 0);
}

void welcomeBanner() {
    cout << "\n=========================================\n";
    cout << "         Welcome to Expense Tracker       \n";
    cout << "        BSSE F24 - IIUI OOP Project       \n";
    cout << "=========================================\n";
    cout << "Tip: Login using one of the default users\n";
}

void goodbyeBanner() {
    cout << "\n=========================================\n";
    cout << "   Thank you for using Expense Tracker!   \n";
    cout << "=========================================\n";
}

void displayUsers(const vector<User>& users) {
    cout << "\n--- Available User Accounts ---\n";
    for (const auto& u : users) {
        cout << " - Username: " << u.getUsername() << "\n";
    }
}

bool loginUser(const vector<User>& users, string& uname) {
    string pass;
    int attempts = 0;
    while (attempts < 3) {
        cout << "\nLogin Attempt " << (attempts + 1) << " of 3\n";
        cout << "Username: ";
        cin >> uname;
        cout << "Password: ";
        cin >> pass;

        for (const auto& u : users) {
            if (u.login(uname, pass)) {
                cout << "Login successful. Welcome " << uname << "!\n";
                return true;
            }
        }

        cout << "Incorrect credentials. Try again.\n";
        attempts++;
    }
    return false;
}

int main() {
    vector<User> users = {
        User("user1", "pass1"),
        User("user2", "pass2"),
        User("user3", "pass3")
    };

    welcomeBanner();
    displayUsers(users);

    string username;
    if (!loginUser(users, username)) {
        cout << "Too many failed attempts. Exiting...\n";
        return 0;
    }

    ExpenseManager manager(username);
    userMenu(manager);
    goodbyeBanner();

    return 0;
}

