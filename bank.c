#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME 100
#define MAX_PASSWORD 50
#define MAX_ACCOUNTS 100  // Maximum number of accounts allowed
#define FILENAME "accounts.dat"

// Account structure to store account details
typedef struct {
    int accountNumber;
    char name[MAX_NAME];
    char password[MAX_PASSWORD];
    float balance;
} Account;

// Function prototypes
void menu();
void createAccount();
Account* login();
void viewAccount();
void deposit();
void withdraw();
void transfer();
void transferToRecipient(int toAccNum, float amount, Account *fromAcc);
void transferFromSender(float amount, Account *fromAcc);
void viewTransactions();
void logTransaction(int accountNumber, const char *type, float amount);

int countAccounts();

int main() {
    menu();
    return 0;
}

// Function to display the main menu
void menu() {
    int choice;
    do {
        printf("\n--- Welcome to *_* Boyz Bank ---\n");
        printf("1. Create Account\n2. View Account\n3. Deposit\n4. Withdraw\n");
        printf("5. Transfer\n6. View Transactions\n7. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: createAccount(); break;
            case 2: viewAccount(); break;
            case 3: deposit(); break;
            case 4: withdraw(); break;
            case 5: transfer(); break;
            case 6: viewTransactions(); break;
            case 7: printf("Thanks for using our bank\nExiting...\n"); break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 7);
}

// Function to count the number of accounts in the file
int countAccounts() {
    FILE *file = fopen(FILENAME, "rb");
    if (!file) return 0; // No accounts if file doesn't exist
    fseek(file, 0, SEEK_END);  // Move to the end of the file
    int size = ftell(file);    // Get the file size in bytes
    fclose(file);
    return size / sizeof(Account);  // Return the number of accounts
}

// Function to create a new account
void createAccount() {
    Account acc;

    // Check if maximum account limit is reached
    if (countAccounts() >= MAX_ACCOUNTS) {
        printf("Maximum account limit reached.\n");
        return;
    }

    FILE *file = fopen(FILENAME, "ab");
    if (!file) {
        printf("Error opening file.\n");
        return;
    }

    // Generate a unique account number based on existing accounts
    int accountCount = countAccounts();
    acc.accountNumber = 1000 + accountCount;  // Example: start account numbers from 1000

    // Collect account details from the user
    printf("Account number (auto-generated): %d\n", acc.accountNumber);
    
    printf("Enter full name: ");
    getchar();  // Clear newline from previous input
    fgets(acc.name, MAX_NAME, stdin);
    acc.name[strcspn(acc.name, "\n")] = '\0';  // Remove trailing newline character

    printf("Enter password: ");
    scanf("%s", acc.password);

    // Ensure initial balance is a positive number
    do {
        printf("Enter initial balance: ");
        scanf("%f", &acc.balance);
        if (acc.balance < 0) {
            printf("Balance cannot be negative. Please enter a valid amount.\n");
        }
    } while (acc.balance < 0);

    // Write account details to the file
    fwrite(&acc, sizeof(Account), 1, file);
    fclose(file);
    
    printf("Account created successfuly.\n");

    // Log the creation transaction (optional for future development)
    logTransaction(acc.accountNumber, "Account Created", acc.balance);
}

// Function to login before any sensitive operation
Account* login() {
    static Account acc;
    int accNum;
    char pass[MAX_PASSWORD];
    FILE *file = fopen(FILENAME, "rb");

    if (!file) {
        printf("Error opening file.\n");
        return NULL;
    }

    // Get account number and password from the user
    printf("Enter account number: ");
    scanf("%d", &accNum);
    printf("Enter password: ");
    scanf("%s", pass);

    // Search for the account in the file
    while (fread(&acc, sizeof(Account), 1, file)) {
        if (acc.accountNumber == accNum && strcmp(acc.password, pass) == 0) {
            fclose(file);
            return &acc;
        }
    }
    printf("Invalid account number or password.\n");
    fclose(file);
    return NULL;
}

// Function to view account details
void viewAccount() {
    Account *acc = login();
    if (acc) {
        printf("Account Number: %d\nName: %s\nBalance: %.2f\n", 
               acc->accountNumber, acc->name, acc->balance);
    }
}

// Function to deposit money into an account
void deposit() {
    Account *acc = login();
    if (!acc) return;

    float amount;
    FILE *file = fopen(FILENAME, "r+b");
    if (!file) {
        printf("Error opening file.\n");
        return;
    }

    printf("Enter deposit amount: ");
    scanf("%f", &amount);

    acc->balance += amount;
    fseek(file, -sizeof(Account), SEEK_CUR);
    fwrite(acc, sizeof(Account), 1, file);
    fclose(file);
    printf("Deposit successful. New balance: %.2f\n", acc->balance);
    logTransaction(acc->accountNumber, "Deposit", amount);
}

// Function to withdraw money from an account
void withdraw() {
    Account *acc = login();
    if (!acc) return;

    float amount;
    FILE *file = fopen(FILENAME, "r+b");
    if (!file) {
        printf("Error opening file.\n");
        return;
    }

    printf("Enter withdrawal amount: ");
    scanf("%f", &amount);

    if (acc->balance >= amount) {
        acc->balance -= amount;
        fseek(file, -sizeof(Account), SEEK_CUR);
        fwrite(acc, sizeof(Account), 1, file);
        fclose(file);
        printf("Withdrawal successful. New balance: %.2f\n", acc->balance);
        logTransaction(acc->accountNumber, "Withdrawal", amount);
    } else {
        printf("Insufficient funds.\n");
        fclose(file);
    }
}

// Function to transfer money between accounts
void transfer() {
    Account *fromAcc = login();
    if (!fromAcc) return;

    int toAccNum;
    float amount;

    printf("Enter destination account number: ");
    scanf("%d", &toAccNum);
    printf("Enter transfer amount: ");
    scanf("%f", &amount);

    transferFromSender(amount, fromAcc);  // Handle sender's side of the transfer
    transferToRecipient(toAccNum, amount, fromAcc);  // Handle recipient's side
}

// Function to handle transfer from the sender's account
void transferFromSender(float amount, Account *fromAcc) {
    if (fromAcc->balance >= amount) {
        FILE *file = fopen(FILENAME, "r+b");
        if (!file) {
            printf("Error opening file.\n");
            return;
        }

        fromAcc->balance -= amount;
        fseek(file, -sizeof(Account), SEEK_CUR);
        fwrite(fromAcc, sizeof(Account), 1, file);
        fclose(file);

        logTransaction(fromAcc->accountNumber, "Transfer Out", amount);
        printf("Transfer from sender successful.\n");
    } else {
        printf("Insufficient funds in sender's account.\n");
    }
}

// Function to handle transfer to the recipient's account
void transferToRecipient(int toAccNum, float amount, Account *fromAcc) {
    FILE *file = fopen(FILENAME, "r+b");
    if (!file) {
        printf("Error opening file.\n");
        return;
    }

    Account toAcc;
    while (fread(&toAcc, sizeof(Account), 1, file)) {
        if (toAcc.accountNumber == toAccNum) {
            toAcc.balance += amount;

            fseek(file, -sizeof(Account), SEEK_CUR);
            fwrite(&toAcc, sizeof(Account), 1, file);
            fclose(file);

            logTransaction(toAcc.accountNumber, "Transfer In", amount);
            printf("Transfer to recipient successful.\n");
            return;
        }
    }
    printf("Destination account not found.\n");
    fclose(file);
}

// Function to view transactions for a specific account
void viewTransactions() {
    int accNum;
    char line[256];

    // Prompt user for account number
    printf("Enter account number: ");
    scanf("%d", &accNum);

    // Convert accNum to string for comparison
    char accNumStr[10];  
    sprintf(accNumStr, "%d", accNum);

    // Open the transaction log file
    FILE *file = fopen("transactions.log", "r");
    if (!file) {
        printf("No transactions found.\n");
        return;
    }

    // Print transactions for the given account
    printf("Transactions for account %d:\n", accNum);
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, accNumStr)) {  // Search for account number in each line
            printf("%s", line);
        }
    }
    fclose(file);
}

// Function to log transactions
void logTransaction(int accountNumber, const char *type, float amount) {
    FILE *file = fopen("transactions.log", "a");
    if (file) {
        time_t now = time(NULL);
        fprintf(file, "Account: %d | Type: %s | Amount: %.2f | Date: %s", 
                accountNumber, type, amount, ctime(&now));
        fclose(file);
    }
}
