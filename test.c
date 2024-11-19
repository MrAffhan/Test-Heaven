#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

#define MAX_USERS 100
#define MAX_MENU_ITEMS 100
#define MAX_ORDERS 100
#define ORDER_FILE "orders.txt" 
#define MAX_ITEMS 100
#define MAX_NAME_LENGTH 50
#define CONSOLE_WIDTH 80

#define ANSI_RED "\x1b[31m"
#define ANSI_GREEN "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_BLUE "\x1b[34m"
#define ANSI_MEGENTA "\x1b[35m"
#define ANSI_CYAN "\x1b[36m"
#define ANSI_RESET "\x1b[0m" 

typedef enum {
    ADMIN,
    MANAGER,
    STAFF,
    CUSTOMER
} Role;

typedef struct {
    char userID[10];
    char email[50];
    char password[30];
    Role role;
} User;

typedef struct {
    int id;
    char name[MAX_NAME_LENGTH];
    float price;
    int stock;
} MenuItem;

typedef struct {
    int itemId;
    int quantity;
    int customerID;
    float totalBill;
    char paymentMethod[15];
} Order;

User users[MAX_USERS];
MenuItem menu[MAX_MENU_ITEMS];
Order orders[MAX_ORDERS];
int userCount = 0;
int menuSize = 0;
int orderCount = 0;

void getHiddenPassword(char* password);
void registerUser();
int loginUser();
void generateUserID(char* userID, Role role);
void displayMenu();
void addMenuItem();
void editMenu();
void placeOrder(int userId);
void editOrder(int userId);
void cancelOrder(int userId);
void processPayment(int orderId);
void viewData(Role role);
void storeOrderData(Order order);  

void printCentered(const char *text) {
    int textLength = strlen(text);
    int spaces = (CONSOLE_WIDTH - textLength) / 2;
    for (int i = 0; i < spaces; i++) {
        printf(" ");
    }
    printf("%s\n", text);
}

void setConsoleColor(int textColor, int backgroundColor) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, (backgroundColor << 4) | textColor);
}

int main() {

    setConsoleColor(0, 15);
    system("cls");  
    
    strcpy(menu[menuSize].name, "Burger");
    menu[menuSize].price = 5.99;
    menu[menuSize].stock = 50;
    menu[menuSize++].id = menuSize;

    strcpy(menu[menuSize].name, "Pizza");
    menu[menuSize].price = 8.99;
    menu[menuSize].stock = 30;
    menu[menuSize++].id = menuSize;

    strcpy(menu[menuSize].name, "Pasta");
    menu[menuSize].price = 6.49;
    menu[menuSize].stock = 40;
    menu[menuSize++].id = menuSize;

    int choice, loggedIn = -1;

    while (1) {
        const char *line = "---Test Heaven Restaurant Management System ---";
    printf("\n"); 
    setConsoleColor(5, 15);
    printCentered(line);
    printf("\n");
        if (loggedIn == -1) {
            setConsoleColor(9, 15);
            printf("1. Sign Up\n2. Login\n3. Exit\n"ANSI_GREEN "Choose an option: ");
            scanf("%d", &choice);

            switch (choice) {
                case 1:
                    registerUser();
                    break;
                case 2:
                    loggedIn = loginUser();
                    break;
                case 3:
                    printf("Thank you for using the system!\n");
                    exit(0);
                default:
                    printf("Invalid choice, try again.\n");
            }
        } else {
            Role role = users[loggedIn].role;
            if (role == ADMIN || role == MANAGER) {
                setConsoleColor(9, 15);
                printf("1. View Data\n2. Logout\n3. Add new Itme\nChoose an option: ");
            } else if (role == STAFF) {
                setConsoleColor(9, 15);
                printf("1. View Menu\n2. Edit Menu\n3. Process Payments\n4. Logout\nChoose an option: ");
            } else if (role == CUSTOMER) {
                setConsoleColor(9, 15);
                printf("1. View Menu\n2. Place Order\n3. Edit Order\n4. Cancel Order\n5. Make Payment\n6. Logout\nChoose an option: ");
            }
            scanf("%d", &choice);

            switch (choice) {
                case 1:
                    if (role == CUSTOMER) displayMenu();
                    else if (role == STAFF) displayMenu();
                    else viewData(role);
                    break;
                case 2:
                    if (role == CUSTOMER) placeOrder(loggedIn);
                    else if (role == STAFF) editMenu();
                    else if (role == ADMIN || role == MANAGER) {
                        printf("Logout successful!\n");
                        loggedIn = -1;
                    }
                    break;
                case 3:
                    if (role == CUSTOMER) editOrder(loggedIn);
                    else if (role == STAFF) processPayment(orderCount - 1);
                    break;
                case 4:
                    if (role == CUSTOMER) cancelOrder(loggedIn);
                    else if (role == STAFF) {
                        printf ("Logout successful!\n");
                        loggedIn = -1;
                    }
                    break;
                case 5:
                    if (role == CUSTOMER) processPayment(orderCount - 1);
                    break;
                case 6: 
                    if (role == CUSTOMER){
                        printf("Logout successful!");
                        loggedIn = -1;
                    }
                    break;
                default:
                    printf("Invalid choice, try again.\n");
            }
        }
    }
    return 0;
}

void getHiddenPassword(char* password) {
    char ch;
    int i = 0;

#ifdef _WIN32
    while ((ch = _getch()) != '\r') {
        if (ch == '\b' && i > 0) { printf("\b \b"); i--; }
        else if (ch != '\b') { password[i++] = ch; printf("*"); }
    }
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while ((ch = getchar()) != '\n') {
        if (ch == '\b' && i > 0) { printf("\b \b"); i--; }
        else if (ch != '\b') { password[i++] = ch; printf("*"); }
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    password[i] = '\0';
}

void registerUser() {
    User newUser;
    char email[50], password[30], confirmPassword[30];
    int roleChoice;

    printf("Enter Email: ");
    scanf("%s", email);
    printf("Choose Role: 0=Admin, 1=Manager, 2=Staff, 3=Customer: ");
    scanf("%d", &roleChoice);

    do {
        printf("Enter Password: ");
        getHiddenPassword(password);
        printf("\nConfirm Password: ");
        getHiddenPassword(confirmPassword);
        if (strcmp(password, confirmPassword) != 0) printf("\nPasswords do not match! Try again.\n");
    } while (strcmp(password, confirmPassword) != 0);

    strcpy(newUser.email, email);
    strcpy(newUser.password, password);
    newUser.role = (Role)roleChoice;
    generateUserID(newUser.userID, newUser.role);

    users[userCount++] = newUser;
    printf("\nRegistration successful! User ID: %s\n", newUser.userID);
}

int loginUser() {
    char userID[10], password[30];

    printf("Enter User ID: ");
    scanf("%s", userID);
    printf("Enter Password: ");
    getHiddenPassword(password);

    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].userID, userID) == 0 && strcmp(users[i].password, password) == 0) {
            printf("\nLogin successful! Welcome %s.\n", userID);
            return i;
        }
    }
    printf("\nIncorrect User ID or Password.\n");
    return -1;
}

void generateUserID(char* userID, Role role) {
    const char* rolePrefix[] = {"ADM", "MGR", "STF", "CST"};
    sprintf(userID, "%s%d", rolePrefix[role], userCount + 1);
}

void displayMenu() {
    printf("\n---- Menu ----\n");
    for (int i = 0; i < menuSize; i++) {
        printf("%d. %s - $%.2f (Stock: %d)\n", menu[i].id, menu[i].name, menu[i].price, menu[i].stock);
    }
}

void addMenuItem() {
    if (menuSize < MAX_MENU_ITEMS) {
        printf("Enter Item Name: ");
        scanf("%s", menu[menuSize].name);
        printf("Enter Price: ");
        scanf("%f", &menu[menuSize].price);
        printf("Enter Stock: ");
        scanf("%d", &menu[menuSize].stock);
        menu[menuSize].id = menuSize + 1;
        menuSize++;
        printf("Item added successfully!\n");
    } else {
        printf("Menu capacity reached!\n");
    }
}

void editMenu() {
    int itemId;
    displayMenu();
    printf("Enter the Item ID to edit: ");
    scanf("%d", &itemId);

    if (itemId > 0 && itemId <= menuSize) {
        printf("Enter New Name: ");
        scanf("%s", menu[itemId - 1].name);
        printf("Enter New Price: ");
        scanf("%f", &menu[itemId - 1].price);
        printf("Enter New Stock: ");
        scanf("%d", &menu[itemId - 1].stock);
        printf("Item updated successfully!\n");
    } else {
        printf("Invalid Item ID!\n");
    }
}

void placeOrder(int userId) {
    int itemId, quantity;
    float totalBill = 0;

    displayMenu();
    printf("Enter Item ID to order: ");
    scanf("%d", &itemId);
    printf("Enter Quantity: ");
    scanf("%d", &quantity);

    if (itemId > 0 && itemId <= menuSize && quantity <= menu[itemId - 1].stock) {
        orders[orderCount].itemId = itemId;
        orders[orderCount].quantity = quantity;
        orders[orderCount].customerID = userId;
        orders[orderCount].totalBill = quantity * menu[itemId - 1].price;
        menu[itemId - 1].stock -= quantity; 

        storeOrderData(orders[orderCount]);
        
        printf("Order placed for %s (x%d). Total Bill: $%.2f\n", menu[itemId - 1].name, quantity, orders[orderCount].totalBill);
        orderCount++;
    } else {
        printf("Invalid item ID or insufficient stock.\n");
    }
}

void storeOrderData(Order order) {
    FILE *file = fopen(ORDER_FILE, "a");
    if (file) {
        fprintf(file, "Order ID: %d, Item ID: %d, Quantity: %d, Total Bill: $%.2f, Payment Method: %s\n",
                orderCount + 1, order.itemId, order.quantity, order.totalBill, order.paymentMethod);
        fclose(file);
    } else {
        printf("Error opening file to store order data.\n");
    }
}

void processPayment(int orderId) {
    int paymentMethod;

    if (orderId >= 0 && orderId < orderCount) {
        printf("Select Payment Method:\n1. Mobile Banking\n2. Card\n3. Cash\nChoose option: ");
        scanf("%d", &paymentMethod);

        switch (paymentMethod) {
            case 1:
                strcpy(orders[orderId].paymentMethod, "Mobile Banking");
                printf("Payment successful via Mobile Banking. Total: $%2f\n", orders[orderId].totalBill);
                break;
            case 2:
                strcpy(orders[orderId].paymentMethod, "Card");
                printf("Payment successful via Card. Total: $%2f\n", orders[orderId].totalBill);
                break;
            case 3:
                strcpy(orders[orderId].paymentMethod, "Cash");
                printf("Please confirm cash payment has been received by staff.\n");

                char confirm;
                printf("Confirm payment received? (y/n); ");
                scanf("%c", &confirm);
                if (confirm == 'y' || confirm == 'Y') {
                    printf("Payment successful via Cash. Total: $%2f\n", orders[orderId].totalBill);
                } else {
                    printf("Payment not confirmed. Please check with staff.\n");
                }
                break;
            default:
                printf("Invalid Payment Method.\n");
                return;
        }

        printf("Payment successful via %s. Total: $%.2f\n", orders[orderId].paymentMethod, orders[orderId].totalBill);
    } else {
        printf("Invalid Order ID.\n");
    }
}

void editOrder(int userId) {
    printf("Edit order functionality is not implemented yet.\n");
}

void cancelOrder(int userId) {
    printf("Cancel order functionality is not implemented yet.\n");
}

void viewData(Role role) {
    printf("\n--- View Data (Accessible by Admin and Manager) ---\n");
    printf("Total Users: %d\n", userCount);
    printf("Total Orders: %d\n", orderCount);
}
