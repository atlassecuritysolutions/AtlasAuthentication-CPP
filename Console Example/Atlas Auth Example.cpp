// Atlas SDK - console test (C++). Build Release / x64.
// The API key is read from Atlas.h (Atlas::API_KEY).
//
//   Dashboard: https://atlassecurity.site/dashboard
//   Docs:      https://atlassecurity.site/docs

#include <Windows.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include "Atlas.h"

namespace {

std::string Ask(const char* prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin >> std::ws, line)) std::exit(0);
    return line;
}

std::string AskOptional(const char* prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) std::exit(0);
    return line;
}

void PrintError() {
    std::cout << "[!] " << Atlas::Data::GetErrorMessage() << "\n";
}

bool SignInWithLicense() {
    if (Atlas::License::Login(Ask("License key: "))) return true;
    PrintError();
    return false;
}

bool SignInWithAccount() {
    const std::string username = Ask("Username: ");
    const std::string password = Ask("Password: ");
    auto r = Atlas::Account::Login(username, password);
    using S = Atlas::Account::LoginResult::Status;
    switch (r.status) {
    case S::Ok:
        return true;
    case S::NeedsVerification:
        std::cout << "We emailed an 8-digit code to " << r.masked_email
                  << " (from " << r.sign_in_ip;
        if (!r.sign_in_country.empty()) std::cout << " / " << r.sign_in_country;
        std::cout << ")\n";
        if (Atlas::Account::SubmitVerification(Ask("Code: "))) return true;
        PrintError();
        return false;
    case S::WrongCredentials:
        std::cout << "[!] Wrong username or password.\n";
        return false;
    case S::Banned:
        std::cout << "[!] This account is banned.\n";
        return false;
    case S::AccountPaused:
        std::cout << "[!] This account is paused.\n";
        return false;
    case S::ServerUnreachable:
        std::cout << "[!] Could not reach the server. Check the connection and try again.\n";
        return false;
    case S::Error:
        break;
    }
    PrintError();
    return false;
}

void RegisterAccount() {
    const std::string username = Ask("Pick a username: ");
    const std::string password = Ask("Pick a password: ");
    const std::string email = AskOptional("Email (optional, Enter to skip): ");
    if (!Atlas::Account::Register(username, password, email)) {
        PrintError();
        return;
    }
    if (Atlas::Account::HasPendingEmailConfirm()) {
        std::cout << "We emailed a confirmation code to " << email << ".\n";
        if (!Atlas::Account::ConfirmEmail(Ask("Code: "))) {
            PrintError();
            return;
        }
    }
    std::cout << "[+] Account '" << username << "' is ready. Sign in with [2].\n";
}

// Before sign-in: clears this machine's binding for an account after an emailed code. The password does not change.
void ResetWithEmailCode() {
    const std::string identifier = Ask("Username or email: ");
    if (!Atlas::Account::RequestResetCode(identifier)) {
        PrintError();
        return;
    }
    std::cout << "If the account has a verified email, an 8-digit code was sent.\n";
    if (!Atlas::Account::ConfirmReset(identifier, Ask("Code: "))) {
        PrintError();
        return;
    }
    std::cout << "[+] Reset done. Sign in on this machine again.\n";
}

// Before sign-in: forgot password. An 8-digit code goes to the account's email, then the new password is set.
void ForgotPassword() {
    const std::string identifier = Ask("Username or email: ");
    if (!Atlas::Account::RequestPasswordReset(identifier)) {
        PrintError();
        return;
    }
    std::cout << "If the account exists, a code was sent to its email. It expires in 30 minutes.\n";
    const std::string code = Ask("Code: ");
    const std::string password = Ask("New password: ");
    if (!Atlas::Account::CompletePasswordReset(code, password)) {
        PrintError();
        return;
    }
    std::cout << "[+] Password changed. Sign in with the new password.\n";
}

// Signed in: resets this machine's binding and ends the session. The library closes the program on success.
void ResetWhileSignedIn(bool is_account) {
    std::cout << "This clears this machine's binding and ends the session. A successful reset closes the program.\n";
    const std::string yn = Ask("Reset now? [y/N]: ");
    if (yn != "y" && yn != "Y") return;
    const bool ok = is_account ? Atlas::Account::ResetWhileSignedIn()
                               : Atlas::License::ResetWhileSignedIn();
    if (!ok) PrintError();
}

void PrintSession() {
    const bool is_account = !Atlas::Data::GetUsername().empty();
    std::cout << "\n--- Session ---\n";
    if (is_account) std::cout << "Username:  " << Atlas::Data::GetUsername() << "\n";
    else            std::cout << "License:   " << Atlas::Data::GetLicense() << "\n";
    std::cout << "Expiry:    " << Atlas::Data::GetExpiry()
              << " (" << Atlas::Data::GetDaysRemaining() << " days left)\n"
              << "IP:        " << Atlas::Data::GetIP() << "\n"
              << "HWID:      " << Atlas::Data::GetHWID() << "\n"
              << "Level:     " << Atlas::Data::GetLevel() << "\n"
              << "Note:      " << Atlas::Data::GetNote() << "\n"
              << "Active:    " << Atlas::Data::GetActiveUserCount()
              << " / total " << Atlas::Data::GetUserCount() << "\n";
}

void ShowVariable() {
    const std::string key = Ask("Variable name: ");
    std::cout << key << " = " << Atlas::Variables::Fetch(key) << "\n";
}

void ShowEntitlements() {
    const auto names = Atlas::Entitlements::List();
    if (names.empty()) std::cout << "No entitlements on this session.\n";
    for (const std::string& name : names)
        std::cout << name << "  remaining " << Atlas::Entitlements::Remaining(name) << "\n";
}

void ChangeEmail() {
    const std::string password = Ask("Current password: ");
    const std::string new_email = Ask("New email: ");
    if (!Atlas::Account::RequestEmailChange(password, new_email)) {
        PrintError();
        return;
    }
    std::cout << "We emailed an 8-digit code to " << new_email << ". It expires in 10 minutes.\n";
    if (!Atlas::Account::ConfirmEmailChange(Ask("Code: "))) {
        PrintError();
        return;
    }
    std::cout << "[+] Email changed to " << new_email << ".\n";
}

// Returns true when the account is gone, so the caller leaves the session.
bool DeleteAccount() {
    std::cout << "This deletes the account and signs out every session of it. The license key is not changed.\n";
    if (Ask("Type DELETE to continue: ") != "DELETE") return false;
    if (!Atlas::Account::RequestAccountDeletion(Ask("Current password: "))) {
        PrintError();
        return false;
    }
    std::cout << "We emailed an 8-digit code to the address on this account. It expires in 10 minutes.\n";
    if (!Atlas::Account::ConfirmAccountDeletion(Ask("Code: "))) {
        PrintError();
        return false;
    }
    std::cout << "[+] Account deleted.\n";
    return true;
}

std::string FormatUnixUtc(long long seconds) {
    const std::time_t t = static_cast<std::time_t>(seconds);
    std::tm utc{};
    gmtime_s(&utc, &t);
    char buf[32];
    std::strftime(buf, sizeof buf, "%Y-%m-%d %H:%M UTC", &utc);
    return buf;
}

void ShowDevices() {
    const auto devices = Atlas::Account::ListDevices();
    if (devices.empty()) {
        const std::string why = Atlas::Data::GetErrorMessage();
        if (why.empty()) std::cout << "No trusted devices.\n";
        else std::cout << "[!] " << why << "\n";
        return;
    }
    for (const auto& d : devices) {
        std::cout << "[" << d.id << "] " << (d.is_this_device ? "this device, " : "")
                  << "trusted " << FormatUnixUtc(d.trusted_at)
                  << ", last sign-in " << FormatUnixUtc(d.last_seen_at)
                  << " from " << d.last_seen_ip << "\n";
    }
    const std::string id = AskOptional("Revoke a device by id (Enter to skip): ");
    if (id.empty()) return;
    if (!Atlas::Account::RevokeDevice(std::strtoll(id.c_str(), nullptr, 10))) {
        PrintError();
        return;
    }
    std::cout << "[+] Device " << id << " is no longer trusted.\n";
}

void SignOutOthers() {
    const int ended = Atlas::Account::SignOutOtherDevices();
    if (ended < 0) {
        PrintError();
        return;
    }
    std::cout << "[+] Signed out " << ended << " other session(s).\n";
}

} // namespace

int main() {
    // Must be called once at startup, before any other Atlas function.
    Atlas::Startup();

    std::cout << "Atlas SDK console test\n";

    bool signed_in = false;
    while (!signed_in) {
        std::cout << "\n[1] License key   [2] Account sign-in   [3] Register account\n"
                  << "[4] Reset this machine (email code)   [5] Forgot password   [0] Exit\n";
        const std::string choice = Ask("Choice: ");
        if      (choice == "1") signed_in = SignInWithLicense();
        else if (choice == "2") signed_in = SignInWithAccount();
        else if (choice == "3") RegisterAccount();
        else if (choice == "4") ResetWithEmailCode();
        else if (choice == "5") ForgotPassword();
        else if (choice == "0") return 0;
    }

    PrintSession();
    const bool is_account = !Atlas::Data::GetUsername().empty();

    for (;;) {
        std::cout << "\n[R] reset this machine   [V] variable   [E] entitlements\n";
        if (is_account)
            std::cout << "[M] change email   [X] delete account   [D] devices   [O] sign out other devices\n";
        std::cout << "[P] ping   [L] send log line   [Q] quit\n";
        const std::string c = Ask("Choice: ");
        if      (c == "R" || c == "r") ResetWhileSignedIn(is_account);
        else if (c == "V" || c == "v") ShowVariable();
        else if (c == "E" || c == "e") ShowEntitlements();
        else if (is_account && (c == "M" || c == "m")) ChangeEmail();
        else if (is_account && (c == "X" || c == "x")) { if (DeleteAccount()) break; }
        else if (is_account && (c == "D" || c == "d")) ShowDevices();
        else if (is_account && (c == "O" || c == "o")) SignOutOthers();
        else if (c == "P" || c == "p") std::cout << "Ping: " << Atlas::Network::Ping() << "\n";
        else if (c == "L" || c == "l") Atlas::Network::SubmitLog("Console test: log line");
        else if (c == "Q" || c == "q") break;
    }

    Atlas::Logout();
    std::cout << "\nPress Enter to exit...\n";
    std::cin.get();
    return 0;
}
