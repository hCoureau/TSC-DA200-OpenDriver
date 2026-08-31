# Third-party notices

This project is an independent implementation and does not contain firmware,
source code, binaries, logos, or PPD files from TSC Auto ID Technology Co.,
Ltd. or Citizen Systems.

The filter builds against the CUPS API supplied by the target operating
system. CUPS and libcups are developed by OpenPrinting and are licensed under
the Apache License, Version 2.0, with the exceptions documented by that
project. The macOS package links to the system-provided libcups; Debian
packages declare CUPS as a dependency and do not bundle it.

The ordered-halftone screen is generated in `src/tspl.c` from the canonical
recursive Bayer construction. It does not contain the lookup table recovered
from the legacy vendor driver during pre-release compatibility testing.

`TSC` and `DA200` are used only to identify the compatible printer. TSC is a
trademark of TSC Auto ID Technology Co., Ltd. Other names and marks belong to
their respective owners.
