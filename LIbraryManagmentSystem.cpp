#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <iomanip>
#include <string>
#include <sstream>

using namespace std;

string getCurrentDate() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    string date = to_string(1900 + ltm->tm_year) + "-" + to_string(1 + ltm->tm_mon) + "-" + to_string(ltm->tm_mday);
    return date;
}

// Utility function to calculate the difference in days between two dates (YYYY-MM-DD)
int calculateDaysLate(const string &borrowDate, const string &returnDate) {
    tm borrow = {};
    tm ret = {};
    istringstream borrowStream(borrowDate);
    istringstream returnStream(returnDate);

    borrowStream >> get_time(&borrow, "%Y-%m-%d");
    returnStream >> get_time(&ret, "%Y-%m-%d");

    time_t borrowTime = mktime(&borrow);
    time_t returnTime = mktime(&ret);

    double difference = difftime(returnTime, borrowTime);
    int daysLate = difference / (60 * 60 * 24);  
    return daysLate;
}


class Book {
public:
    int id;
    string title;
    string author;
    bool available;

    Book(int _id, string _title, string _author, bool _available = true)
        : id(_id), title(_title), author(_author), available(_available) {}

    void displayBook() const {
        cout << setw(5) << id << setw(30) << title << setw(30) << author
             << setw(15) << (available ? "Yes" : "No") << endl;
    }
};

class User {
public:
    string name;
    virtual void searchBook(vector<Book>& books) = 0;  
};


class Admin : public User {
private:
    string password = "admin123"; // Password for admin
public:
    bool authenticateAdmin() {
        string inputPassword;
        cout << "Enter Admin Password: ";
        cin >> inputPassword;
        return inputPassword == password;
    }

    void addBook(vector<Book> &books) {
        int id;
        string title, author;
        cout << "Enter book ID: ";
        cin >> id;
        cout << "Enter book title: ";
        cin.ignore();
        getline(cin, title);
        cout << "Enter book author: ";
        getline(cin, author);
        books.push_back(Book(id, title, author));
        saveBooks(books);
        cout << "Book added successfully.\n";
    }

    void removeBook(vector<Book> &books) {
        int id;
        cout << "Enter book ID to remove: ";
        cin >> id;
        for (auto it = books.begin(); it != books.end(); ++it) {
            if (it->id == id) {
                logRemovedBook(*it);
                books.erase(it);
                saveBooks(books);
                cout << "Book removed successfully.\n";
                return;
            }
        }
        cout << "Book not found.\n";
    }

    void viewAllBooks(const vector<Book> &books) const {
        cout << setw(5) << "ID" << setw(30) << "Title" << setw(30) << "Author" << setw(15) << "Available" << endl;
        cout << "-------------------------------------------------------------------------------------------\n";
        for (const auto &book : books) {
            book.displayBook();
        }
    }

    void viewAvailableBooks(const vector<Book> &books) const {
        cout << setw(5) << "ID" << setw(30) << "Title" << setw(30) << "Author" << setw(15) << "Available" << endl;
        cout << "-------------------------------------------------------------------------------------------\n";
        for (const auto &book : books) {
            if (book.available) {
                book.displayBook();
            }
        }
    }

    void searchBook(vector<Book> &books) override {
        string title;
        cin.ignore();  
        cout << "Enter book title to search: ";
        getline(cin, title);
        for (const auto &book : books) {
            if (book.title == title) {
                book.displayBook();
                return;
            }
        }
        cout << "Book not found.\n";
    }

private:
    void saveBooks(const vector<Book> &books) {
        ofstream file("books.txt");
        file << setw(5) << "ID" << setw(30) << "Title" << setw(30) << "Author" << setw(15) << "Available" << endl;
        file << "-------------------------------------------------------------------------------------------\n";
        for (const auto &book : books) {
            file << setw(5) << book.id << setw(30) << book.title << setw(30) << book.author << setw(15) << (book.available ? "Yes" : "No") << endl;
        }
        file.close();
    }

    void logRemovedBook(const Book &book) {
        ofstream file("removed.txt", ios::app);
        file << setw(5) << "ID" << setw(30) << "Title" << setw(30) << "Author" << setw(20) << "Date Removed" << endl;
        file << "--------------------------------------------------------------------------------------------\n";
        file << setw(5) << book.id << setw(30) << book.title << setw(30) << book.author << setw(20) << getCurrentDate() << endl;
        file.close();
    }
};


class Member : public User {
public:
    void borrowBook(vector<Book> &books) {
        int id;
        cout << "Enter book ID to borrow: ";
        cin >> id;

        bool found = false;
        for (auto &book : books) {
            if (book.id == id) {
                found = true;
                if (book.available) {
                    book.available = false;
                    logBorrowedBook(book, name);  
                    cout << "Book borrowed successfully.\n";
                    saveBooks(books); // Update the file after borrowing
                    return;
                } else {
                    cout << "Book is currently not available.\n";
                    return;
                }
            }
        }

        if (!found) {
            cout << "Book does not exist.\n";
        }
    }

    void returnBook(vector<Book> &books) {
        int id;
        cout << "Enter book ID to return: ";
        cin >> id;

        bool found = false;
        for (auto &book : books) {
            if (book.id == id) {
                found = true;
                if (!book.available) {
                    string borrowDate;
                    cout << "Enter the date you borrowed the book (YYYY-MM-DD): ";
                    cin >> borrowDate;

                    string currentDate = getCurrentDate();
                    int daysLate = calculateDaysLate(borrowDate, currentDate);

                    int allowedDays = 14;  
                    
                    if (daysLate > allowedDays) {
                        int fine = (daysLate - allowedDays) * 5;  // Fine of $5 per day late
                        cout << "You are " << (daysLate - allowedDays) << " days late. Fine: $" << fine << endl;
                    } else {
                        cout << "No fine. Book returned on time.\n";
                    }

                    book.available = true;
                    logReturnedBook(book, name, borrowDate);  
                    cout << "Book returned successfully.\n";
                    saveBooks(books);  
                    return;
                } else {
                    cout << "Book was not borrowed.\n";
                    return;
                }
            }
        }

        if (!found) {
            cout << "Book not found.\n";
        }
    }

    void searchBook(vector<Book> &books) override {
        string title;
        cin.ignore();  
        cout << "Enter book title to search: ";
        getline(cin, title);
        for (const auto &book : books) {
            if (book.title == title) {
                book.displayBook();
                return;
            }
        }
        cout << "Book not found.\n";
    }

private:

    void logBorrowedBook(const Book &book, const string &memberName) {
        ofstream file("borrowedBooks.txt", ios::app);
        file << setw(5) << "ID" << setw(30) << "Title" << setw(30) << "Author" << setw(20) << "Date Borrowed" << setw(30) << "Borrowed By" << endl;
        file << "-----------------------------------------------------------------------------------------------------\n";
        file << setw(5) << book.id << setw(30) << book.title << setw(30) << book.author << setw(20) << getCurrentDate() << setw(30) << memberName << endl;
        file.close();
    }

    void logReturnedBook(const Book &book, const string &memberName, const string &borrowDate) {
        ofstream file("returnedBooks.txt", ios::app);
        file << setw(5) << "ID" << setw(30) << "Title" << setw(30) << "Author" << setw(20) << "Borrow Date" << setw(20) << "Return Date" << setw(30) << "Returned By" << endl;
        file << "-------------------------------------------------------------------------------------------------------------------\n";
        file << setw(5) << book.id << setw(30) << book.title << setw(30) << book.author << setw(20) << borrowDate << setw(20) << getCurrentDate() << setw(30) << memberName << endl;
        file.close();
    }

    void saveBooks(const vector<Book> &books) {
        ofstream file("books.txt");
        file << setw(5) << "ID" << setw(30) << "Title" << setw(30) << "Author" << setw(15) << "Available" << endl;
        file << "-------------------------------------------------------------------------------------------\n";
        for (const auto &book : books) {
            file << setw(5) << book.id << setw(30) << book.title << setw(30) << book.author << setw(15) << (book.available ? "Yes" : "No") << endl;
        }
        file.close();
    }
};

int main() {
    vector<Book> books = {
        Book(1, "The Great Gatsby", "F. Scott Fitzgerald"),
        Book(2, "To Kill a Mockingbird", "Harper Lee"),
        Book(3, "1984", "George Orwell"),
        Book(4, "Pride and Prejudice", "Jane Austen"),
        Book(5, "Moby Dick", "Herman Melville"),
        Book(6, "The Catcher in the Rye", "J.D. Salinger"),
        Book(7, "The Lord of the Rings", "J.R.R. Tolkien"),
        Book(8, "The Hobbit", "J.R.R. Tolkien"),
        Book(9, "War and Peace", "Leo Tolstoy"),
        Book(10, "Ulysses", "James Joyce")
    };

    while (true) {
        cout << "\n Welcome to the Library \n";
        cout << "1. Admin\n";
        cout << "2. Member\n";
        cout << "3. Exit\n";
        cout << "Choose your role: ";
        int choice;
        cin >> choice;

        if (choice == 1) {
            Admin admin;
            if (admin.authenticateAdmin()) {
                int adminChoice;
                do {
                    cout << "\n Admin Menu \n";
                    cout << "1. Add Book\n";
                    cout << "2. Remove Book\n";
                    cout << "3. View All Books\n";
                    cout << "4. View Available Books\n";
                    cout << "5. Search for a Book\n";
                    cout << "6. Logout\n";
                    cout << "Enter your choice: ";
                    cin >> adminChoice;

                    switch (adminChoice) {
                        case 1:
                            admin.addBook(books);
                            break;
                        case 2:
                            admin.removeBook(books);
                            break;
                        case 3:
                            admin.viewAllBooks(books);
                            break;
                        case 4:
                            admin.viewAvailableBooks(books);
                            break;
                        case 5:
                            admin.searchBook(books);
                            break;
                        case 6:
                            cout << "Logging out...\n";
                            break;
                        default:
                            cout << "Invalid choice. Try again.\n";
                    }
                } while (adminChoice != 6);
            } else {
                cout << "Invalid password. Returning to main menu...\n";
            }
        } else if (choice == 2) {
            Member member;
            cout << "Enter your name: ";
            cin >> member.name;

            int memberChoice;
            do {
                cout << "\n Member Menu \n";
                cout << "1. Borrow Book\n";
                cout << "2. Return Book\n";
                cout << "3. Search for a Book\n";
                cout << "4. Logout\n";
                cout << "Enter your choice: ";
                cin >> memberChoice;

                switch (memberChoice) {
                    case 1:
                        member.borrowBook(books);
                        break;
                    case 2:
                        member.returnBook(books);
                        break;
                    case 3:
                        member.searchBook(books);
                        break;
                    case 4:
                        cout << "Logging out...\n";
                        break;
                    default:
                        cout << "Invalid choice. Try again.\n";
                }
            } while (memberChoice != 4);
        } else if (choice == 3) {
            cout << "Exiting the program...\n";
            break;
        } else {
            cout << "Invalid choice. Try again.\n";
        }
    }

    return 0;
}
