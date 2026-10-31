#!/usr/bin/env bash
# Runs the tests on Linux with no screen: a virtual display, plus a window
# manager, which JUCE needs before it can open a plug-in's window.
set -euo pipefail
exe="$1"
xvfb-run -a -s "-screen 0 1920x1080x24" bash -c 'openbox >/dev/null 2>&1 & sleep 1; "$0"; status=$?; kill %1 2>/dev/null || true; exit $status' "$exe"
