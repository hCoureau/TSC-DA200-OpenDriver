# Platform support status

Support labels are deliberately evidence-based:

| Platform and path | v1 status | Evidence still required |
|---|---|---|
| Apple Silicon macOS 26, direct USB | Hardware and visually tested | Additional printer units and macOS 26 point releases |
| Intel macOS 11-26 package slice | Build tested | Physical Intel Mac installation and print |
| macOS 27 Golden Gate | Compatibility target | Final macOS 27 + Xcode 27 install and physical print |
| Raspberry Pi OS / Debian 12 ARM64 | Native build/install tested | Physical Pi USB and LAN client acceptance |
| Raspberry Pi OS / Debian 12 ARMHF | Native build/install tested | Physical Pi USB and LAN client acceptance |
| Debian 12 AMD64 | Native build/install tested | Representative physical Linux host print |
| Fedora 42 AArch64 / x86_64 | Native RPM build/install tested | Representative physical Fedora host print |
| Other CUPS 2.x Linux | Source compatible | Distribution-specific package and hardware tests |
| CUPS 3.x | Not supported by classic package | PAPPL Printer Application completion |
| Windows via Pi IPP | Designed path, not certified | Printer Application plus Windows 11 testbed |
| iPhone/iPad via Pi IPP | Designed path, app-dependent | IPP Everywhere/AirPrint Printer Application plus physical iOS testbed |
| Windows direct USB | Not implemented | Native spooler integration, package signing, testbed |

“Build tested” means compilation, automated tests, package installation, and
PPD validation in the named architecture's native userspace. It is not a
substitute for a physical printer test.
