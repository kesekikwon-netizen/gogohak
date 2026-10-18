# Gemini Desktop Bridge Rules

You have access to the **Gemini Desktop Bridge** MCP tools (`gemini_web_ask`, `gemini_web_status`, `gemini_web_toggle_panel`, `gemini_web_new_chat`), which execute prompts against the embedded Google Gemini Web (`https://gemini.google.com/app`) session running inside Antigravity (`GeminiWindowsApp/1.15.0 (prod)`).

## Quota Optimization Guidelines
- This bridge uses the separate **Gemini Web (consumer/Workspace)** quota, completely independent of Antigravity's Cloud Code / agent quota.
- When asked to use `@gemini-desktop`, `gemini-web`, or when offloading large code generation, boilerplate drafting, long documentation generation, or second-opinion code reviews to save Antigravity quota, call `gemini_web_ask`.
- If the user wants to open or log into the embedded Gemini panel inside the Antigravity PC app, call `gemini_web_toggle_panel` (or tell them to press `Ctrl+Shift+G` or click the floating `✨ Gemini` button in the bottom-right corner of the Antigravity window).
