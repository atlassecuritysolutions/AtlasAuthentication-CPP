#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <cstdint>

// Atlas authentication library.
// Get your API key from atlassecurity.site/dashboard.
//
//   Atlas::API_KEY = "YOUR_API_KEY";
//   Atlas::Startup();
//   if (Atlas::License::Login("license-key")) { /* signed in */ }
//
// Namespace layout — pick the right bucket for the job:
//
//   Atlas::                       shared/session (Data, Network, Variables, Webhook)
//   Atlas::License::              license-key sign-in (headless)
//   Atlas::License::Dialog::      license-key sign-in with built-in Win32 UI
//   Atlas::Account::              username+password + email flow (headless)
//   Atlas::Account::Dialog::      username+password + email flow with built-in Win32 UI
//   Atlas::Dialog::               non-auth dialogs (fatal error, etc.)
//
// Anything inside a `Dialog::` namespace opens a modal Win32 window and blocks
// until the user closes it. Anything outside is pure API — never opens UI.


namespace Atlas {

    // Your app's API key. Get it from atlassecurity.site/dashboard.
    inline std::string API_KEY = "YOUR_API_KEY";


    // -- Session lifecycle ---------------------------------------------------

    // Initialise the library. Call once at the top of main().
    void Startup();

    // Terminate the session and clear all authentication state.
    void Logout();

    // Hard-kill the process via __fastfail. Uncatchable, no cleanup.
    void Exit();


    // -- License mode --------------------------------------------------------
    // Classic single-user, license-key auth. No email, no verify code.

    namespace License {
        // License-key sign-in.
        bool Login(const std::string& license_key);

        // Username + password sign-in for a license bound to one user (legacy USR mode).
        // For the multi-user flow with email verify, use Atlas::Account::Login.
        bool Login(const std::string& username, const std::string& password);

        // Bind a license key to a new username/password (legacy REG mode).
        // Does NOT sign in on success — call Login(u, p) after.
        bool Register(const std::string& license_key, const std::string& username, const std::string& password);


        // Built-in Win32 dialogs for the license flow. Each function opens a
        // modal window and blocks until the user closes it.
        namespace Dialog {
            // Prompt for a license key, then sign in.
            bool Login();

            // Prompt for license + username + password, then bind them.
            bool Register();
        }
    }


    // -- Account mode --------------------------------------------------------
    // Multi-user (username / password / email) accounts with email verify,
    // password reset, and the redeem-a-key-onto-my-account flow.

    namespace Account {
        // Full return shape defined at the bottom of this file.
        struct LoginResult;

        // Sign in with account credentials. Check result.status.
        // On NeedsVerification the SDK holds the challenge — call SubmitVerification(code).
        LoginResult Login(const std::string& username, const std::string& password);

        // Create a standalone account. Email optional but needed for password reset.
        // Does NOT sign in. If email is set, account stays unverified until ConfirmEmail.
        bool Register(const std::string& username, const std::string& password, const std::string& email = "");

        // Submit the 8-digit code for the pending sign-in verify challenge.
        bool SubmitVerification(const std::string& eight_digit_code);

        // Resend the sign-in verification code (60s server-side cooldown).
        bool ResendVerification();

        // Confirm a newly-registered account's email with the emailed code.
        bool ConfirmEmail(const std::string& eight_digit_code);

        // True while a registration email-confirm is pending.
        bool HasPendingEmailConfirm();

        // Redeem a license key onto the currently signed-in account.
        bool Redeem(const std::string& license_key);

        // Start a password reset. identifier = username or email.
        // Always returns true — anti-enumeration, the server never leaks whether it matched.
        bool RequestPasswordReset(const std::string& identifier);

        // Complete the reset with the emailed code + new password.
        bool CompletePasswordReset(const std::string& eight_digit_code, const std::string& new_password);


        // Built-in Win32 dialogs for the account flow. Each function opens a
        // modal window and blocks until the user closes it.
        namespace Dialog {
            // Full sign-in flow: creds dialog → sign in → verify dialog if needed.
            bool Login();

            // Creds pre-collected. Opens the verify dialog if the server requires it.
            bool Login(const std::string& username, const std::string& password);

            // Register dialog → create account. When registered with an email,
            // the confirm-code dialog opens automatically.
            bool Register();

            // 8-digit code dialog for the pending sign-in verify challenge.
            bool VerifyCode();

            // 8-digit code dialog for the pending registration email-confirm.
            bool ConfirmEmail();

            // Full password-reset flow: request-code dialog → email → complete-reset dialog.
            bool ResetPassword();
        }
    }


    // -- Non-auth dialogs ----------------------------------------------------
    // Reusable Win32 dialogs unrelated to a specific auth flow. Global theme
    // and per-dialog customisation live in the Dialog namespace too (see below).

    namespace Dialog {
        // Palette theme. Dark matches the Atlas dashboard.
        enum class Theme { Dark, Light };

        inline std::string AppName = "Atlas";           // Shown in every dialog title bar.
        inline Theme       theme   = Theme::Dark;       // Active theme.
        inline HWND        parent  = nullptr;           // Modal parent, null = foreground window.

        struct Accents;                                 // Colour overrides — defined at bottom.
        struct Copy;                                    // String overrides — defined at bottom.

        // Unrecoverable-error dialog with a Copy Details button. Informational.
        void FatalError(const std::string& title, const std::string& body, const std::string& error_code = "");
    }


    // -- Network -------------------------------------------------------------
    // Direct server RPCs on the current session.

    namespace Network {
        // Poll the server to confirm the current session is still valid.
        bool CheckAuthentication();

        // Fetch a dashboard-uploaded file by id. Empty vector on failure.
        std::vector<uint8_t> Download(int file_id);

        // Ban the current user from your app. duration_minutes = 0 → permanent.
        bool BanUser(const std::string& reason, int duration_minutes);

        // Emit a custom log line (max 512 chars) to the dashboard's Logs tab.
        bool SubmitLog(const char* log_text);

        // Change the current account's password.
        bool ChangePassword(const std::string& old_password, const std::string& new_password);

        // Round-trip latency to the auth server in ms, or -1 if unreachable.
        int Ping();
    }


    // -- Data ----------------------------------------------------------------
    // Read-only session accessors. Populated after a successful sign-in.

    namespace Data {
        //Authentication Data
        std::string GetLicense();                       // License key the session opened with.
        std::string GetUsername();                      // Account username, "" on license-only sessions.
        std::string GetEmail();                         // Account email, "" if none / license-only.
        std::string GetPassword();                      // Password used at sign-in, "" on license-only.
        std::string GetIP();                            // Server-detected client IP.
        std::string GetHWID();                          // Hardware fingerprint.
        std::string GetDevice();                        // ComputerName / Windows username.
        std::string GetNote();                          // Admin-set note, "" if none.
        std::string GetFirstSeenDate();                 // First-ever authentication timestamp.
        std::string GetLastSeenDate();                  // Most recent authentication timestamp.
        int         GetUserId();                        // Account row id, 0 if signed out.
        int         GetLevel();                         // Access level, 0 if unknown.

        //Expiry
        std::string GetExpiry();                        // "DD-MM-YYYY HH:MM:SS" or "Lifetime".
        int         GetDaysRemaining();                 // -1 = lifetime, 0 = expired.
        bool        IsLifetime();                       // True if the license never expires.
        bool        IsExpiringSoon(int days_threshold = 7); // True if expiring within days_threshold.

        //Authentication Verdicts
        bool        IsAuthenticated();                  // True if a live session is open.
        bool        IsBanned();                         // True if the current user is banned.

        //Global Application Stats
        std::string GetActiveUserCount();               // Users currently authenticated app-wide.
        std::string GetUserCount();                     // Total registered users.

        //Atlas Errors
        std::string GetErrorMessage();                  // Last error message, "" if none.
        void        ClearError();                       // Reset the error state.
        bool        HasError();                         // True if the last call set an error.
    }


    // -- Variables -----------------------------------------------------------
    // Read-only key/value store you configure on the dashboard.

    namespace Variables {
        std::string Fetch(const std::string& key); // "" if the key doesn't exist.
        bool FetchBool(const std::string& key); // "true" / "1" / "yes" → true; else false.
        int FetchInt(const std::string& key);  // 0 if missing or unparseable.
    }


    // -- Webhook -------------------------------------------------------------
    // Fire-and-forget HTTP POSTs (Discord, Slack, custom). Unrelated to Atlas auth.

    namespace Webhook {
        // Plaintext Discord webhook message.
        bool SendDiscord(const std::string& webhook_url, const std::string& message);
        // Discord embed. color is 0xRRGGBB.
        bool SendDiscordEmbed(const std::string& webhook_url, const std::string& title, const std::string& description, int color = 0x3498db);
        // POST an arbitrary JSON payload — Slack, custom endpoints, telemetry.
        bool Send(const std::string& url, const std::string& json_payload);
    }


    // -- Types ---------------------------------------------------------------
    // Struct definitions kept out of the API surface above so the function
    // list reads fast. Just data shapes here.

    namespace Dialog {
        // Colour overrides. 0xAARRGGBB (alpha ignored). Fields at 0 keep the theme default.
        // Setting `panel` also auto-derives raised/raisedHover/lineSoft so a single override stays coherent.
        //   Atlas::Dialog::accents.signal = 0xFFE04A2C;   // orange primary
        struct Accents {
            unsigned int signal     = 0;    // Primary CTA + focus rings.
            unsigned int panel      = 0;    // Dialog surface.
            unsigned int ink        = 0;    // Caption bar / dark backing.
            unsigned int hi_text    = 0;    // Primary text.
            unsigned int lo_text    = 0;    // Secondary text.
            unsigned int faint_text = 0;    // Labels / hints.
            unsigned int line       = 0;    // Hairline borders.
            unsigned int alert      = 0;    // Errors + destructive.
            unsigned int ok         = 0;    // Success + verified.
        };
        inline Accents accents{};

        // User-facing string overrides. "" keeps the built-in default —
        // every field is pre-initialised to "" so you only touch what you
        // want to change (no need to write = "" yourself for the rest).
        //   Atlas::Dialog::copy.verify_title = "Enter code";
        struct Copy {
            std::string verify_title           = "";    // Verify dialog: hero title.
            std::string verify_prompt_prefix   = "";    // Verify dialog: text before the masked email.
            std::string verify_prompt_fallback = "";    // Verify dialog: prompt when no masked email.
            std::string verify_button_verify   = "";    // Verify dialog: primary button.
            std::string verify_button_cancel   = "";    // Verify dialog: secondary button.
            std::string verify_button_resend   = "";    // Verify dialog: resend link.
            std::string verify_footer_note     = "";    // Verify dialog: footer attribution.
            std::string confirm_title          = "";    // Confirm-email dialog: hero title.
            std::string confirm_prompt_prefix  = "";    // Confirm-email dialog: text before the email.
            std::string confirm_prompt_fallback= "";    // Confirm-email dialog: prompt when no email.
            std::string login_title            = "";    // Login dialog: hero title.
            std::string register_title         = "";    // Register dialog: hero title.
            std::string reset_title            = "";    // Reset dialogs: hero title.
        };
        inline Copy copy{};
    }

    namespace Account {
        // Return from Atlas::Account::Login. Always check `status` first.
        struct LoginResult {
            enum class Status {
                Ok,                 // Signed in — session is live.
                WrongCredentials,   // Bad username/password.
                NeedsVerification,  // Server sent a code — call SubmitVerification.
                Banned,             // Account is banned.
                AccountPaused,      // Account is paused by the seller.
                ServerUnreachable,  // Network / DNS failure.
                Error,              // Anything else — check error_message.
            };
            Status      status = Status::Error;
            std::string error_message;      // Human-readable reason on any non-Ok status.
            int         user_id = 0;        // Signed-in row id (Ok only).
            std::string expiry;             // ISO-8601, "" = no expiry (Ok only).
            int         level = 1;          // Access level (Ok only).
            std::string note;               // Admin-set note (Ok only).
            std::string masked_email;       // e.g. "ob...d@gmail.com" (NeedsVerification only).
            std::string sign_in_ip;         // Server-detected IP (NeedsVerification only).
            std::string sign_in_country;    // 2-letter ISO (NeedsVerification only).
        };
    }

} // namespace Atlas
