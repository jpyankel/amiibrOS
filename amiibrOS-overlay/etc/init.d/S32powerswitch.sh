#! /bin/sh
# /etc/init.d/powerswitch.sh
#
# description: powerswitch init script
# processname: powerswitch_init
#
# Joseph Yankel (jpyankel)

start() {
  echo "starting powerswitch control..."
  chmod +x /usr/bin/amiibrOS/powerswitch/powerswitch.py
  /usr/bin/amiibrOS/powerswitch/powerswitch.py &
}

case "$1" in
    start)
  start
  ;;
    *)
  echo "Usage: $0 {start}"
	exit 1
  ;;
esac

exit 0
