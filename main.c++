#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <stdexcept>

using namespace std;

class Object {
public:
    virtual vector<unsigned char> serialize() const = 0;
    virtual size_t deserialize(const vector<unsigned char>& data, size_t start) = 0;
    virtual ~Object() = default;
};

class Employee : public Object {
public:
    string name;
    float salary;

    vector<unsigned char> serialize() const override {
        vector<unsigned char> data;
        serializeString(data, name);
        serializeFloat(data, salary);
        return data;
    }

    size_t deserialize(const vector<unsigned char>& data, size_t start) override {
        start = deserializeString(data, start, name);
        start = deserializeFloat(data, start, salary);
        return start;
    }

private:
    void serializeString(vector<unsigned char>& data, const string& str) const {
        size_t size = str.size();
        data.insert(data.end(), reinterpret_cast<const unsigned char*>(&size), reinterpret_cast<const unsigned char*>(&size) + sizeof(size_t));
        data.insert(data.end(), str.begin(), str.end());
    }

    size_t deserializeString(const vector<unsigned char>& data, size_t start, string& str) {
        size_t size;
        memcpy(&size, data.data() + start, sizeof(size_t));
        start += sizeof(size_t);
        str.resize(size);
        memcpy(&str[0], data.data() + start, size);
        start += size;
        return start;
    }

    void serializeFloat(vector<unsigned char>& data, float value) const {
        unsigned char bytes[sizeof(float)];
        memcpy(bytes, &value, sizeof(float));
        data.insert(data.end(), bytes, bytes + sizeof(float));
    }

    size_t deserializeFloat(const vector<unsigned char>& data, size_t start, float& value) {
        memcpy(&value, data.data() + start, sizeof(float));
        start += sizeof(float);
        return start;
    }
};

class Customer : public Object {
public:
    string name;
    string address;

    vector<unsigned char> serialize() const override {
        vector<unsigned char> data;
        serializeString(data, name);
        serializeString(data, address);
        return data;
    }

    size_t deserialize(const vector<unsigned char>& data, size_t start) override {
        start = deserializeString(data, start, name);
        start = deserializeString(data, start, address);
        return start;
    }

private:
    void serializeString(vector<unsigned char>& data, const string& str) const {
        size_t size = str.size();
        data.insert(data.end(), reinterpret_cast<const unsigned char*>(&size), reinterpret_cast<const unsigned char*>(&size) + sizeof(size_t));
        data.insert(data.end(), str.begin(), str.end());
    }

    size_t deserializeString(const vector<unsigned char>& data, size_t start, string& str) {
        size_t size;
        memcpy(&size, data.data() + start, sizeof(size_t));
        start += sizeof(size_t);
        str.resize(size);
        memcpy(&str[0], data.data() + start, size);
        start += size;
        return start;
    }
};

class Item : public Object {
public:
    string name;
    float amount;

    virtual unsigned char getType() const {
        return 0; // Type ID for Item
    }

    vector<unsigned char> serialize() const override {
        vector<unsigned char> data;
        data.push_back(getType());
        serializeString(data, name);
        serializeFloat(data, amount);
        return data;
    }

    size_t deserialize(const vector<unsigned char>& data, size_t start) override {
        start = deserializeString(data, start, name);
        start = deserializeFloat(data, start, amount);
        return start;
    }

protected:
    void serializeString(vector<unsigned char>& data, const string& str) const {
        size_t size = str.size();
        data.insert(data.end(), reinterpret_cast<const unsigned char*>(&size), reinterpret_cast<const unsigned char*>(&size) + sizeof(size_t));
        data.insert(data.end(), str.begin(), str.end());
    }

    size_t deserializeString(const vector<unsigned char>& data, size_t start, string& str) {
        size_t size;
        memcpy(&size, data.data() + start, sizeof(size_t));
        start += sizeof(size_t);
        str.resize(size);
        memcpy(&str[0], data.data() + start, size);
        start += size;
        return start;
    }

    void serializeFloat(vector<unsigned char>& data, float value) const {
        unsigned char bytes[sizeof(float)];
        memcpy(bytes, &value, sizeof(float));
        data.insert(data.end(), bytes, bytes + sizeof(float));
    }

    size_t deserializeFloat(const vector<unsigned char>& data, size_t start, float& value) {
        memcpy(&value, data.data() + start, sizeof(float));
        start += sizeof(float);
        return start;
    }
};

class FoodItem : public Item {
public:
    float foodCouponDiscount;

    unsigned char getType() const override {
        return 1; // Type ID for FoodItem
    }

    vector<unsigned char> serialize() const override {
        vector<unsigned char> data = Item::serialize();
        serializeFloat(data, foodCouponDiscount);
        return data;
    }

    size_t deserialize(const vector<unsigned char>& data, size_t start) override {
        start = Item::deserialize(data, start);
        start = deserializeFloat(data, start, foodCouponDiscount);
        return start;
    }
};

class Sale : public Object {
public:
    Customer* customer;
    Employee* salerep;
    vector<Item*> items;
    float totalAmount;
    float totalFoodCouponDiscount;
    long long date; // Assuming timestamp in milliseconds since epoch

    vector<unsigned char> serialize() const override {
        vector<unsigned char> data;
        auto customerData = customer->serialize();
        data.insert(data.end(), customerData.begin(), customerData.end());
        auto employeeData = salerep->serialize();
        data.insert(data.end(), employeeData.begin(), employeeData.end());

        size_t itemCount = items.size();
        data.insert(data.end(), reinterpret_cast<const unsigned char*>(&itemCount), reinterpret_cast<const unsigned char*>(&itemCount) + sizeof(size_t));
        for (auto item : items) {
            auto itemData = item->serialize();
            data.insert(data.end(), itemData.begin(), itemData.end());
        }

        serializeFloat(data, totalAmount);
        serializeFloat(data, totalFoodCouponDiscount);
        unsigned char bytes[sizeof(long long)];
        memcpy(bytes, &date, sizeof(long long));
        data.insert(data.end(), bytes, bytes + sizeof(long long));

        return data;
    }

    size_t deserialize(const vector<unsigned char>& data, size_t start) override {
        start = customer->deserialize(data, start);
        start = salerep->deserialize(data, start);

        size_t itemCount;
        memcpy(&itemCount, data.data() + start, sizeof(size_t));
        start += sizeof(size_t);
        items.resize(itemCount);
        for (size_t i = 0; i < itemCount; ++i) {
            unsigned char itemType = data[start++];
            Item* newItem = nullptr;
            if (itemType == 0) {
                newItem = new Item();
            }
            else if (itemType == 1) {
                newItem = new FoodItem();
            }
            items[i] = newItem;
            start = items[i]->deserialize(data, start);
        }

        start = deserializeFloat(data, start, totalAmount);
        start = deserializeFloat(data, start, totalFoodCouponDiscount);
        memcpy(&date, data.data() + start, sizeof(long long));
        start += sizeof(long long);

        return start;
    }

private:
    void serializeFloat(vector<unsigned char>& data, float value) const {
        unsigned char bytes[sizeof(float)];
        memcpy(bytes, &value, sizeof(float));
        data.insert(data.end(), bytes, bytes + sizeof(float));
    }

    size_t deserializeFloat(const vector<unsigned char>& data, size_t start, float& value) {
        memcpy(&value, data.data() + start, sizeof(float));
        start += sizeof(float);
        return start;
    }
};

// Global lists for storing Employees, Customers, and Items
vector<Employee*> employees;
vector<Customer*> customers;
vector<Item*> items;

// Function to add an Employee
void addEmployee() {
    auto* employee = new Employee();
    cout << "Enter Employee Name: ";
    cin >> employee->name;
    cout << "Enter Employee Salary: ";
    cin >> employee->salary;
    employees.push_back(employee);
    cout << "Employee added successfully.\n";
}

// Function to add a Customer
void addCustomer() {
    auto* customer = new Customer();
    cout << "Enter Customer Name: ";
    cin >> customer->name;
    cout << "Enter Customer Address: ";
    cin >> customer->address;
    customers.push_back(customer);
    cout << "Customer added successfully.\n";
}

// Function to add an Item
void addItem() {
    cout << "Enter 0 for regular Item, 1 for FoodItem: ";
    int type;
    cin >> type;
    Item* item = nullptr;
    if (type == 0) {
        item = new Item();
    }
    else if (type == 1) {
        item = new FoodItem();
        cout << "Enter Food Coupon Discount: ";
        cin >> dynamic_cast<FoodItem*>(item)->foodCouponDiscount;
    }
    cout << "Enter Item Name: ";
    cin >> item->name;
    cout << "Enter Item Amount: ";
    cin >> item->amount;
    items.push_back(item);
    cout << "Item added successfully.\n";
}

// Function to save data to a file
void saveData(const string& filename) {
    ofstream out(filename, ios::binary);
    if (!out.is_open()) {
        throw runtime_error("Failed to open file for writing");
    }
    size_t employeeCount = employees.size();
    out.write(reinterpret_cast<const char*>(&employeeCount), sizeof(size_t));
    for (const auto& emp : employees) {
        vector<unsigned char> empData = emp->serialize();
        size_t empDataSize = empData.size();
        out.write(reinterpret_cast<const char*>(&empDataSize), sizeof(size_t));
        out.write(reinterpret_cast<const char*>(empData.data()), empDataSize);
    }

    size_t customerCount = customers.size();
    out.write(reinterpret_cast<const char*>(&customerCount), sizeof(size_t));
    for (const auto& cust : customers) {
        vector<unsigned char> custData = cust->serialize();
        size_t custDataSize = custData.size();
        out.write(reinterpret_cast<const char*>(&custDataSize), sizeof(size_t));
        out.write(reinterpret_cast<const char*>(custData.data()), custDataSize);
    }

    size_t itemCount = items.size();
    out.write(reinterpret_cast<const char*>(&itemCount), sizeof(size_t));
    for (const auto& itm : items) {
        vector<unsigned char> itemData = itm->serialize();
        size_t itemDataSize = itemData.size();
        out.write(reinterpret_cast<const char*>(&itemDataSize), sizeof(size_t));
        out.write(reinterpret_cast<const char*>(itemData.data()), itemDataSize);
    }
    out.close();
    cout << "Data saved successfully.\n";
}

// Function to load data from a file
void loadData(const string& filename) {
    ifstream in(filename, ios::binary);
    if (!in.is_open()) {
        throw runtime_error("Failed to open file for reading");
    }

    size_t employeeCount;
    in.read(reinterpret_cast<char*>(&employeeCount), sizeof(size_t));
    employees.resize(employeeCount);
    for (size_t i = 0; i < employeeCount; ++i) {
        size_t empDataSize;
        in.read(reinterpret_cast<char*>(&empDataSize), sizeof(size_t));
        vector<unsigned char> empData(empDataSize);
        in.read(reinterpret_cast<char*>(empData.data()), empDataSize);
        employees[i] = new Employee();
        employees[i]->deserialize(empData, 0);
    }

    size_t customerCount;
    in.read(reinterpret_cast<char*>(&customerCount), sizeof(size_t));
    customers.resize(customerCount);
    for (size_t i = 0; i < customerCount; ++i) {
        size_t custDataSize;
        in.read(reinterpret_cast<char*>(&custDataSize), sizeof(size_t));
        vector<unsigned char> custData(custDataSize);
        in.read(reinterpret_cast<char*>(custData.data()), custDataSize);
        customers[i] = new Customer();
        customers[i]->deserialize(custData, 0);
    }

    size_t itemCount;
    in.read(reinterpret_cast<char*>(&itemCount), sizeof(size_t));
    items.resize(itemCount);
    for (size_t i = 0; i < itemCount; ++i) {
        size_t itemDataSize;
        in.read(reinterpret_cast<char*>(&itemDataSize), sizeof(size_t));
        vector<unsigned char> itemData(itemDataSize);
        in.read(reinterpret_cast<char*>(itemData.data()), itemDataSize);
        unsigned char itemType = itemData[0];
        if (itemType == 0) {
            items[i] = new Item();
        }
        else if (itemType == 1) {
            items[i] = new FoodItem();
        }
        items[i]->deserialize(itemData, 1);
    }
    in.close();
    cout << "Data loaded successfully.\n";
}

// Main function
int main() {
    int choice;
    string filename;
    while (true) {
        cout << "1. Add Employee\n2. Add Customer\n3. Add Item\n4. Save Data\n5. Load Data\n6. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;
        switch (choice) {
        case 1:
            addEmployee();
            break;
        case 2:
            addCustomer();
            break;
        case 3:
            addItem();
            break;
        case 4:
            cout << "Enter filename to save data: ";
            cin >> filename;
            saveData(filename);
            break;
        case 5:
            cout << "Enter filename to load data: ";
            cin >> filename;
            loadData(filename);
            break;
        case 6:
            cout << "Exiting...\n";
            return 0;
        default:
            cout << "Invalid choice. Please try again.\n";
        }
    }
}
