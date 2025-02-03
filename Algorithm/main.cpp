
#include <iostream>
#include <string>
#include <iomanip>
#include <cctype>
#include <limits>
#include "sqlite3.h"
using namespace std;

struct Employee {
    int id;
    string name;
    string role;
    double hourlyRate;
    double workHours;
    double overtimeHours;
    double bonuses;
    double taxRate;
    double providentFund;
    string bankAccountNumber;
    double grossSalary;
    double netSalary;
    Employee* next;
};

Employee* head = NULL;

sqlite3* connectDatabase() {
    sqlite3* db;
    int rc = sqlite3_open("algoproject.db", &db);
    if (rc) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return NULL;
    }
    return db;
}

bool checkUserExists(sqlite3* db, const string& username) {
    sqlite3_stmt* stmt;
    string sql = "SELECT COUNT(*) FROM Users WHERE Username = ?;";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int count = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);
            return count > 0;
        }
        sqlite3_finalize(stmt);
    }
    return false;
}

void signUp() {
    sqlite3* db = connectDatabase();
    if (!db) return;

    string username, password;
    cout << "Enter new username: ";
    cin >> username;
    cout << "Enter new password: ";
    cin >> password;

    if (checkUserExists(db, username)) {
        cout << "Username already exists! Please choose another." << endl;
        sqlite3_close(db);
        return;
    }

    sqlite3_stmt* stmt;
    string sql = "INSERT INTO Users (Username, Password) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "Error signing up: " << sqlite3_errmsg(db) << endl;
        } else {
            cout << "Sign up successful!" << endl;
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

bool logIn() {
    sqlite3* db = connectDatabase();
    if (!db) return false;

    string username, password;
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;

    sqlite3_stmt* stmt;
    string sql = "SELECT COUNT(*) FROM Users WHERE Username = ? AND Password = ?;";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0) {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return true;
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
    return false;
}

bool checkIfExistsInDatabase(sqlite3* db, const string& tableName, const string& columnName, int id) {
    string sql = "SELECT COUNT(*) FROM " + tableName + " WHERE " + columnName + " = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int count = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);
            return count > 0;
        }
        sqlite3_finalize(stmt);
    }
    return false;
}

void saveData() {
   if (head == NULL) {
        cout << "No employee data to save." << endl;
        return;
    }

    sqlite3* db = connectDatabase();
    if (!db) return;

char* errMsg = NULL;
    string sql =
        "CREATE TABLE IF NOT EXISTS Employees ("
        "ID INTEGER PRIMARY KEY, "
        "Name TEXT, "
        "Role TEXT, "
        "HourlyRate REAL, "
        "WorkHours REAL, "
        "OvertimeHours REAL, "
        "Bonuses REAL, "
        "TaxRate REAL, "
        "ProvidentFund REAL, "
        "BankAccountNumber TEXT, "
        "GrossSalary REAL, "
        "NetSalary REAL);";

    int rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return;
    }

    Employee* temp = head;
    while (temp) {
        if (!checkIfExistsInDatabase(db, "Employees", "ID", temp->id)) {
            sql = "INSERT INTO Employees (ID, Name, Role, HourlyRate, WorkHours, OvertimeHours, Bonuses, TaxRate, ProvidentFund, BankAccountNumber, GrossSalary, NetSalary) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
            sqlite3_stmt* stmt;

            if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, temp->id);
                sqlite3_bind_text(stmt, 2, temp->name.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 3, temp->role.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_double(stmt, 4, temp->hourlyRate);
                sqlite3_bind_double(stmt, 5, temp->workHours);
                sqlite3_bind_double(stmt, 6, temp->overtimeHours);
                sqlite3_bind_double(stmt, 7, temp->bonuses);
                sqlite3_bind_double(stmt, 8, temp->taxRate);
                sqlite3_bind_double(stmt, 9, temp->providentFund);
                sqlite3_bind_text(stmt, 10, temp->bankAccountNumber.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_double(stmt, 11, temp->grossSalary);
                sqlite3_bind_double(stmt, 12, temp->netSalary);

                if (sqlite3_step(stmt) != SQLITE_DONE) {
                    cerr << "Error inserting data: " << sqlite3_errmsg(db) << endl;
                }
                sqlite3_finalize(stmt);
            } else {
                cerr << "Failed to prepare insert statement: " << sqlite3_errmsg(db) << endl;
            }
        }
        temp = temp->next;
    }

    sqlite3_close(db);
    cout << "Data Saved Successfully!" << endl;
}

void loadData() {
    while (head != NULL) {
        Employee* temp = head;
        head = head->next;
        delete temp;
    }

    sqlite3* db = connectDatabase();
    if (!db) return;

    const char* selectSQL = "SELECT * FROM Employees;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, selectSQL, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            string role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            double hourlyRate = sqlite3_column_double(stmt, 3);
            double workHours = sqlite3_column_double(stmt, 4);
            double overtimeHours = sqlite3_column_double(stmt, 5);
            double bonuses = sqlite3_column_double(stmt, 6);
            double taxRate = sqlite3_column_double(stmt, 7);
            double providentFund = sqlite3_column_double(stmt, 8);
            string bankAccountNumber = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
            double grossSalary = sqlite3_column_double(stmt, 10);
            double netSalary = sqlite3_column_double(stmt, 11);


Employee* newEmployee = new Employee();
            newEmployee->id = id;
            newEmployee->name = name;
            newEmployee->role = role;
            newEmployee->hourlyRate = hourlyRate;
            newEmployee->workHours = workHours;
            newEmployee->overtimeHours = overtimeHours;
            newEmployee->bonuses = bonuses;
            newEmployee->taxRate = taxRate;
            newEmployee->providentFund = providentFund;
            newEmployee->bankAccountNumber = bankAccountNumber;
            newEmployee->grossSalary = grossSalary;
            newEmployee->netSalary = netSalary;
            newEmployee->next = head;
            head = newEmployee;
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void displayEmployees() {
    Employee* temp = head;
    while (temp) {
        cout << "ID: " << temp->id << ", Name: " << temp->name << ", Role: " << temp->role
             << ", Salary: $" << fixed << setprecision(2) << temp->netSalary << endl;
        temp = temp->next;
    }
}

bool isValidString(const string& str) {
    if (str.empty()) return false;
    for (size_t i = 0; i < str.size(); ++i) {
        char c = str[i];
        if (!isalpha(c) && c != ' ') {
            return false;
        }
    }
    return true;
}

double getValidDouble(const string& prompt) {
    double value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (cin.fail()) {
            cout << "Invalid input. Please enter a valid number." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
    }
}

int getValidInt(const string& prompt) {
    int value;
    while(true) {
        cout << prompt;
        cin >> value;
        if (cin.fail()) {
            cout << "Invalid input. Please enter a valid integer." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
    }
}

void addEmployee(int id, const string& name, const string& role, double hourlyRate, double workHours,
                 double overtimeHours, double bonuses, double taxRate, double providentFund,
                 const string& bankAccountNumber) {

    if(id <= 0) {
        cout << "Error: ID must be greater than 0!" << endl;
        return;
    }

    Employee* temp = head;
    while(temp) {
        if(temp->id == id) {
            cout << "Error: Employee ID already exists in local records!" << endl;
            return;
        }
        temp = temp->next;
    }

    sqlite3* db = connectDatabase();
    if(db) {
        bool existsInDB = checkIfExistsInDatabase(db, "Employees", "ID", id);
        sqlite3_close(db);

        if(existsInDB) {
            cout << "Error: Employee ID already exists in database!" << endl;
            return;
        }
    }

    Employee* newEmployee = new Employee;
    newEmployee->id = id;
    newEmployee->name = name;
    newEmployee->role = role;
    newEmployee->hourlyRate = hourlyRate;
    newEmployee->workHours = workHours;
    newEmployee->overtimeHours = overtimeHours;
    newEmployee->bonuses = bonuses;
    newEmployee->taxRate = taxRate;
    newEmployee->providentFund = providentFund;
    newEmployee->bankAccountNumber = bankAccountNumber;

    newEmployee->grossSalary = (hourlyRate * workHours) +
                              (overtimeHours * hourlyRate * 1.5) +
                              bonuses;
    newEmployee->netSalary = newEmployee->grossSalary -
                            (newEmployee->grossSalary * taxRate / 100) -
                            providentFund;
    newEmployee->next = head;
    head = newEmployee;

    cout << "Employee added successfully!" << endl;
}
void removeEmployee() {
    int employeeID;
    cout << "Enter Employee ID to remove: ";
    cin >> employeeID;

    Employee* temp = head;
    Employee* prev = NULL;


while (temp != NULL) {
        if (temp->id == employeeID) {
            if (prev == NULL) {
                head = temp->next;
            } else {
                prev->next = temp->next;
            }

            sqlite3* db = connectDatabase();
            if (db) {
                string sql = "DELETE FROM Employees WHERE ID = ?;";
                sqlite3_stmt* stmt;

                if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
                    sqlite3_bind_int(stmt, 1, employeeID);
                    if (sqlite3_step(stmt) != SQLITE_DONE) {
                        cerr << "Error deleting data: " << sqlite3_errmsg(db) << endl;
                    }
                    sqlite3_finalize(stmt);
                } else {
                    cerr << "Failed to prepare delete statement: " << sqlite3_errmsg(db) << endl;
                }
                sqlite3_close(db);
            }

            delete temp;
            cout << "Employee with ID " << employeeID << " removed successfully!" << endl;
            return;
        }
        prev = temp;
        temp = temp->next;
    }

    cout << "Employee with ID " << employeeID << " not found." << endl;
}

void updateEmployee() {
    int employeeID;
    cout << "Enter Employee ID to update: ";
    cin >> employeeID;

    Employee* temp = head;
    while (temp != NULL) {
        if (temp->id == employeeID) {
            cout << "Employee found! ID: " << employeeID << "\n";
            cout << "Choose the part you want to update:\n";
            cout << "1: Update Name\n";
            cout << "2: Update Role\n";
            cout << "3: Update Hourly Rate\n";
            cout << "4: Update Work Hours\n";
            cout << "5: Update Overtime Hours\n";
            cout << "6: Update Bonuses\n";
            cout << "7: Update Tax Rate\n";
            cout << "8: Update Provident Fund\n";
            cout << "9: Update Bank Account Number\n";
            cout << "Select an option: ";
            int option;
            cin >> option;
            cin.ignore();

            string columnToUpdate;
            string newValue;
            double newNumericValue;


switch (option) {
                case 1:
                    cout << "Enter the new name: ";
                    getline(cin, newValue);
                    if (!isValidString(newValue)) {
                        cout << "Invalid employee name. Update failed." << endl;
                        return;
                    }
                    temp->name = newValue;
                    columnToUpdate = "Name";
                    break;
                case 2:
                    cout << "Enter the new role: ";
                    getline(cin, newValue);
                    if (!isValidString(newValue)) {
                        cout << "Invalid employee role. Update failed." << endl;
                        return;
                    }
                    temp->role = newValue;
                    columnToUpdate = "Role";
                    break;
                case 3:
                    newNumericValue = getValidDouble("Enter the new hourly rate: ");
                    temp->hourlyRate = newNumericValue;
                    columnToUpdate = "HourlyRate";
                    break;
                case 4:
                    newNumericValue = getValidDouble("Enter the new work hours: ");
                    temp->workHours = newNumericValue;
                    columnToUpdate = "WorkHours";
                    break;
                case 5:
                     newNumericValue = getValidDouble("Enter the new overtime hours: ");
                    temp->overtimeHours = newNumericValue;
                    columnToUpdate = "OvertimeHours";
                    break;
                case 6:
                    newNumericValue = getValidDouble("Enter the new bonuses: ");
                    temp->bonuses = newNumericValue;
                    columnToUpdate = "Bonuses";
                    break;
                case 7:
                    newNumericValue = getValidDouble("Enter the new tax rate: ");
                    temp->taxRate = newNumericValue;
                    columnToUpdate = "TaxRate";
                    break;
                case 8:
                   newNumericValue = getValidDouble("Enter the new provident fund: ");
                    temp->providentFund = newNumericValue;
                    columnToUpdate = "ProvidentFund";
                    break;
                case 9:
                    cout << "Enter the new bank account number: ";
                    getline(cin, newValue);
                    temp->bankAccountNumber = newValue;
                    columnToUpdate = "BankAccountNumber";
                    break;
                default:
                    cout << "Invalid selection!" << endl;
                    return;
            }

            temp->grossSalary = (temp->hourlyRate * temp->workHours) + (temp->overtimeHours * temp->hourlyRate * 1.5) + temp->bonuses;
            temp->netSalary = temp->grossSalary - (temp->grossSalary * temp->taxRate / 100) - temp->providentFund;

            sqlite3* db = connectDatabase();
            if (db) {
                string sql = "UPDATE Employees SET " + columnToUpdate + " = ? WHERE ID = ?;";
                sqlite3_stmt* stmt;

                if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
                    if (option ==1 || option == 2 || option == 9) {
                        sqlite3_bind_text(stmt, 1, newValue.c_str(), -1, SQLITE_STATIC);
                    } else {
                        sqlite3_bind_double(stmt, 1, newNumericValue);
                    }
                    sqlite3_bind_int(stmt, 2, employeeID);


if (sqlite3_step(stmt) != SQLITE_DONE) {
                        cerr << "Error updating data: " << sqlite3_errmsg(db) << endl;
                    } else {
                        cout << "Employee details updated successfully in the database!" << endl;
                    }
                    sqlite3_finalize(stmt);
                } else {
                    cerr << "Failed to prepare update statement: " << sqlite3_errmsg(db) << endl;
                }
                sqlite3_close(db);
            }

            cout << "Employee details updated successfully!" << endl;
            return;
        }
        temp = temp->next;
    }
    cout << "Employee with ID " << employeeID << " not found." << endl;
}

void searchEmployee() {
    int employeeID;
    cout << "Enter Employee ID to search: ";
    cin >> employeeID;

    Employee* temp = head;
    while (temp != NULL) {
        if (temp->id == employeeID) {
            cout << "\nEmployee found!\n";
            cout << "ID: " << temp->id << "\n";
            cout << "Name: " << temp->name << "\n";
            cout << "Role: " << temp->role << "\n";
            cout << "Gross Salary: $" << fixed << setprecision(2) << temp->grossSalary << "\n";
            cout << "Net Salary: $" << fixed << setprecision(2) << temp->netSalary << "\n";
            cout << "Bank Account Number: " << temp->bankAccountNumber << "\n";
            return;
        }
        temp = temp->next;
    }

    cout << "Employee with ID " << employeeID << " not found." << endl;
}

void generatePayslip() {
    int employeeID;
    cout << "Enter Employee ID to generate payslip: ";
    cin >> employeeID;

    Employee* temp = head;
    while (temp != NULL) {
        if (temp->id == employeeID) {
            cout << "\n----- Payslip -----\n";
            cout << "Employee ID: " << temp->id << "\n";
            cout << "Name: " << temp->name << "\n";
            cout << "Role: " << temp->role << "\n";
            cout << "Gross Salary: $" << fixed << setprecision(2) << temp->grossSalary << "\n";
            cout << "Net Salary: $" << fixed << setprecision(2) << temp->netSalary << "\n";
            cout << "Bonuses: $" << temp->bonuses << "\n";
            cout << "Tax Rate: " << temp->taxRate << "%\n";
            cout << "Provident Fund: $" << temp->providentFund << "\n";
            cout << "Bank Account Number: " << temp->bankAccountNumber << "\n";
            cout << "-------------------\n";
            return;
        }
        temp = temp->next;
    }

    cout << "Employee with ID " << employeeID << " not found." << endl;
}

void clearEmployees() {
    while (head != NULL) {
        Employee* temp = head;
        head = head->next;
        delete temp;
    }
}

int main() {
    sqlite3* db = connectDatabase();
    if (!db) {
        return 1;
    }

    char* errMsg = NULL;
    string sql =
        "CREATE TABLE IF NOT EXISTS Users ("
        "Username TEXT PRIMARY KEY, "
        "Password TEXT);";
    int rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return 1;
    }
    sqlite3_close(db);

    bool isAuthenticated = false;

    while (!isAuthenticated) {
        cout << "Welcome to the Payroll System\n";
        cout << "1. Sign Up\n";
        cout << "2. Log In\n";
        cout << "Choose an option: ";
        int choice;
        cin >> choice;

        if (choice == 1) {
            signUp();
        } else if (choice == 2) {
            isAuthenticated = logIn();
            if (!isAuthenticated) {
                cout << "Invalid username or password. Please try again." << endl;
            } else {
                cout << "Login successful!" << endl;
            }
        } else {
            cout << "Invalid selection! Please choose again." << endl;
        }
    }

    loadData();
    bool running = true;


while (running) {
            cout << "\nPayroll Management System\n";
            cout << "1. Add Employee\n";
            cout << "2. Display Employees\n";
            cout << "3. Save Data\n";
            cout << "4. Load Data\n";
            cout << "5. Update Employee by ID\n";
            cout << "6. Search Employee by ID\n";
            cout << "7. Generate Payslip\n";
            cout << "8. Remove Employee by ID\n";
            cout << "0. Exit\n";
            cout << "Choose an option: ";

            int choice;
            cin >> choice;
            cin.ignore();

            switch (choice) {
            case 1: {
                int id;
                string name, role, bankAccountNumber;
                double hourlyRate, workHours, overtimeHours, bonuses, taxRate, providentFund;

                id = getValidInt("Enter Employee ID: ");
                cout << "Enter Name: ";
                getline(cin, name);
                cout << "Enter Role: ";
                getline(cin, role);

                hourlyRate = getValidDouble("Enter Hourly Rate: ");
                workHours = getValidDouble("Enter Work Hours: ");
                overtimeHours = getValidDouble("Enter Overtime Hours: ");
                bonuses = getValidDouble("Enter Bonuses: ");
                taxRate = getValidDouble("Enter Tax Rate (%): ");
                providentFund = getValidDouble("Enter Provident Fund: ");

                cout << "Enter Bank Account Number: ";
                getline(cin, bankAccountNumber);

                addEmployee(id, name, role, hourlyRate, workHours, overtimeHours, bonuses, taxRate, providentFund, bankAccountNumber);
                break;
            }

                case 2:
                    displayEmployees();
                    break;
                case 3:
                    saveData();
                   break;
                case 4:
                    loadData();
                    cout << "Data loaded successfully!" << endl;
                    break;
                case 5:
                    updateEmployee();
                    break;
                case 6:
                    searchEmployee();
                    break;
                case 7:
                    generatePayslip();
                    break;
                case 8:
                    removeEmployee();
                    break;
                case 0:
                    running = false;
                    clearEmployees();
                    break;
                default:
                    cout << "Invalid choice! Please try again." << endl;
            }
        }

    return 0;
}
