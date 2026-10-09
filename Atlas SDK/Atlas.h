#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <cstdint>

// Atlas authentication library for Windows x64.
// Reference: https://atlassecurity.site/docs

namespace Atlas {

    // Your app's API key. Set it before Startup().
    inline std::string API_KEY = "YOUR_API_KEY";


    // -- Session lifecycle ---------------------------------------------------
    // Startup() once, first. Logout() ends the session; the library stays loaded. Exit() kills the process, no cleanup.
    // https://atlassecurity.site/docs?p=sdk/lifecycle

    void Startup();
    void Logout();
    void Exit();

    // Stops the library from opening any message box of its own: server notices, the wrong-API-key box and the
    // update notice. Call it before Startup(). The reason for a refusal stays in Data::GetErrorMessage().
    // The Dialog:: windows you call yourself still open.
    void DisableMessageBoxes(bool disabled = true);


    // -- License -------------------------------------------------------------
    // Sign in with a license key. The first sign-in locks the key to this PC.
    // Login(username, password) and Register() are only for a license that carries its own username and password.
    // For real user accounts use Account.
    // https://atlassecurity.site/docs?p=sdk/license

    namespace License {
        bool Login(const std::string& license_key);
        bool Login(const std::string& username, const std::string& password);
        bool Register(const std::string& license_key, const std::string& username, const std::string& password);

        // Built-in windows. Each blocks until the user closes it.
        namespace Dialog {
            bool Login();
            bool Register();
        }
    }


    // -- Account -------------------------------------------------------------
    // Username and password accounts, with optional email verification and password reset.
    // Login() returns a LoginResult: read result.status first. NeedsVerification means an 8-digit code was emailed,
    // so call SubmitVerification(code). Register() does not sign in.
    // https://atlassecurity.site/docs?p=sdk/account

    namespace Account {
        struct LoginResult;     // Defined at the bottom of this file.

        LoginResult Login(const std::string& username, const std::string& password);
        bool Register(const std::string& username, const std::string& password, const std::string& email = "");
        bool SubmitVerification(const std::string& eight_digit_code);
        bool ResendVerification();
        bool ConfirmEmail(const std::string& eight_digit_code);
        bool HasPendingEmailConfirm();
        bool Redeem(const std::string& license_key);
        bool RequestPasswordReset(const std::string& identifier);
        bool CompletePasswordReset(const std::string& eight_digit_code, const std::string& new_password);

        // Built-in windows. Each blocks until the user closes it.
        namespace Dialog {
            bool Login();
            bool Login(const std::string& username, const std::string& password);
            bool Register();
            bool VerifyCode();
            bool ConfirmEmail();
            bool ResetPassword();
        }
    }


    // -- Network -------------------------------------------------------------
    // Ask the server something during a session. The library already checks the session in the background,
    // so CheckAuthentication() is only for right before a sensitive action.
    // https://atlassecurity.site/docs?p=sdk/network

    namespace Network {
        bool CheckAuthentication();
        std::vector<uint8_t> Download(int file_id);
        bool BanUser(const std::string& reason, int duration_minutes);
        bool SubmitLog(const char* log_text);
        bool ChangePassword(const std::string& old_password, const std::string& new_password);
        int Ping();
    }


    // -- Data ----------------------------------------------------------------
    // Facts about the signed-in session. Valid only after a successful sign-in.
    // A getter with nothing to return gives "" or 0. GetDaysRemaining() is the exception: -1 means no expiry,
    // 0 means expired or under 24 hours left. GetExpiry() is "DD-MM-YYYY" or "Never".
    // https://atlassecurity.site/docs?p=sdk/data

    namespace Data {
        // Identity
        std::string GetLicense();
        std::string GetUsername();
        std::string GetEmail();
        std::string GetPassword();
        std::string GetIP();
        std::string GetHWID();
        std::string GetDevice();
        std::string GetNote();
        std::string GetFirstSeenDate();
        std::string GetLastSeenDate();
        int         GetUserId();
        int         GetLevel();

        // Expiry
        std::string GetExpiry();
        int         GetDaysRemaining();
        bool        IsLifetime();
        bool        IsExpiringSoon(int days_threshold = 7);

        // Status
        bool        IsAuthenticated();
        bool        IsBanned();

        // App-wide counts
        std::string GetActiveUserCount();
        std::string GetUserCount();

        // Errors
        std::string GetErrorMessage();
        void        ClearError();
        bool        HasError();
    }


    // -- Variables -----------------------------------------------------------
    // Values you set on the dashboard, read while the app runs. Change one without shipping a new build.
    // A key that does not exist gives "" (Fetch), 0 (FetchInt) or false (FetchBool).
    // https://atlassecurity.site/docs?p=sdk/variables

    namespace Variables {
        std::string Fetch(const std::string& key);
        bool        FetchBool(const std::string& key);
        int         FetchInt(const std::string& key);
    }


    // -- Entitlements --------------------------------------------------------
    // What this license or account may do: the features and credits you create on the dashboard.
    // Has() and Remaining() are for showing and hiding. Only Consume() is enforced by the server.
    // https://atlassecurity.site/docs?p=sdk/entitlements

    namespace Entitlements {
        bool                     Has(const std::string& key);
        long long                Remaining(const std::string& key);
        bool                     Consume(const std::string& key, int amount = 1);
        std::vector<std::string> List();
        bool                     Refresh();
    }


    // -- Webhook -------------------------------------------------------------
    // Send an HTTP POST from the client: Discord, Slack or your own endpoint. Unrelated to Atlas sign-in.
    // https://atlassecurity.site/docs?p=sdk/webhook

    namespace Webhook {
        bool SendDiscord(const std::string& webhook_url, const std::string& message);
        bool SendDiscordEmbed(const std::string& webhook_url, const std::string& title, const std::string& description, int color = 0x3498db);
        bool Send(const std::string& url, const std::string& json_payload);
    }


    // -- Dialog --------------------------------------------------------------
    // Theme, title and colour overrides for the built-in windows, plus FatalError. The sign-in windows are
    // License::Dialog and Account::Dialog.
    // https://atlassecurity.site/docs?p=sdk/dialog

    namespace Dialog {
        enum class Theme { Dark, Light };

        inline std::string AppDialogTitle = "";     // Heading of the dialog window. "" keeps the built-in one.
        inline std::string AppName = "Atlas";       // Title bar text of every window.
        inline Theme       theme = Theme::Dark;
        inline HWND        parent = nullptr;        // nullptr = the foreground window.

        // Colour overrides, 0xAARRGGBB (alpha ignored). A field left at 0 keeps the theme colour.
        // Setting panel also derives raised, raisedHover and lineSoft.
        //   Atlas::Dialog::accents.signal = 0xFFE04A2C;
        struct Accents {
            unsigned int signal     = 0;    // Primary button and focus rings.
            unsigned int panel      = 0;    // Window surface.
            unsigned int ink        = 0;    // Caption bar and dark backing.
            unsigned int hi_text    = 0;    // Primary text.
            unsigned int lo_text    = 0;    // Secondary text.
            unsigned int faint_text = 0;    // Labels and hints.
            unsigned int line       = 0;    // Hairline borders.
            unsigned int alert      = 0;    // Errors and destructive actions.
            unsigned int ok         = 0;    // Success and verified.
        };
        inline Accents accents{};

        void FatalError(const std::string& title, const std::string& body, const std::string& error_code = "");
    }


    // -- Types ---------------------------------------------------------------
    // The result type Account::Login returns.

    namespace Account {
        // Return from Atlas::Account::Login. Always check `status` first.
        struct LoginResult {
            enum class Status {
                Ok,                 // Signed in - session is live.
                WrongCredentials,   // Bad username/password.
                NeedsVerification,  // Server sent a code - call SubmitVerification.
                Banned,             // Account is banned.
                AccountPaused,      // Account is paused by the seller.
                ServerUnreachable,  // Network / DNS failure.
                Error,              // Anything else - check error_message.
            };
            Status      status = Status::Error;
            std::string error_message;      // Human-readable reason on any non-Ok status.
            int         user_id = 0;        // Signed-in row id (Ok only).
            std::string expiry;             // DD-MM-YYYY, "" = no expiry (Ok only).
            int         level = 1;          // Access level (Ok only).
            std::string note;               // Admin-set note (Ok only).
            std::string masked_email;       // e.g. "m...s@example.com" (NeedsVerification only).
            std::string sign_in_ip;         // Server-detected IP (NeedsVerification only).
            std::string sign_in_country;    // 2-letter ISO (NeedsVerification only).
        };
    }

} // namespace Atlas
