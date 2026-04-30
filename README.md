# WinMTR-Official

Thank you for downloading WinMTR v0.92!

## About

WinMTR is a free Microsoft Windows visual application that combines the functionality of the traceroute and ping in a single network diagnostic tool. WinMTR is Open Source Software, (barely) maintained by Dragos Manac.

It was started in 2000 by Vasile Laurentiu Stanimir as a clone for the popular Matt's Traceroute (hence MTR) Linux/UNIX utility.

## License & Redistribution

WinMTR is offered as Open Source Software under GPL v2.

- [Read more about the licensing conditions](http://www.gnu.org/licenses/gpl-2.0.html)
- [Download the code](https://github.com/WinMTR/WinMTR-Official)

## Installation

You will get a `.zip` archive containing two folders: `WinMTR-32` and `WinMTR-64`. Both contain two files: `WinMTR.exe` and `README.md`.

Just extract the `WinMTR.exe` for your platform (32 or 64 bit) and click to run it. If you do not know what version you need, try the executable that matches your Windows installation.

WinMTR requires no other installation effort.

**Tip:** You can copy `WinMTR.exe` to `Windows/System32` so it is accessible via the command line.

## Usage

### Visual

1. Start WinMTR.
2. Write the name or IP of the host, for example `github.com`.
3. Press the **Options** button to configure ping size, maximum hops and ping interval.
4. Push the **Start** button and wait.
5. Copy or export the results in text, CSV or HTML format.
6. Click on **Clear History** to remove the hosts you have previously traced.

### MTR-style table features

- The results table includes additional MTR-style metrics: `Drop`, `StDev`, `Gmean`, `Jttr`, `Javg`, `Jmax`, and `Jint`.
- The **Help** button opens a short explanation of every displayed metric.
- Rows can be selected across the full table width, which helps during troubleshooting calls.
- Columns can be reordered and the order is persisted beside the executable.
- Manual window resize triggers a final redraw when the resize operation ends.

### Command Line

```text
winmtr --help        # See available options
winmtr github.com    # Trace a host
```

## Troubleshooting

**I type in the address and nothing happens.**
Usually this has to do with antivirus or firewall applications. Disable them when debugging or using WinMTR, or configure them properly.

**I get an error saying the program cannot be executed.**
You are running the 64-bit version on a 32-bit platform. Try the `WinMTR.exe` in the 32-bit package.

**I get an error not listed here.**
Please report it to make sure it is not a bug in the application.

## Changelog

| Date | Version | Changes |
|------|---------|---------|
| Unreleased | - | Added MTR-style metrics, CSV export, metric help, full-row selection, column persistence, resize redraw stabilization, export hardening and shared metric metadata. |
| 2025-11-26 | - | Homepage moved to WinMTR.net and development repository to GitHub. Still looking for developers. |
| 2011-01-31 | v0.92 | Fixed reporting errors for very slow connections |
| 2011-01-11 | v0.91 | Released under GPL v2 by popular request |
| 2010-12-24 | v0.9 | Support for 32 and 64 bit OS. Works on Windows 7 as regular user. Various bug fixes. |
| 2002-01-20 | - | Last entered hosts and options now stored in registry. Moved to SourceForge. |
| 2001-09-05 | v0.7 | Combo box for host history. Fixed memory leak causing crashes. |
| 2000-11-27 | v0.6 | Added resizing support and flat buttons |
| 2000-11-26 | v0.5 | Copy to clipboard, save as text/HTML |
| 2000-08-03 | v0.4 | Double-click host for detailed info |
| 2000-08-02 | v0.3 | Fixed ICMP error codes handling |
| 2000-08-01 | v0.2 | Full command-line support |
| 2000-07-28 | v0.1 | First release |

## Bug Reports

Let us know if you identify bugs. Please include:

- WinMTR version
- Operating system and setup details

Before submitting, make sure the issue is not related to your specific configuration, such as antivirus or firewall settings.

## Feature Requests

If you need functionality that others could also benefit from, let us know. We will try to integrate it in future releases.

If you are a developer planning to extend the code, please reach out so changes can be integrated into the official tree.

## Contact

Email: contact AT winmtr DOT net
