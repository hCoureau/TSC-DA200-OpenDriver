#!/bin/sh
set -eu

usage() {
  echo "Usage: $0 [--yes] QUEUE gap|black-mark|auto" >&2
  exit 2
}

assume_yes=0
if [ "${1:-}" = "--yes" ]; then
  assume_yes=1
  shift
fi
[ "$#" -eq 2 ] || usage
queue=$1
mode=$2

case "$mode" in
  gap) command='GAPDETECT' ;;
  black-mark) command='BLINEDETECT' ;;
  auto) command='AUTODETECT' ;;
  *) usage ;;
esac

if [ "$assume_yes" -ne 1 ]; then
  printf 'Calibration will feed several labels on queue %s. Continue? [y/N] ' "$queue"
  read -r answer
  case "$answer" in y|Y|yes|YES) ;; *) echo "Cancelled."; exit 1 ;; esac
fi

printf '%s\r\n' "$command" | lp -d "$queue" -o raw -t "DA200 media calibration"
echo "Calibration command submitted: $command"
