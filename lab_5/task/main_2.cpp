#include <windows.h>
#include <string>
#include <fstream>
#include <iostream>

HANDLE FileMutex;

int ReadFromFile() {
    WaitForSingleObject(FileMutex, INFINITE);
    std::fstream myfile("balance.txt", std::ios_base::in);
    int result = 0;
    myfile >> result;
    myfile.close();
    ReleaseMutex(FileMutex);
    return result;
}

void WriteToFile(int data) {
    WaitForSingleObject(FileMutex, INFINITE);
    std::fstream myfile("balance.txt", std::ios_base::out);
    myfile << data << std::endl;
    myfile.close();
    ReleaseMutex(FileMutex);
}

int GetBalance() {
    return ReadFromFile();
}

void Deposit(int money) {
    int balance = GetBalance();
    balance += money;

    WriteToFile(balance);
    printf("Balance after deposit: %d\n", balance);
}

void Withdraw(int money) {
    int balance = GetBalance();
    Sleep(20);
    if (balance < money) {
        printf("Cannot withdraw money, balance lower than %d\n", money);
    }
    else {
        balance -= money;
        WriteToFile(balance);
        printf("Balance after withdraw: %d\n", balance);
    }
}

DWORD WINAPI DoDeposit(CONST LPVOID lpParameter) {
    Deposit(static_cast<int>(reinterpret_cast<intptr_t>(lpParameter)));
    ExitThread(0);
}

DWORD WINAPI DoWithdraw(CONST LPVOID lpParameter) {
    Withdraw(static_cast<int>(reinterpret_cast<intptr_t>(lpParameter)));
    ExitThread(0);
}

int main() {
    HANDLE handles[50];

    FileMutex = CreateMutex(NULL, FALSE, reinterpret_cast<LPCSTR>(L"Global\\FileReadingMutex"));

    WriteToFile(0);

    for (int i = 0; i < 50; i++) {
        handles[i] = (i % 2 == 0)
            ? CreateThread(NULL, 0, &DoDeposit, reinterpret_cast<LPVOID>(230), CREATE_SUSPENDED, NULL)
            : CreateThread(NULL, 0, &DoWithdraw, reinterpret_cast<LPVOID>(1000), CREATE_SUSPENDED, NULL);
        ResumeThread(handles[i]);
    }

    WaitForMultipleObjects(50, handles, TRUE, INFINITE);
    printf("Final Balance: %d\n", GetBalance());

    for (const auto& handle : handles) {
        CloseHandle(handle);
    }

    CloseHandle(FileMutex);

    char some;
    std::cin >> some;
    return 0;
}