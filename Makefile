CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -Werror
CPPFLAGS ?=
CUPS_CFLAGS := $(shell cups-config --cflags 2>/dev/null)
CUPS_LIBS := $(shell cups-config --libs 2>/dev/null || echo -lcups)
SERVERBIN := $(shell cups-config --serverbin 2>/dev/null || echo /usr/lib/cups)
PPDDIR ?= /usr/share/ppd/tsc
TOOLDIR ?= /usr/local/bin
DESTDIR ?=

.PHONY: all clean install test check-release audit-macos-pkg

all: build/rastertotspl

build:
	mkdir -p build

build/rastertotspl: src/rastertotspl.c src/tspl.c src/tspl.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CUPS_CFLAGS) -Isrc -o $@ src/rastertotspl.c src/tspl.c $(CUPS_LIBS)

build/test_tspl: tests/test_tspl.c src/tspl.c src/tspl.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -D_GNU_SOURCE -Isrc -o $@ tests/test_tspl.c src/tspl.c

build/make_raster: tests/make_raster.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CUPS_CFLAGS) -o $@ tests/make_raster.c $(CUPS_LIBS)

test: build/test_tspl build/rastertotspl build/make_raster
	./build/test_tspl
	./build/make_raster > build/test.raster
	./build/rastertotspl 1 test test 1 "TscMediaType=Gap TscGap=3" build/test.raster > build/test.tspl
	LC_ALL=C grep -a -q "BITMAP 0,0,2,8,1," build/test.tspl
	LC_ALL=C grep -a -q "PRINT 1,1" build/test.tspl
	./build/make_raster --gray > build/test-gray.raster
	./build/rastertotspl 2 test gray 1 "ColorOption=Halftone" build/test-gray.raster > build/test-gray.tspl
	LC_ALL=C grep -a -q "BITMAP 0,0,2,8,1," build/test-gray.tspl
	test `wc -c < build/test-gray.tspl` -gt 100
	./build/make_raster --two-pages > build/test-collate.raster
	./build/rastertotspl 3 test collate 2 "Collate" build/test-collate.raster > build/test-collate.tspl
	test `LC_ALL=C grep -a -o "BITMAP 0,0,2,8,1," build/test-collate.tspl | wc -l` -eq 4
	test `LC_ALL=C grep -a -o "PRINT 1,1" build/test-collate.tspl | wc -l` -eq 4
	./build/rastertotspl 4 test uncollated 2 "noCollate" build/test-collate.raster > build/test-uncollated.tspl
	test `LC_ALL=C grep -a -o "BITMAP 0,0,2,8,1," build/test-uncollated.tspl | wc -l` -eq 2
	test `LC_ALL=C grep -a -o "PRINT 1,2" build/test-uncollated.tspl | wc -l` -eq 2
	./build/make_raster --bad-dpi > build/test-bad-dpi.raster
	! ./build/rastertotspl 5 test bad-dpi 1 "" build/test-bad-dpi.raster > /dev/null
	./build/make_raster --bad-width > build/test-bad-width.raster
	! ./build/rastertotspl 6 test bad-width 1 "" build/test-bad-width.raster > /dev/null
	./build/make_raster --short-row > build/test-short-row.raster
	! ./build/rastertotspl 7 test short-row 1 "" build/test-short-row.raster > /dev/null
	./build/make_raster --truncated > build/test-truncated.raster
	! ./build/rastertotspl 8 test truncated 1 "" build/test-truncated.raster > /dev/null
	! ./build/rastertotspl 9 test copies 0 "" build/test.raster > /dev/null
	! ./build/rastertotspl 10 test copies 10000 "" build/test.raster > /dev/null
	@echo "CUPS raster integration test passed"

check-release:
	./scripts/check-release.sh

audit-macos-pkg: check-release
	./macos/audit-pkg.sh

install: build/rastertotspl
	install -d "$(DESTDIR)$(SERVERBIN)/filter" "$(DESTDIR)$(PPDDIR)" "$(DESTDIR)$(TOOLDIR)"
	install -m 0755 build/rastertotspl "$(DESTDIR)$(SERVERBIN)/filter/rastertotspl"
	install -m 0644 ppd/TSC-DA200.ppd "$(DESTDIR)$(PPDDIR)/TSC-DA200.ppd"
	install -m 0644 ppd/TSC-DA200-4x6.ppd "$(DESTDIR)$(PPDDIR)/TSC-DA200-4x6.ppd"
	install -m 0755 tools/calibrate.sh "$(DESTDIR)$(TOOLDIR)/tsc-da200-calibrate"
	install -m 0755 tools/diagnose.sh "$(DESTDIR)$(TOOLDIR)/tsc-da200-diagnose"
	install -m 0755 scripts/setup-printer.sh "$(DESTDIR)$(TOOLDIR)/tsc-da200-setup"

clean:
	rm -rf build
